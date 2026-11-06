package course

import (
	"context"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"syscall"
	"time"
)

type execResult struct {
	stdout, stderr string
	code           int
	note           string // e.g. timed out
	timedOut       bool
	took           time.Duration
}

// exec runs a learner-built binary in a fresh temp dir holding the given
// files. stdin, stdout and stderr are real files rather than Go pipes, and
// the program runs in its own process group so a timeout kills everything
// it started. (Pipe-based launches of sanitizer builds hung on macOS.)
func (e *Engine) exec(ctx context.Context, bin string, args []string, stdin string, files map[string]string, timeout time.Duration) execResult {
	tmp, err := os.MkdirTemp("", "sysroad-run-*")
	if err != nil {
		return execResult{stderr: err.Error(), code: -1}
	}
	defer os.RemoveAll(tmp)
	work := filepath.Join(tmp, "cwd")
	os.MkdirAll(work, 0o755)
	for name, data := range files {
		p := filepath.Join(work, filepath.FromSlash(name))
		os.MkdirAll(filepath.Dir(p), 0o755)
		os.WriteFile(p, []byte(data), 0o644)
	}
	inPath := filepath.Join(tmp, "stdin")
	if err := os.WriteFile(inPath, []byte(stdin), 0o644); err != nil {
		return execResult{stderr: err.Error(), code: -1}
	}
	in, err := os.Open(inPath)
	if err != nil {
		return execResult{stderr: err.Error(), code: -1}
	}
	defer in.Close()
	out, err := os.Create(filepath.Join(tmp, "stdout"))
	if err != nil {
		return execResult{stderr: err.Error(), code: -1}
	}
	defer out.Close()
	errf, err := os.Create(filepath.Join(tmp, "stderr"))
	if err != nil {
		return execResult{stderr: err.Error(), code: -1}
	}
	defer errf.Close()

	cmd := exec.Command(bin, args...)
	cmd.Dir = work
	cmd.Env = sanitizerEnv()
	cmd.Stdin, cmd.Stdout, cmd.Stderr = in, out, errf
	cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
	start := time.Now()
	if err := cmd.Start(); err != nil {
		return execResult{stderr: err.Error(), code: -1}
	}
	done := make(chan error, 1)
	go func() { done <- cmd.Wait() }()
	killGroup := func() {
		syscall.Kill(-cmd.Process.Pid, syscall.SIGKILL)
		cmd.Process.Kill()
	}

	var r execResult
	tick := time.NewTicker(100 * time.Millisecond)
	defer tick.Stop()
	deadline := time.After(timeout)
	var waitErr error
wait:
	for {
		select {
		case waitErr = <-done:
			break wait
		case <-deadline:
			r.timedOut = true
			killGroup()
			waitErr = <-done
			break wait
		case <-ctx.Done():
			killGroup()
			waitErr = <-done
			r.note = " (cancelled)"
			break wait
		case <-tick.C:
			// A runaway print loop could fill the disk before the timeout.
			if st, err := out.Stat(); err == nil && st.Size() > 8*outputLimit {
				killGroup()
				waitErr = <-done
				r.note = " — killed: it printed more than 512 KiB (an infinite loop?)"
				break wait
			}
		}
	}
	r.took = time.Since(start)
	syscall.Kill(-cmd.Process.Pid, syscall.SIGKILL) // anything it left running

	read := func(f *os.File) (string, bool) {
		buf := make([]byte, outputLimit+1)
		n, _ := f.ReadAt(buf, 0)
		return string(buf[:min(n, outputLimit)]), n > outputLimit
	}
	var cut1, cut2 bool
	r.stdout, cut1 = read(out)
	r.stderr, cut2 = read(errf)
	if (cut1 || cut2) && r.note == "" {
		r.note = " (output cut off at 64 KiB)"
	}
	var ee *exec.ExitError
	switch {
	case r.timedOut:
		r.code, r.note = -1, fmt.Sprintf(" — killed after %s (infinite loop, or waiting for input?)", timeout)
	case errors.As(waitErr, &ee):
		r.code = ee.ExitCode()
		if r.code == -1 && r.note == "" {
			r.note = " (" + ee.String() + ")"
		}
	case waitErr != nil:
		r.code, r.stderr = -1, r.stderr+waitErr.Error()
	}
	return r
}

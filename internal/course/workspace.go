package course

import (
	"errors"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
)

// SnapshotsDirName holds a copy of each project's folder as it was when
// each of its lessons was passed (or caught up), so Reset can go back to
// the start of a lesson. It's in the data repo, so it syncs.
const SnapshotsDirName = "snapshots"

// LeftOut reports whether an entry in your folder is left out of builds
// and snapshots: dot files and folders, and build output, as the data
// repo's .gitignore leaves them out of sync.
func LeftOut(name string, dir bool) bool {
	if strings.HasPrefix(name, ".") {
		return true
	}
	if dir {
		return name == "build"
	}
	return name == "a.out" || strings.HasSuffix(name, ".o") || strings.HasSuffix(name, ".tmp")
}

// listFiles lists the regular files under root as slash paths relative to
// it, leaving out what LeftOut says. A missing root has none.
func listFiles(root string) ([]string, error) {
	var out []string
	err := filepath.WalkDir(root, func(p string, d fs.DirEntry, err error) error {
		if err != nil {
			if p == root && errors.Is(err, fs.ErrNotExist) {
				return fs.SkipAll
			}
			return err
		}
		if p == root {
			return nil
		}
		if LeftOut(d.Name(), d.IsDir()) {
			if d.IsDir() {
				return fs.SkipDir
			}
			return nil
		}
		if d.Type().IsRegular() {
			rel, err := filepath.Rel(root, p)
			if err != nil {
				return err
			}
			out = append(out, filepath.ToSlash(rel))
		}
		return nil
	})
	return out, err
}

// WorkFiles lists the files in a lesson's folder (for a project lesson, the
// whole project), as slash paths relative to it, sorted, without dot files
// and folders or build output.
func (e *Engine) WorkFiles(l *Lesson) ([]string, error) { return listFiles(e.WorkDir(l)) }

// projectSources is every .cpp and .cc file in a project folder, however
// deep, as paths relative to it.
func projectSources(work string) ([]string, error) {
	files, err := listFiles(work)
	if err != nil {
		return nil, err
	}
	var srcs []string
	for _, f := range files {
		if strings.HasSuffix(f, ".cpp") || strings.HasSuffix(f, ".cc") {
			srcs = append(srcs, filepath.FromSlash(f))
		}
	}
	return srcs, nil
}

// SnapshotDir is where a project lesson's snapshot is kept.
func (e *Engine) SnapshotDir(l *Lesson) string {
	return filepath.Join(e.DataDir, SnapshotsDirName, workName(l), l.ID)
}

// SaveSnapshot copies the project folder to the lesson's snapshot,
// replacing an older one.
func (e *Engine) SaveSnapshot(l *Lesson) error {
	if l.project == nil {
		return errNotProject
	}
	dst := e.SnapshotDir(l)
	tmp := dst + ".tmp"
	os.RemoveAll(tmp)
	if err := copyTree(e.WorkDir(l), tmp); err != nil {
		os.RemoveAll(tmp)
		return err
	}
	if err := os.RemoveAll(dst); err != nil {
		return err
	}
	return os.Rename(tmp, dst)
}

// UseReference replaces the project folder with the course's reference as
// of the end of l, after moving your files into .local/reset-backup/. It
// returns where they went.
func (e *Engine) UseReference(l *Lesson) (string, error) {
	if l.project == nil {
		return "", errNotProject
	}
	ref, err := l.Solution()
	if err != nil {
		return "", err
	}
	backup, err := e.moveAside(l)
	if err != nil {
		return "", err
	}
	dir := e.WorkDir(l)
	for name, data := range ref {
		p := filepath.Join(dir, filepath.FromSlash(name))
		if err := os.MkdirAll(filepath.Dir(p), 0o755); err != nil {
			return backup, err
		}
		if err := os.WriteFile(p, data, 0o644); err != nil {
			return backup, err
		}
	}
	return backup, os.MkdirAll(dir, 0o755)
}

// copyTree copies the files listFiles finds under src into dst, keeping
// their permissions.
func copyTree(src, dst string) error {
	files, err := listFiles(src)
	if err != nil {
		return err
	}
	if err := os.MkdirAll(dst, 0o755); err != nil {
		return err
	}
	for _, f := range files {
		from := filepath.Join(src, filepath.FromSlash(f))
		info, err := os.Stat(from)
		if err != nil {
			return err
		}
		data, err := os.ReadFile(from)
		if err != nil {
			return err
		}
		to := filepath.Join(dst, filepath.FromSlash(f))
		if err := os.MkdirAll(filepath.Dir(to), 0o755); err != nil {
			return err
		}
		if err := os.WriteFile(to, data, info.Mode().Perm()); err != nil {
			return err
		}
	}
	return nil
}

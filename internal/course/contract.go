package course

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"sync"
	"unicode"
)

// The contract check runs before the tests build: it names the contract
// items your header is missing or declares differently, where the test
// build would bury the same mistake in template errors.

// contractIncludes come after your header in the check, so the types a
// signature mentions resolve even if the header doesn't need them itself.
var contractIncludes = []string{"cstddef", "cstdint", "functional", "map", "memory", "optional",
	"span", "string", "string_view", "utility", "vector"}

// contractSource is a file that includes the header and uses each item the
// way the tests will: a function's address taken as exactly its type, a
// type's size.
func contractSource(header string, items []ContractItem) string {
	var b strings.Builder
	fmt.Fprintf(&b, "#include %q\n", header)
	for _, inc := range contractIncludes {
		fmt.Fprintf(&b, "#include <%s>\n", inc)
	}
	for i, it := range items {
		if it.IsType() {
			fmt.Fprintf(&b, "static void sysroad_check_%d() { (void)sizeof(%s); }\n", i, it.Name)
		} else {
			fmt.Fprintf(&b, "static void sysroad_check_%d() { using expected_type = %s; expected_type* p = &%s; (void)p; }\n", i, it.Sig, it.Name)
		}
	}
	return b.String()
}

// missingHeader is the result when the project has no contract header yet.
func missingHeader(l *Lesson) *Result {
	h := l.project.Header
	var b strings.Builder
	b.WriteString("The tests include " + h + " from your project folder. Create it, and declare:\n\n")
	for _, it := range l.Contract() {
		b.WriteString("  " + it.Decl() + "\n")
	}
	return &Result{Stage: "contract", Summary: "✗ create " + h, Output: b.String()}
}

// checkContract compiles the contract file syntax-only in dir (one compile
// when everything matches) and, if it fails, finds out which items fail. It
// returns nil when the header matches the contract.
func (e *Engine) checkContract(ctx context.Context, cxx string, flags []string, l *Lesson, work, dir string) *Result {
	items := l.Contract()
	if len(items) == 0 {
		return nil
	}
	if err := os.MkdirAll(dir, 0o755); err != nil {
		return &Result{Stage: "contract", Summary: "✗ couldn't check the contract", Output: err.Error() + "\n"}
	}
	header := l.project.Header
	try := func(name string, items []ContractItem) compiled {
		src := filepath.Join(dir, name+".cpp")
		if err := os.WriteFile(src, []byte(contractSource(header, items)), 0o644); err != nil {
			return compiled{err: err, out: []byte(err.Error())}
		}
		args := append(append([]string{}, flags...), "-fsyntax-only", "-w", "-I", ".", src)
		return compile(ctx, work, cxx, args)
	}
	if try("contract", items).err == nil {
		return nil
	}
	// The header itself may not compile; then no item can be checked.
	if res := try("header", nil); res.err != nil {
		return &Result{Stage: "contract", Summary: "✗ " + header + " doesn't compile",
			Output: tidyContract(string(res.out), work, dir) + "\n"}
	}
	fails := make([]string, len(items))
	var wg sync.WaitGroup
	sem := make(chan struct{}, runtime.NumCPU())
	for i, it := range items {
		wg.Add(1)
		go func() {
			defer wg.Done()
			sem <- struct{}{}
			defer func() { <-sem }()
			if res := try(fmt.Sprint("item-", i), []ContractItem{it}); res.err != nil {
				fails[i] = contractProblem(it, header, tidyContract(string(res.out), work, dir))
			}
		}()
	}
	wg.Wait()
	// A function whose signature uses a contract type that's missing fails
	// with whatever the compiler makes of the unknown name; say why instead.
	for i, it := range items {
		if fails[i] == "" || it.IsType() {
			continue
		}
		for j, t := range items {
			if t.IsType() && fails[j] != "" && usesName(it.Sig, t.Name) {
				fails[i] = "needs " + t.Name + ", which " + header + " doesn't declare yet"
				break
			}
		}
	}

	r := &Result{Stage: "contract", Total: len(items)}
	var b strings.Builder
	b.WriteString(section("Contract", header) + "\n")
	for i, it := range items {
		if fails[i] == "" {
			r.Passed++
			b.WriteString(green("✓ ") + dim(it.Decl()) + "\n")
			continue
		}
		b.WriteString(red("✗ ") + it.Decl() + "\n" + indent(fails[i], "    ") + "\n")
	}
	if r.Passed == r.Total {
		// Each item works alone, so they clash with each other or the
		// standard headers: show the compiler's whole say.
		res := try("contract", items)
		b.WriteString("\n" + tidyContract(string(res.out), work, dir) + "\n")
	}
	b.WriteString("\n" + dim("The tests use these exactly as written here. Everything else in your project is up to you.") + "\n")
	r.Summary = "✗ " + header + " doesn't match the lesson's contract"
	r.Output = b.String()
	return r
}

// usesName reports whether a C++ signature mentions name as a whole word.
func usesName(sig, name string) bool {
	word := func(r rune) bool { return r == '_' || r == ':' || unicode.IsLetter(r) || unicode.IsDigit(r) }
	for _, w := range strings.FieldsFunc(sig, func(r rune) bool { return !word(r) }) {
		if w == name || strings.TrimPrefix(w, "::") == name {
			return true
		}
	}
	return false
}

// contractProblem says what's wrong with one item, from the compiler's
// first error line about it.
func contractProblem(it ContractItem, header, out string) string {
	line := ""
	for _, s := range strings.Split(stripANSI(out), "\n") {
		if strings.Contains(s, "error:") {
			line = strings.TrimSpace(s)
			break
		}
	}
	name := it.Name[strings.LastIndex(it.Name, ":")+1:]
	undeclared := strings.Contains(line, "undeclared identifier") || strings.Contains(line, "unknown type name") ||
		strings.Contains(line, "was not declared") || strings.Contains(line, "does not name a type") || strings.Contains(line, "no member named")
	if undeclared && strings.Contains(line, "'"+name+"'") {
		return "not declared in " + header
	}
	if it.IsType() {
		return dim(line)
	}
	return header + " declares it differently:\n" + dim(line)
}

// tidyContract drops the check file's own location from compiler output,
// which means nothing to you, and shortens paths into your folder.
func tidyContract(out, work, dir string) string {
	var lines []string
	for _, s := range strings.Split(strings.TrimRight(out, "\n"), "\n") {
		plain := stripANSI(s)
		if strings.HasPrefix(plain, dir+string(filepath.Separator)) {
			if i := strings.Index(plain, " error: "); i >= 0 {
				s = strings.TrimSpace(plain[i:])
			} else if strings.Contains(plain, " note: ") || strings.Contains(plain, "warning") {
				continue
			}
		}
		lines = append(lines, strings.ReplaceAll(s, work+string(filepath.Separator), ""))
	}
	return strings.Join(lines, "\n")
}

import { escape } from "./highlight";

// ANSI SGR → HTML spans with classes the theme styles (ansi-red, ansi-bold…).
// The engine and clang only use the 16 colours, bold, dim and resets; other
// sequences (256-colour, cursor movement) are dropped.
const names = ["black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"];

export function ansiToHtml(input: string): string {
  let out = "";
  let fg = "";
  let bg = "";
  let bold = false;
  let dim = false;
  let italic = false;
  let underline = false;

  const flush = (text: string) => {
    if (!text) return;
    const cls: string[] = [];
    if (fg) cls.push("ansi-" + fg);
    if (bg) cls.push("ansi-bg-" + bg);
    if (bold) cls.push("ansi-bold");
    if (dim) cls.push("ansi-dim");
    if (italic) cls.push("ansi-italic");
    if (underline) cls.push("ansi-underline");
    const t = escape(text);
    out += cls.length ? `<span class="${cls.join(" ")}">${t}</span>` : t;
  };

  // eslint-disable-next-line no-control-regex
  const re = /\x1b\[([0-9;?]*)([A-Za-z])|\x1b\][^\x07\x1b]*(?:\x07|\x1b\\)/g;
  let last = 0;
  let m: RegExpExecArray | null;
  while ((m = re.exec(input))) {
    flush(input.slice(last, m.index));
    last = re.lastIndex;
    if (m[2] !== "m") continue; // not SGR: drop it
    const codes = m[1] === "" ? [0] : m[1].split(";").map((n) => parseInt(n, 10) || 0);
    for (let i = 0; i < codes.length; i++) {
      const c = codes[i];
      if (c === 0) [fg, bg, bold, dim, italic, underline] = ["", "", false, false, false, false];
      else if (c === 1) bold = true;
      else if (c === 2) dim = true;
      else if (c === 3) italic = true;
      else if (c === 4) underline = true;
      else if (c === 22) bold = dim = false;
      else if (c === 23) italic = false;
      else if (c === 24) underline = false;
      else if (c >= 30 && c <= 37) fg = names[c - 30];
      else if (c === 39) fg = "";
      else if (c >= 40 && c <= 47) bg = names[c - 40];
      else if (c === 49) bg = "";
      else if (c >= 90 && c <= 97) fg = "bright-" + names[c - 90];
      else if (c >= 100 && c <= 107) bg = "bright-" + names[c - 100];
      else if (c === 38 || c === 48) i += codes[i + 1] === 5 ? 2 : 4; // skip extended colour
    }
  }
  flush(input.slice(last));
  return out;
}

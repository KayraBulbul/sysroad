import type { Theme } from "./api";

// Applies an Omarchy palette as --c-<key> variables on :root. app.css derives
// every other colour from these, so switching themes is one style pass.
export function applyTheme(t: Theme) {
  const root = document.documentElement;
  const old: string[] = [];
  for (let i = 0; i < root.style.length; i++) old.push(root.style.item(i));
  for (const name of old) if (name.startsWith("--c-")) root.style.removeProperty(name);
  for (const [k, v] of Object.entries(t.colors ?? {})) {
    if (v) root.style.setProperty(`--c-${k}`, v);
  }
  root.dataset.mode = t.mode === "light" ? "light" : "dark";
}

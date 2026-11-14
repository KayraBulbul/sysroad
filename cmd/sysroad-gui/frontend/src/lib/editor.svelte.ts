import { Annotation, Compartment, EditorState, Prec, type Extension, type Transaction } from "@codemirror/state";
import {
  EditorView,
  GutterMarker,
  crosshairCursor,
  drawSelection,
  dropCursor,
  gutter,
  highlightActiveLine,
  highlightActiveLineGutter,
  highlightSpecialChars,
  keymap,
  rectangularSelection,
} from "@codemirror/view";
import { defaultKeymap, history, historyKeymap, indentWithTab } from "@codemirror/commands";
import { bracketMatching, indentOnInput, indentUnit, syntaxHighlighting } from "@codemirror/language";
import {
  acceptCompletion,
  autocompletion,
  closeBrackets,
  closeBracketsKeymap,
  closeCompletion,
  moveCompletionSelection,
  startCompletion,
} from "@codemirror/autocomplete";
import { highlightSelectionMatches, searchKeymap } from "@codemirror/search";
import { diff, unifiedMergeView } from "@codemirror/merge";
import { cpp } from "@codemirror/lang-cpp";
import { Vim, getCM, vim } from "@replit/codemirror-vim";
import { Service, errText, type File, type FileEvent } from "./api";
import { highlighter } from "./highlight";
import { lessonCompletions } from "./complete";

// ---- the editor workspace ----
//
// One EditorView for the whole app. Each file keeps its own EditorState
// (undo history, cursor, folds), so switching tabs or lessons is a setState,
// not a rebuild. Files autosave shortly after you stop typing; every save
// says which content it started from, and the Go side refuses to overwrite a
// file that changed underneath (the other machine syncing, an edit in nvim).

const external = Annotation.define<boolean>();

export type Buffer = {
  key: string;
  lesson: string;
  name: string;
  state: EditorState;
  base: string; // hash of the disk content this buffer started from
  saved: string; // content last known to be on disk
  onDisk: boolean;
  dirty: boolean;
  conflict: File | null; // disk content that differs from ours; onDisk false: the file was deleted
  readOnly: boolean; // a solution, or a file the lesson gives you (its header)
  solution: boolean;
  timer?: ReturnType<typeof setTimeout>;
  saving?: Promise<void> | null;
  again?: boolean;
};

export type Tab = { key: string; name: string; kind: "file" | "solution" | "diff"; readOnly: boolean; dirty: boolean; conflict: boolean; gone: boolean };

export type Actions = {
  run: () => void;
  submit: () => void;
  hint: () => void;
  solution: () => void;
  reset: () => void;
  uth: () => void;
  format: () => void;
  next: () => void;
  prev: () => void;
  focusLesson: () => void;
  focusOutput: () => void;
  palette: () => void;
};

const AUTOSAVE_MS = 500;

class RelNumber extends GutterMarker {
  constructor(
    readonly text: string,
    readonly current: boolean,
  ) {
    super();
  }
  eq(o: RelNumber) {
    return o.text === this.text && o.current === this.current;
  }
  toDOM() {
    const s = document.createElement("span");
    s.textContent = this.text;
    if (this.current) s.className = "cur";
    return s;
  }
}

// Relative line numbers with the absolute one on the cursor line, as in
// LazyVim (number + relativenumber).
const relativeNumbers = gutter({
  class: "cm-lineNumbers cm-relnums",
  lineMarker(view, line) {
    const cur = view.state.doc.lineAt(view.state.selection.main.head).number;
    const n = view.state.doc.lineAt(line.from).number;
    return new RelNumber(n === cur ? String(n) : String(Math.abs(n - cur)), n === cur);
  },
  lineMarkerChange: (u) => u.selectionSet || u.docChanged,
  initialSpacer: () => new RelNumber("000", false),
});

const theme = EditorView.theme({
  "&": { height: "100%", color: "var(--fg)", backgroundColor: "var(--bg)", fontSize: "13.5px" },
  ".cm-scroller": { fontFamily: "var(--font)", lineHeight: "1.6", overflow: "auto" },
  ".cm-content": { caretColor: "var(--accent)", padding: "10px 0" },
  ".cm-cursor, .cm-dropCursor": { borderLeftColor: "var(--accent)", borderLeftWidth: "2px" },
  ".cm-fat-cursor": { background: "color-mix(in oklab, var(--accent) 75%, transparent) !important", color: "var(--bg) !important" },
  "&:not(.cm-focused) .cm-fat-cursor": { background: "none !important", outline: "1px solid var(--accent) !important" },
  "&.cm-focused .cm-selectionBackground, .cm-selectionBackground, .cm-content ::selection": {
    backgroundColor: "var(--selection) !important",
  },
  ".cm-activeLine": { backgroundColor: "color-mix(in oklab, var(--fg) 4.5%, transparent)" },
  ".cm-gutters": { backgroundColor: "var(--bg)", color: "var(--fg-dim)", border: "none", paddingLeft: "6px" },
  ".cm-relnums .cm-gutterElement": { padding: "0 14px 0 6px", minWidth: "3ch", textAlign: "right", opacity: "0.55" },
  ".cm-relnums .cm-gutterElement .cur": { color: "var(--accent)", opacity: "1" },
  ".cm-activeLineGutter": { backgroundColor: "transparent" },
  ".cm-activeLineGutter.cm-gutterElement": { opacity: "1" },
  ".cm-matchingBracket": { backgroundColor: "var(--line-strong)", outline: "none", color: "var(--fg-bright)" },
  ".cm-selectionMatch": { backgroundColor: "color-mix(in oklab, var(--fg) 10%, transparent)" },
  ".cm-searchMatch": { backgroundColor: "color-mix(in oklab, var(--yellow) 30%, transparent)", outline: "none" },
  ".cm-searchMatch-selected": { backgroundColor: "color-mix(in oklab, var(--accent) 40%, transparent)" },
  ".cm-panels": { backgroundColor: "var(--bg-dark)", color: "var(--fg)", borderTop: "1px solid var(--line)" },
  ".cm-panels-bottom": { borderTop: "1px solid var(--line)" },
  ".cm-vim-panel": { padding: "3px 12px", fontFamily: "var(--font)", fontSize: "12.5px", minHeight: "24px" },
  ".cm-vim-panel input": { fontFamily: "var(--font)", color: "var(--fg-bright)", caretColor: "var(--accent)" },
  ".cm-tooltip": { backgroundColor: "var(--bg-dark)", border: "1px solid var(--line-strong)", color: "var(--fg)", borderRadius: "6px" },
  ".cm-tooltip.cm-tooltip-autocomplete > ul": { fontFamily: "var(--font)", fontSize: "12.5px", maxHeight: "16em", padding: "3px" },
  ".cm-tooltip.cm-tooltip-autocomplete > ul > li": { padding: "1px 8px 1px 4px", borderRadius: "4px", lineHeight: "1.6" },
  ".cm-tooltip.cm-tooltip-autocomplete > ul > li[aria-selected]": { background: "var(--accent-soft)", color: "var(--fg-bright)" },
  ".cm-completionLabel": { color: "var(--fg)" },
  ".cm-completionMatchedText": { textDecoration: "none", color: "var(--accent)", fontWeight: "600" },
  ".cm-completionIcon": { width: "1.4em", opacity: "0.9", paddingRight: "0.5em", fontSize: "11px" },
  ".cm-completionIcon-variable": { color: "var(--blue)" },
  ".cm-completionIcon-function, .cm-completionIcon-method": { color: "var(--magenta)" },
  ".cm-completionIcon-type": { color: "var(--yellow)" },
  ".cm-completionIcon-namespace": { color: "var(--cyan)" },
  ".cm-completionIcon-keyword": { color: "var(--red)" },
  ".cm-completionIcon-property": { color: "var(--green)" },
  ".cm-changedLine": { backgroundColor: "color-mix(in oklab, var(--green) 9%, transparent) !important" },
  ".cm-changedText": { background: "color-mix(in oklab, var(--green) 28%, transparent) !important" },
  ".cm-deletedChunk": { backgroundColor: "color-mix(in oklab, var(--red) 10%, transparent) !important" },
  ".cm-deletedChunk .cm-deletedText, .cm-deletedChunk del": { background: "color-mix(in oklab, var(--red) 28%, transparent) !important", textDecoration: "none" },
  ".cm-changeGutter": { width: "3px", paddingLeft: "0" },
  ".cm-changedLineGutter": { background: "var(--green)" },
  ".cm-deletedLineGutter": { background: "var(--red)" },
});

function minimalChange(from: string, to: string) {
  let start = 0;
  const max = Math.min(from.length, to.length);
  while (start < max && from.charCodeAt(start) === to.charCodeAt(start)) start++;
  let endA = from.length;
  let endB = to.length;
  while (endA > start && endB > start && from.charCodeAt(endA - 1) === to.charCodeAt(endB - 1)) {
    endA--;
    endB--;
  }
  return { from: start, to: endA, insert: to.slice(start, endB) };
}

export class Workspace {
  view: EditorView | null = null;
  lesson = $state("");
  tabs = $state<Tab[]>([]);
  active = $state(0);
  vimMode = $state(true);
  vimStatus = $state("");
  loading = $state(false);

  private bufs = new Map<string, Buffer>();
  private headers = new Map<string, string>(); // a project lesson's contract header, shown first
  private openGen = 0; // bumps on every open(), so a superseded one gives up
  private vimConf = new Compartment();
  private actions: Actions | null = null;
  private onError: (msg: string) => void = () => {};

  constructor() {
    this.defineVim();
  }

  mount(parent: HTMLElement, actions: Actions, onError: (msg: string) => void, vimOn: boolean) {
    this.actions = actions;
    this.onError = onError;
    this.vimMode = vimOn;
    this.view = new EditorView({ parent, state: EditorState.create({ doc: "", extensions: this.extensions(true) }) });
    if (this.pending) {
      const l = this.pending;
      this.pending = "";
      this.open(l);
    }
  }

  private pending = ""; // a lesson opened before the editor was mounted

  destroy() {
    this.view?.destroy();
    this.view = null;
  }

  private extensions(readOnly: boolean, extra: Extension[] = []): Extension[] {
    return [
      this.vimConf.of(this.vimMode ? vim({ status: true }) : []),
      relativeNumbers,
      highlightActiveLineGutter(),
      highlightSpecialChars(),
      history(),
      drawSelection(),
      dropCursor(),
      EditorState.allowMultipleSelections.of(true),
      indentOnInput(),
      indentUnit.of("    "),
      EditorState.tabSize.of(4),
      syntaxHighlighting(highlighter),
      bracketMatching(),
      closeBrackets(),
      rectangularSelection(),
      crosshairCursor(),
      highlightActiveLine(),
      highlightSelectionMatches(),
      autocompletion({
        override: [lessonCompletions(() => this.otherStates())],
        defaultKeymap: false,
        icons: true,
        maxRenderedOptions: 40,
        activateOnTypingDelay: 0,
      }),
      // blink.cmp-style keys, ahead of vim and the default keymap. Each one
      // does nothing (returns false) when no completion is open.
      Prec.highest(
        keymap.of([
          { key: "Ctrl-Space", run: startCompletion },
          { key: "Ctrl-n", run: moveCompletionSelection(true) },
          { key: "Ctrl-p", run: moveCompletionSelection(false) },
          { key: "ArrowDown", run: moveCompletionSelection(true) },
          { key: "ArrowUp", run: moveCompletionSelection(false) },
          { key: "PageDown", run: moveCompletionSelection(true, "page") },
          { key: "PageUp", run: moveCompletionSelection(false, "page") },
          { key: "Enter", run: acceptCompletion },
          { key: "Ctrl-y", run: acceptCompletion },
          { key: "Tab", run: acceptCompletion },
          // Close the menu and still let vim leave insert mode.
          { key: "Escape", run: (v) => (closeCompletion(v), false) },
        ]),
      ),
      keymap.of([...closeBracketsKeymap, ...defaultKeymap, ...searchKeymap, ...historyKeymap, indentWithTab]),
      cpp(),
      theme,
      EditorState.readOnly.of(readOnly),
      EditorView.updateListener.of((u) => {
        if (!u.docChanged && !u.selectionSet) return;
        const buf = this.current();
        if (!buf || u.state !== this.view?.state) return;
        buf.state = u.state;
        if (u.docChanged && !u.transactions.some((tr: Transaction) => tr.annotation(external)) && !buf.readOnly) {
          this.markDirty(buf);
        }
      }),
      ...extra,
    ];
  }

  // ---- vim ----

  private defineVim() {
    Vim.defineEx("write", "w", () => this.saveActive());
    Vim.defineEx("wall", "wa", () => this.flush());
    // Yanks also go to the system clipboard (LazyVim's clipboard=unnamedplus).
    const regs = Vim.getRegisterController();
    const push = regs.pushText.bind(regs);
    regs.pushText = (reg, op, text, linewise, blockwise) => {
      push(reg, op, text, linewise, blockwise);
      if (op === "yank" && (!reg || reg === "+" || reg === "*")) navigator.clipboard?.writeText(text).catch(() => {});
    };
  }

  setVim(on: boolean) {
    this.vimMode = on;
    this.view?.dispatch({ effects: this.vimConf.reconfigure(on ? vim({ status: true }) : []) });
  }

  // leader is the LazyVim-style <space> leader: in vim's normal mode with
  // nothing pending, Space waits for the next key. It's handled before vim
  // sees the keys, because vim's own Space motion would win over a mapping.
  leaderPending = $state(false);
  private leaderBuf = "";
  private leaderTimer: ReturnType<typeof setTimeout> | undefined;

  static leaderKeys: Record<string, keyof Actions> = {
    cf: "format",
    r: "run",
    s: "submit",
    h: "hint",
    v: "solution",
    R: "reset",
    u: "uth",
    n: "next",
    p: "prev",
    " ": "palette",
  };

  // handleLeader takes a keydown; true means it was ours. In the editor the
  // leader only starts in vim's normal mode; outside it (inEditor false)
  // Space always starts it.
  handleLeader(e: KeyboardEvent, inEditor = true): boolean {
    if (e.ctrlKey || e.metaKey || e.altKey) return false;
    if (this.leaderPending) {
      if (["Shift", "CapsLock"].includes(e.key)) return true;
      const seq = this.leaderBuf + e.key;
      const keys = Object.keys(Workspace.leaderKeys);
      if (keys.some((k) => k !== seq && k.startsWith(seq))) {
        this.leaderBuf = seq; // a longer sequence (cf) may follow
        this.armLeader();
        return true;
      }
      this.endLeader();
      const name = Workspace.leaderKeys[seq];
      if (name) this.actions?.[name]();
      return true; // an unknown key after the leader is dropped, as in vim
    }
    if (e.key !== " " || e.shiftKey || (inEditor && !this.vimIdle())) return false;
    this.leaderPending = true;
    this.leaderBuf = "";
    this.armLeader();
    return true;
  }

  private armLeader() {
    clearTimeout(this.leaderTimer);
    this.leaderTimer = setTimeout(() => this.endLeader(), 1500);
  }

  private endLeader() {
    clearTimeout(this.leaderTimer);
    this.leaderPending = false;
    this.leaderBuf = "";
  }

  // vimIdle: vim is on, in normal mode, with no operator or count pending.
  private vimIdle(): boolean {
    if (!this.vimMode || !this.view) return false;
    const st = getCM(this.view)?.state?.vim;
    if (!st || st.insertMode || st.visualMode) return false;
    const input = st.inputState;
    return !input || (!input.operator && !input.motion && !input.keyBuffer?.length && !input.prefixRepeat?.length && !input.motionRepeat?.length && !input.selectedCharacter);
  }

  // insertMode is true while vim is inserting (or vim is off), when plain
  // keys belong to the text.
  insertMode(): boolean {
    if (!this.vimMode || !this.view) return true;
    return !!getCM(this.view)?.state?.vim?.insertMode;
  }

  // ---- buffers ----

  private current(): Buffer | undefined {
    const t = this.tabs[this.active];
    return t ? this.bufs.get(t.key) : undefined;
  }

  private newState(doc: string, readOnly: boolean, extra: Extension[] = []) {
    return EditorState.create({ doc, extensions: this.extensions(readOnly, extra) });
  }

  private tabFor(buf: Buffer): Tab {
    return { key: buf.key, name: buf.name, kind: "file", readOnly: buf.readOnly, dirty: buf.dirty, conflict: !!buf.conflict, gone: buf.conflict?.onDisk === false };
  }

  private fileBuffer(lesson: string, f: File): Buffer {
    const readOnly = !!f.readOnly;
    return { key: `${lesson}/${f.name}`, lesson, name: f.name, state: this.newState(f.content, readOnly), base: f.hash, saved: f.content, onDisk: f.onDisk, dirty: false, conflict: null, readOnly, solution: false };
  }

  // open shows a lesson's files. It switches at once: buffers you already
  // had for the lesson show straight away (history and cursor included) and
  // the old lesson's pending saves finish in the background. Then it fetches
  // the files from Go and reconciles, unless another open() came after it,
  // so quick <leader>n/p presses always end on the last lesson.
  async open(lesson: string) {
    if (!this.view) {
      this.pending = lesson;
      return;
    }
    const gen = ++this.openGen;
    if (lesson !== this.lesson) {
      void this.flushLesson(this.lesson);
      this.lesson = lesson;
      this.showCached(lesson);
    }
    this.loading = true;
    let files: File[] = [];
    let listed = true;
    try {
      files = (await Service.OpenFiles(lesson)) ?? [];
    } catch (e) {
      listed = false;
      if (gen === this.openGen) this.onError(errText(e));
    }
    if (gen !== this.openGen) return; // moved on meanwhile
    this.loading = false;
    Service.Watch(lesson).catch(() => {});
    const tabs: Tab[] = [];
    for (const f of files) {
      const key = `${lesson}/${f.name}`;
      let buf = this.bufs.get(key);
      if (!buf) {
        buf = this.fileBuffer(lesson, f);
        this.bufs.set(key, buf);
      } else {
        if (f.hash !== buf.base) this.diskChanged(buf, f);
        // A project lesson you just passed: its files are read-only now.
        if (!!f.readOnly !== buf.readOnly && !buf.dirty) this.setReadOnly(buf, !!f.readOnly);
      }
      tabs.push(this.tabFor(buf));
    }
    // A project file that went while you were away: let it go, unless you
    // have changes in it.
    const names = new Set(files.map((f) => `${lesson}/${f.name}`));
    for (const b of listed ? [...this.bufs.values()] : []) {
      if (b.lesson !== lesson || b.solution || names.has(b.key)) continue;
      if (b.dirty) {
        b.conflict = this.goneFile(b);
        tabs.push(this.tabFor(b));
      } else {
        clearTimeout(b.timer);
        this.bufs.delete(b.key);
      }
    }
    // Keep a solution tab that was open.
    for (const t of this.tabs) if (t.kind !== "file" && this.bufs.has(t.key)) tabs.push(t);
    this.tabs = tabs;
    this.select(Math.min(this.active, Math.max(tabs.length - 1, 0)));
  }

  // showCached shows the buffers already open for a lesson, or an empty
  // editor, without waiting for Go.
  private showCached(lesson: string) {
    const tabs: Tab[] = [];
    for (const b of this.bufs.values()) {
      if (b.lesson === lesson && !b.solution) tabs.push(this.tabFor(b));
    }
    this.tabs = tabs;
    this.active = 0;
    if (tabs.length) {
      this.active = -1;
      this.select(0);
    } else {
      this.view?.setState(this.newState("", true));
    }
  }

  select(i: number) {
    const t = this.tabs[i];
    if (!t || !this.view) return;
    const buf = this.bufs.get(t.key);
    if (!buf) return;
    this.active = i;
    if (this.view.state !== buf.state) this.view.setState(buf.state);
  }

  focus() {
    this.view?.focus();
  }

  private sync(buf: Buffer) {
    const t = this.tabs.find((x) => x.key === buf.key);
    if (t) {
      t.dirty = buf.dirty;
      t.conflict = !!buf.conflict;
      t.gone = buf.conflict?.onDisk === false;
    }
  }

  private setReadOnly(buf: Buffer, readOnly: boolean) {
    buf.readOnly = readOnly;
    buf.state = this.newState(buf.state.doc.toString(), readOnly);
    if (this.current() === buf && this.view) this.view.setState(buf.state);
    const t = this.tabs.find((x) => x.key === buf.key);
    if (t) t.readOnly = readOnly;
  }

  // ---- a project's files: you make, rename and delete them ----

  // setHeader names a project lesson's contract header, which sorts first.
  setHeader(lesson: string, header: string) {
    this.headers.set(lesson, header);
  }

  // fileTabs counts the open lesson's files (not the solution's).
  fileTabs(): number {
    return this.tabs.filter((t) => t.kind === "file").length;
  }

  hasFile(lesson: string, name: string): boolean {
    return this.bufs.has(`${lesson}/${name}`);
  }

  activeFile(): string {
    const t = this.tabs[this.active];
    return t?.kind === "file" ? t.name : "";
  }

  // placeTab puts a file's tab where the Go side lists it: the header
  // first, then by name, before any solution tabs.
  private placeTab(lesson: string, tab: Tab) {
    const header = this.headers.get(lesson) ?? "";
    const before = (a: string, b: string) => (a === header ? b !== header : b === header ? false : a < b);
    const name = this.bufs.get(tab.key)?.name ?? tab.name;
    let i = this.tabs.findIndex((t) => t.kind !== "file" || before(name, t.name));
    if (i < 0) i = this.tabs.length;
    this.tabs.splice(i, 0, tab);
    return i;
  }

  // addFile opens a file that's new in the folder: made here (focus it) or
  // in a terminal next to the app.
  addFile(lesson: string, f: File, focus: boolean) {
    if (lesson !== this.lesson) return;
    const key = `${lesson}/${f.name}`;
    if (this.bufs.has(key)) {
      const i = this.tabs.findIndex((t) => t.key === key);
      if (focus && i >= 0) this.select(i);
      return;
    }
    const buf = this.fileBuffer(lesson, f);
    this.bufs.set(key, buf);
    const activeKey = this.tabs[this.active]?.key;
    const i = this.placeTab(lesson, this.tabFor(buf));
    if (focus || this.fileTabs() === 1) {
      this.active = -1;
      this.select(i);
      if (focus) this.focus();
    } else {
      this.active = this.tabs.findIndex((t) => t.key === activeKey);
    }
  }

  // removeFile closes a file that's gone from the folder.
  removeFile(lesson: string, name: string) {
    const key = `${lesson}/${name}`;
    const buf = this.bufs.get(key);
    if (buf) clearTimeout(buf.timer);
    this.bufs.delete(key);
    const i = this.tabs.findIndex((t) => t.key === key);
    if (i < 0) return;
    const activeKey = this.tabs[this.active]?.key;
    this.tabs.splice(i, 1);
    if (activeKey !== key) {
      this.active = this.tabs.findIndex((t) => t.key === activeKey);
      return;
    }
    this.active = -1;
    if (this.tabs.length) this.select(Math.min(i, this.tabs.length - 1));
    else this.view?.setState(this.newState("", true));
  }

  // renamed moves a file's buffer (history, cursor) to its new name.
  renamed(lesson: string, from: string, f: File) {
    const buf = this.bufs.get(`${lesson}/${from}`);
    if (!buf) return;
    const wasActive = this.tabs[this.active]?.key === buf.key;
    const activeKey = this.tabs[this.active]?.key;
    this.tabs = this.tabs.filter((t) => t.key !== buf.key);
    this.bufs.delete(buf.key);
    buf.key = `${lesson}/${f.name}`;
    buf.name = f.name;
    buf.base = f.hash;
    buf.saved = f.content;
    buf.onDisk = true;
    this.bufs.set(buf.key, buf);
    const i = this.placeTab(lesson, this.tabFor(buf));
    this.active = wasActive ? i : this.tabs.findIndex((t) => t.key === activeKey);
  }

  // saveFile saves one file now (before it's renamed).
  async saveFile(lesson: string, name: string) {
    const buf = this.bufs.get(`${lesson}/${name}`);
    if (buf && !buf.readOnly && (buf.dirty || buf.saving)) await this.save(buf);
    return !buf?.conflict;
  }

  private goneFile(buf: Buffer): File {
    return { name: buf.name, content: "", hash: "", onDisk: false, readOnly: false };
  }

  private markDirty(buf: Buffer) {
    buf.dirty = true;
    this.sync(buf);
    clearTimeout(buf.timer);
    buf.timer = setTimeout(() => this.save(buf), AUTOSAVE_MS);
  }

  private save(buf: Buffer): Promise<void> {
    clearTimeout(buf.timer);
    if (buf.conflict || buf.readOnly) return Promise.resolve();
    if (buf.saving) {
      buf.again = true;
      return buf.saving;
    }
    buf.saving = (async () => {
      do {
        buf.again = false;
        const content = buf.state.doc.toString();
        if (content === buf.saved && buf.onDisk) {
          buf.dirty = false;
          continue;
        }
        try {
          const r = await Service.SaveFile(buf.lesson, buf.name, content, buf.base);
          if (!r) break;
          if (r.conflict && r.disk) {
            buf.conflict = r.disk;
            break;
          }
          buf.base = r.hash;
          buf.saved = content;
          buf.onDisk = true;
          buf.dirty = buf.state.doc.toString() !== content;
        } catch (e) {
          this.onError(`Couldn't save ${buf.name}: ${errText(e)}`);
          break;
        }
      } while (buf.again);
    })().finally(() => {
      buf.saving = null;
      this.sync(buf);
    });
    return buf.saving;
  }

  // otherStates are the lesson's other files (headers included), for completion.
  private otherStates(): EditorState[] {
    const cur = this.current();
    const out: EditorState[] = [];
    for (const b of this.bufs.values()) {
      if (b !== cur && b.lesson === this.lesson && !b.solution) out.push(b.state);
    }
    return out;
  }

  // format runs clang-format on the open file and applies the result as
  // edits, so the cursor stays put and undo takes it back in one step.
  async format(): Promise<"formatted" | "unchanged" | "skipped"> {
    const buf = this.current();
    if (!buf || buf.readOnly || !this.view) return "skipped";
    const before = buf.state.doc.toString();
    const after = await Service.Format(buf.lesson, buf.name, before);
    if (buf.state.doc.toString() !== before) return "skipped"; // you typed meanwhile
    if (after === before) return "unchanged";
    const changes = diff(before, after).map((c) => ({ from: c.fromA, to: c.toA, insert: after.slice(c.fromB, c.toB) }));
    if (this.current() === buf && this.view.state === buf.state) {
      this.view.dispatch({ changes, userEvent: "input.format" });
    } else {
      buf.state = buf.state.update({ changes, userEvent: "input.format" }).state;
      this.markDirty(buf);
    }
    return "formatted";
  }

  saveActive() {
    const b = this.current();
    if (b) this.save(b);
  }

  // flush saves every pending change in the open lesson (before a run).
  flush() {
    return this.flushLesson(this.lesson);
  }

  private async flushLesson(lesson: string) {
    const pending: Promise<void>[] = [];
    for (const b of this.bufs.values()) {
      if (b.lesson === lesson && !b.readOnly && (b.dirty || b.saving)) pending.push(this.save(b));
    }
    await Promise.all(pending);
  }

  conflicted(): boolean {
    for (const b of this.bufs.values()) if (b.lesson === this.lesson && b.conflict) return true;
    return false;
  }

  // ---- changes from disk ----

  private replaceDoc(buf: Buffer, content: string) {
    const change = minimalChange(buf.state.doc.toString(), content);
    const tr = buf.state.update({ changes: change, annotations: [external.of(true)] });
    buf.state = tr.state;
    if (this.current() === buf && this.view) this.view.setState(buf.state);
  }

  private diskChanged(buf: Buffer, f: File) {
    if (f.hash === buf.base) return;
    const mine = buf.state.doc.toString();
    if (mine === f.content) {
      buf.base = f.hash;
      buf.saved = mine;
      buf.onDisk = true;
      buf.conflict = null;
    } else if (!buf.dirty && !buf.saving) {
      this.replaceDoc(buf, f.content);
      buf.base = f.hash;
      buf.saved = f.content;
      buf.onDisk = true;
    } else {
      buf.conflict = f;
    }
    this.sync(buf);
  }

  onFileEvent(ev: FileEvent) {
    const buf = this.bufs.get(`${ev.lesson}/${ev.name}`);
    if (ev.removed) {
      if (buf) this.diskRemoved(buf);
    } else if (buf) {
      this.diskChanged(buf, ev);
    } else if (ev.lesson === this.lesson && !this.loading) {
      this.addFile(ev.lesson, ev, false); // made outside the app
    }
  }

  // diskRemoved: a project file was deleted outside the app. A clean one
  // closes; one with changes stays, asking whether to let it go.
  private diskRemoved(buf: Buffer) {
    if (buf.solution) return;
    if (!buf.dirty && !buf.saving) {
      this.removeFile(buf.lesson, buf.name);
      return;
    }
    buf.conflict = this.goneFile(buf);
    this.sync(buf);
  }

  // resolve settles a conflict: take the disk's version, or keep yours and
  // write it over the disk's.
  resolve(choice: "reload" | "keep") {
    const buf = this.current();
    const disk = buf?.conflict;
    if (!buf || !disk) return;
    if (!disk.onDisk) {
      // Deleted on disk: let it go, or write yours back.
      buf.conflict = null;
      if (choice === "reload") {
        this.removeFile(buf.lesson, buf.name);
        return;
      }
      buf.base = "";
      buf.onDisk = false;
      buf.dirty = true;
      this.sync(buf);
      this.save(buf);
      return;
    }
    buf.conflict = null;
    buf.base = disk.hash;
    if (choice === "reload") {
      clearTimeout(buf.timer);
      this.replaceDoc(buf, disk.content);
      buf.saved = disk.content;
      buf.onDisk = true;
      buf.dirty = false;
      this.sync(buf);
    } else {
      buf.saved = disk.content;
      buf.dirty = true;
      this.sync(buf);
      this.save(buf);
    }
  }

  // replaceAll swaps in fresh files after a Reset or catching up (no undo
  // across it). In a project the files may differ from the ones open.
  replaceAll(lesson: string, files: File[]) {
    for (const b of [...this.bufs.values()]) {
      if (b.lesson === lesson && !b.solution) {
        clearTimeout(b.timer);
        this.bufs.delete(b.key);
      }
    }
    for (const f of files) this.bufs.set(`${lesson}/${f.name}`, this.fileBuffer(lesson, f));
    if (lesson === this.lesson) {
      for (const t of this.tabs) if (t.kind !== "file") this.bufs.delete(t.key);
      this.tabs = files.map((f) => this.tabFor(this.bufs.get(`${lesson}/${f.name}`)!));
      const a = this.active < this.tabs.length ? Math.max(this.active, 0) : 0;
      this.active = -1;
      if (this.tabs.length) this.select(a);
      else this.view?.setState(this.newState("", true));
    }
  }

  // ---- solution ----

  // showSolution opens the solution read-only, or (diff) your code with the
  // solution's changes marked against it.
  showSolution(files: File[], diff: boolean) {
    const tabs = this.tabs.filter((t) => t.kind === "file");
    for (const f of files) {
      const key = `${this.lesson}/~solution/${f.name}`;
      const mine = this.bufs.get(`${this.lesson}/${f.name}`);
      const state = this.newState(
        f.content,
        true,
        diff && mine ? [unifiedMergeView({ original: mine.state.doc.toString(), mergeControls: false, highlightChanges: true, gutter: true })] : [],
      );
      this.bufs.set(key, { key, lesson: this.lesson, name: f.name, state, base: f.hash, saved: f.content, onDisk: false, dirty: false, conflict: null, readOnly: true, solution: true });
      tabs.push({ key, name: f.name, kind: diff ? "diff" : "solution", readOnly: true, dirty: false, conflict: false, gone: false });
    }
    this.tabs = tabs;
    const first = tabs.findIndex((t) => t.kind !== "file");
    this.active = -1;
    this.select(first >= 0 ? first : 0);
  }

  hideSolution() {
    for (const t of this.tabs) if (t.kind !== "file") this.bufs.delete(t.key);
    this.tabs = this.tabs.filter((t) => t.kind === "file");
    this.active = -1;
    if (this.tabs.length) this.select(0);
    else this.view?.setState(this.newState("", true));
  }

  solutionOpen() {
    return this.tabs.some((t) => t.kind !== "file");
  }
}

export const workspace = new Workspace();

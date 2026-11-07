import { Service, errText, type LessonDetail, type Snapshot, type LessonRow, type PhaseDTO, type ChapterDTO } from "./api";

export type Route = { view: "dashboard" } | { view: "phase"; id: string } | { view: "lesson"; id: string };

export type Toast = { id: number; text: string; kind: "info" | "good" | "bad" | "xp" };

export type Confirm = { title: string; detail?: string; yes: string; danger?: boolean; onYes: () => void };

// All UI state in one place. The whole course (titles, statuses) arrives in
// one Snapshot at startup, so moving between pages never waits on Go.
export const ui = $state({
  snap: null as Snapshot | null,
  route: { view: "dashboard" } as Route,
  lessons: {} as Record<string, LessonDetail>,
  prefs: {} as Record<string, unknown>,
  palette: false,
  confirm: null as Confirm | null,
  toasts: [] as Toast[],
  loadError: "",
});

// ---- lookups over the snapshot ----

type Where = { phase: PhaseDTO; chapter: ChapterDTO; row: LessonRow };
let index = new Map<string, Where>();
let indexed: Snapshot | null = null;

function reindex() {
  if (indexed === ui.snap) return;
  index = new Map();
  for (const phase of ui.snap?.phases ?? []) {
    for (const chapter of phase.chapters ?? []) {
      for (const row of chapter.lessons ?? []) index.set(row.id, { phase, chapter, row });
    }
  }
  indexed = ui.snap;
}

export function where(id: string): Where | undefined {
  reindex();
  return index.get(id);
}

export function phase(id: string): PhaseDTO | undefined {
  return ui.snap?.phases?.find((p) => p.id === id);
}

// ---- loading ----

let refreshing: Promise<void> | null = null;
let again = false;

// refresh refetches the snapshot; calls while one is in flight coalesce.
export function refresh(): Promise<void> {
  if (refreshing) {
    again = true;
    return refreshing;
  }
  refreshing = (async () => {
    try {
      do {
        again = false;
        ui.snap = await Service.Snapshot();
      } while (again);
    } catch (e) {
      ui.loadError = errText(e);
    } finally {
      refreshing = null;
    }
  })();
  return refreshing;
}

export async function loadLesson(id: string): Promise<LessonDetail | undefined> {
  try {
    const d = await Service.Lesson(id);
    if (d) ui.lessons[id] = d;
    return d ?? undefined;
  } catch (e) {
    toast(errText(e), "bad");
  }
}

// ---- navigation ----

// Back/forward history, for the mouse's side buttons and Alt+←/→.
const backStack: Route[] = [];
const fwdStack: Route[] = [];

function sameRoute(a: Route, b: Route) {
  return a.view === b.view && ("id" in a ? a.id : "") === ("id" in b ? b.id : "");
}

export function go(r: Route, record = true) {
  if (record && !sameRoute(ui.route, r)) {
    backStack.push($state.snapshot(ui.route) as Route);
    if (backStack.length > 100) backStack.shift();
    fwdStack.length = 0;
  }
  if (r.view === "lesson") {
    if (!ui.lessons[r.id]) loadLesson(r.id);
    savePref("lastLesson", r.id);
  }
  ui.route = r;
}

export function back() {
  const r = backStack.pop();
  if (!r) return;
  fwdStack.push($state.snapshot(ui.route) as Route);
  go(r, false);
}

export function forward() {
  const r = fwdStack.pop();
  if (!r) return;
  backStack.push($state.snapshot(ui.route) as Route);
  go(r, false);
}

export function openLesson(id: string) {
  go({ view: "lesson", id });
}

// up goes one level towards the dashboard: lesson → phase → dashboard.
export function up() {
  const r = ui.route;
  if (r.view === "lesson") {
    const w = where(r.id);
    go(w ? { view: "phase", id: w.phase.id } : { view: "dashboard" });
  } else if (r.view === "phase") {
    go({ view: "dashboard" });
  }
}

// ---- toasts and confirms ----

let toastID = 0;

export function toast(text: string, kind: Toast["kind"] = "info", ms = 4200) {
  const t = { id: ++toastID, text, kind };
  ui.toasts = [...ui.toasts.slice(-3), t];
  setTimeout(() => (ui.toasts = ui.toasts.filter((x) => x.id !== t.id)), ms);
}

export function fail(e: unknown) {
  toast(errText(e), "bad", 6000);
}

export function ask(c: Confirm) {
  ui.confirm = c;
}

// ---- prefs (this machine's .local/gui.json) ----

let saveTimer: ReturnType<typeof setTimeout> | undefined;

export async function loadPrefs() {
  try {
    ui.prefs = (await Service.Prefs()) ?? {};
  } catch {
    ui.prefs = {};
  }
}

export function pref<T>(key: string, fallback: T): T {
  const v = ui.prefs[key];
  return v === undefined || v === null ? fallback : (v as T);
}

export function savePref(key: string, value: unknown) {
  if (ui.prefs[key] === value) return;
  ui.prefs[key] = value;
  clearTimeout(saveTimer);
  saveTimer = setTimeout(() => Service.SavePrefs($state.snapshot(ui.prefs)).catch(() => {}), 400);
}

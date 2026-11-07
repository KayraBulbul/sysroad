<script lang="ts">
  import { onMount } from "svelte";
  import { Service, Mode, errText, type Result } from "../lib/api";
  import { ui, loadLesson, openLesson, refresh, toast, fail, ask, pref, savePref, up, where } from "../lib/state.svelte";
  import { workspace, type Actions } from "../lib/editor.svelte";
  import LessonPane from "../components/LessonPane.svelte";
  import OutputPane from "../components/OutputPane.svelte";
  import Splitter from "../components/Splitter.svelte";

  let { id, active }: { id: string; active: boolean } = $props();

  const d = $derived(ui.lessons[id]);
  const hasCode = $derived(!!d?.hasCode);
  const graded = $derived(!!d?.graded);

  // Pane sizes, remembered per machine.
  let lessonW = $state(pref("lessonW", 0.42));
  let outH = $state(pref("outputH", 0.36));
  let collapsed = $state(pref("lessonCollapsed", false));

  let showUTH = $state(false);
  let solDiff = $state(false);
  let result = $state<Result | null>(null);
  let resultFor = "";
  let mine = $state<"" | Mode>(""); // our own run in flight
  let mineSince = $state(0);

  let editorEl: HTMLDivElement;
  let lessonEl: HTMLDivElement | undefined = $state();
  let outEl: HTMLDivElement | undefined = $state();
  let paneRef: LessonPane | undefined = $state();

  // A run in flight anywhere in this app shows here; one from another window
  // (TUI, CLI) arrives as a new result.
  const running = $derived(mine || (ui.snap?.running?.lesson === id ? (ui.snap.running.mode as string) : ""));
  const since = $derived(mine ? mineSince : ui.snap?.running ? new Date(ui.snap.running.since).getTime() : 0);

  // Switching lessons: load the files and the latest result.
  let shown = "";
  $effect(() => {
    const dd = d;
    if (!dd || dd.id === shown) return;
    shown = dd.id;
    showUTH = false;
    solDiff = false;
    if (dd.hasCode) workspace.open(dd.id);
    lessonEl?.scrollTo({ top: 0 });
  });
  $effect(() => {
    // Keep the shown result in step with the lesson's saved one.
    const r = d?.result ?? null;
    if (!d) return;
    if (resultFor !== d.id || (r && (!result || r.at !== result.at))) {
      result = r;
      resultFor = d.id;
    }
  });

  // ---- actions ----

  async function run(mode: Mode) {
    if (!d) return;
    if (!d.graded) {
      toast(d.written || d.body ? "Nothing to run here — mark it done when you've done it" : "This lesson isn't written yet");
      return;
    }
    if (running) {
      toast("Already running — one at a time");
      return;
    }
    const lessonID = d.id;
    mine = mode;
    mineSince = Date.now();
    try {
      await workspace.flush();
      if (workspace.conflicted()) {
        toast("A file changed on disk — choose Reload or Keep mine first", "bad");
        return;
      }
      const out = await Service.Run(lessonID, mode);
      if (!out) return;
      if (lessonID === id) {
        result = out.result;
        resultFor = lessonID;
      }
      if (out.terminal) toast("The program opened in a terminal window");
      const o = out.outcome;
      if (o?.completed) {
        toast(`✓ Lesson complete  +${o.xpGained} XP`, "xp", 5000);
        if (o.levelUp) setTimeout(() => toast(`★ Level ${o.level}!`, "xp", 5000), 400);
      }
      await Promise.all([loadLesson(lessonID), refresh()]);
      if (o?.completed && ui.lessons[lessonID]?.hasUth) {
        showUTH = true;
        paneRef?.reveal("#uth");
      }
    } catch (e) {
      const msg = errText(e);
      if (msg !== "stopped") fail(e);
    } finally {
      mine = "";
    }
  }

  async function hint() {
    if (!d) return;
    if (!d.hintsTotal) return toast("No hints for this one");
    if ((d.hints?.length ?? 0) >= d.hintsTotal) return toast("That's every hint");
    try {
      const hints = (await Service.RevealHint(d.id)) ?? [];
      d.hints = hints;
      paneRef?.reveal(`#hint-${hints.length}`);
    } catch (e) {
      fail(e);
    }
  }

  function uth() {
    if (!d?.hasUth) return toast("No under-the-hood notes for this one");
    if (d.status !== "done") return toast("Under the hood unlocks when you pass Submit");
    showUTH = !showUTH;
    if (showUTH) paneRef?.reveal("#uth");
  }

  async function openSolution(diff: boolean) {
    if (!d) return;
    try {
      const files = (await Service.Solution(d.id)) ?? [];
      solDiff = diff;
      workspace.showSolution(files, diff);
    } catch (e) {
      fail(e);
    }
  }

  function solution() {
    if (!d?.hasCode) return;
    if (workspace.solutionOpen()) {
      workspace.hideSolution();
      return;
    }
    if (d.status === "done") return void openSolution(false);
    ask({
      title: "Peek at the solution?",
      detail: "It opens read-only next to your files. You can still finish the lesson yourself.",
      yes: "Show solution",
      onYes: () => openSolution(false),
    });
  }

  function reset() {
    if (!d?.hasCode) return;
    const lessonID = d.id;
    ask({
      title: "Reset to the starter code?",
      detail: "Your files move to .local/reset-backup/ in the data folder, so nothing is lost.",
      yes: "Reset",
      danger: true,
      onYes: async () => {
        try {
          const [backup, files] = await Service.Reset(lessonID);
          workspace.replaceAll(lessonID, files ?? []);
          toast(backup ? `Reset. Your old files are in ${backup.replace(/^\/(home|Users)\/[^/]+/, "~")}` : "Reset to the starter code", "good", 6000);
          refresh();
        } catch (e) {
          fail(e);
        }
      },
    });
  }

  async function toggleDone() {
    if (!d || d.graded) return;
    try {
      await Service.ToggleDone(d.id);
      await Promise.all([loadLesson(d.id), refresh()]);
    } catch (e) {
      fail(e);
    }
  }

  function next() {
    if (d?.next) openLesson(d.next);
  }
  function prev() {
    if (d?.prev) openLesson(d.prev);
  }
  function focusLesson() {
    if (collapsed) toggleCollapsed();
    lessonEl?.focus();
  }
  function focusEditor() {
    if (hasCode) workspace.focus();
  }
  function focusOutput() {
    outEl?.focus();
  }
  function toggleCollapsed() {
    collapsed = !collapsed;
    savePref("lessonCollapsed", collapsed);
  }

  async function format() {
    try {
      const r = await workspace.format();
      if (r === "unchanged") toast("Already formatted");
    } catch (e) {
      fail(e);
    }
  }

  const actions: Actions = {
    run: () => run(Mode.ModeRun),
    submit: () => run(Mode.ModeSubmit),
    hint,
    solution,
    reset,
    uth,
    format,
    next,
    prev,
    focusLesson,
    focusOutput,
    palette: () => (ui.palette = true),
  };

  onMount(() => {
    workspace.mount(editorEl, actions, (m) => toast(m, "bad", 6000), pref("vim", true));
    return () => workspace.destroy();
  });

  function toggleVim() {
    workspace.setVim(!workspace.vimMode);
    savePref("vim", workspace.vimMode);
    toast(workspace.vimMode ? "Vim mode on" : "Vim mode off");
  }

  // ---- keys ----

  function inEditor(t: EventTarget | null) {
    return !!(t as HTMLElement | null)?.closest?.(".cm-editor");
  }

  // Chords that work everywhere in the lesson view, even inside the editor:
  // handled in the capture phase, before CodeMirror sees them.
  function keyCapture(e: KeyboardEvent) {
    if (!active || ui.palette || ui.confirm) return;
    if (inEditor(e.target) && workspace.handleLeader(e)) {
      e.preventDefault();
      e.stopPropagation();
      return;
    }
    const mod = e.ctrlKey || e.metaKey;
    if (!mod || e.altKey) return;
    const k = e.key.toLowerCase();
    let handled = true;
    if (k === "enter") run(e.shiftKey ? Mode.ModeSubmit : Mode.ModeRun);
    else if (k === "s" && !e.shiftKey) workspace.saveActive();
    else if (k === "b" && !e.shiftKey) toggleCollapsed();
    else if (k === "h" && !e.shiftKey && !(inEditor(e.target) && workspace.insertMode())) focusLesson();
    else if (k === "l" && !e.shiftKey) focusEditor();
    else if (k === "j" && !e.shiftKey && !(inEditor(e.target) && workspace.insertMode())) focusOutput();
    else if (k === "." && !e.shiftKey && running) Service.StopRun();
    else handled = false;
    if (handled) {
      e.preventDefault();
      e.stopPropagation();
    }
  }

  // The TUI's letter keys, when the editor doesn't have focus.
  function key(e: KeyboardEvent) {
    if (!active || ui.palette || ui.confirm || e.defaultPrevented) return;
    if (e.ctrlKey || e.metaKey || e.altKey) return;
    const t = e.target as HTMLElement;
    if (inEditor(t) || t.closest("input, textarea")) return;
    const scroll = (dy: number) => lessonEl?.scrollBy({ top: dy });
    const h = lessonEl?.clientHeight ?? 400;
    switch (e.key) {
      case "r": run(Mode.ModeRun); break;
      case "s": run(Mode.ModeSubmit); break;
      case "h": hint(); break;
      case "u": uth(); break;
      case "v": solution(); break;
      case "R": reset(); break;
      case "d": toggleDone(); break;
      case "]": next(); break;
      case "[": prev(); break;
      case "e":
      case "i": focusEditor(); break;
      case "j": scroll(48); break;
      case "k": scroll(-48); break;
      case "g": lessonEl?.scrollTo({ top: 0 }); break;
      case "G": lessonEl?.scrollTo({ top: 1 << 20 }); break;
      case "Escape": up(); break;
      default:
        if (e.key === " " && e.shiftKey) scroll(-h * 0.8);
        else return;
    }
    e.preventDefault();
  }
</script>

<svelte:window onkeydowncapture={keyCapture} onkeydown={key} />

<div class="lesson" class:collapsed>
  <div class="left" style="width:{collapsed ? 0 : lessonW * 100}%">
    {#if d}
      <LessonPane bind:this={paneRef} bind:el={lessonEl} {d} {showUTH} onhint={hint} onuth={uth} />
    {:else}
      <div class="loading"></div>
    {/if}
  </div>
  {#if !collapsed}
    <Splitter dir="x" value={lessonW} min={0.22} max={0.7} onchange={(v) => (lessonW = v)} ondone={(v) => savePref("lessonW", v)} />
  {/if}

  <div class="right">
    <div class="editor-area" style="height:{hasCode ? (1 - outH) * 100 : 0}%" class:hidden={!hasCode}>
      <div class="tabs">
        {#each workspace.tabs as t, i (t.key)}
          <button class="tab" class:on={i === workspace.active} class:ro={t.kind !== "file"} onclick={() => { workspace.select(i); workspace.focus(); }}>
            {#if t.kind === "solution"}<span class="badge">solution</span>{:else if t.kind === "diff"}<span class="badge">diff</span>{/if}
            {t.name}
            {#if t.conflict}<span class="mark bad" title="Changed on disk">!</span>{:else if t.dirty}<span class="mark" title="Unsaved">●</span>{/if}
          </button>
        {/each}
        <span class="spacer"></span>
        {#if workspace.solutionOpen()}
          <button class="tool" class:on={solDiff} onclick={() => openSolution(!solDiff)} title="Show the solution's changes against your code">diff</button>
          <button class="tool" onclick={() => workspace.hideSolution()} title="Close the solution">close solution</button>
        {/if}
        {#if workspace.leaderPending}<span class="leader">␣ r run · s submit · h hint · cf format · v solution · n/p next/prev</span>{/if}
        <button class="tool" class:on={workspace.vimMode} onclick={toggleVim} title="Vim mode">vim</button>
      </div>
      {#if workspace.tabs[workspace.active]?.conflict}
        <div class="conflict">
          <span>This file changed on disk (another machine, or another editor).</span>
          <button class="btn" onclick={() => workspace.resolve("reload")}>Reload</button>
          <button class="btn ghost" onclick={() => workspace.resolve("keep")}>Keep mine</button>
        </div>
      {/if}
      <div class="cm" bind:this={editorEl}></div>
    </div>

    {#if hasCode}
      <Splitter dir="y" value={1 - outH} min={0.2} max={0.85} onchange={(v) => (outH = 1 - v)} ondone={(v) => savePref("outputH", 1 - v)} />
    {/if}

    <div class="bottom" style="height:{hasCode ? outH * 100 : 100}%">
      <div class="actions">
        {#if graded}
          <button class="btn" onclick={() => run(Mode.ModeRun)} disabled={!!running}>Run <span class="k">Ctrl ⏎</span></button>
          <button class="btn primary" onclick={() => run(Mode.ModeSubmit)} disabled={!!running}>Submit <span class="k">Ctrl ⇧ ⏎</span></button>
          {#if running}<button class="btn ghost" onclick={() => Service.StopRun()}>Stop <span class="k">Ctrl .</span></button>{/if}
        {:else if d && (d.written || d.body || d.url)}
          <button class="btn primary" onclick={toggleDone}>{d.status === "done" ? "Mark not done" : "Mark done"} <span class="k">d</span></button>
        {/if}
        <span class="spacer"></span>
        {#if d?.hintsTotal}
          <button class="btn ghost" onclick={hint} disabled={(d.hints?.length ?? 0) >= d.hintsTotal}>Hint <span class="k">h</span></button>
        {/if}
        {#if hasCode}
          <button class="btn ghost" onclick={solution}>{workspace.solutionOpen() ? "Hide solution" : "Solution"} <span class="k">v</span></button>
          <button class="btn ghost" onclick={reset}>Reset <span class="k">R</span></button>
        {/if}
      </div>
      <div class="output">
        {#if !hasCode && d && !graded}
          <div class="manual">
            {#if d.status === "done"}
              <p class="ok">✓ Done</p>
            {:else}
              <p>This one is done outside sysroad. Mark it done when you've finished it.</p>
            {/if}
          </div>
        {:else}
          <OutputPane bind:el={outEl} {result} {running} {since} {graded} next={d?.next ? (where(d.next)?.row.title ?? "") : ""} onnext={next} />
        {/if}
      </div>
    </div>
  </div>
</div>

<style>
  .lesson {
    display: flex;
    height: 100%;
    min-height: 0;
  }
  .left {
    flex: none;
    min-width: 0;
    overflow: hidden;
    background: var(--bg);
  }
  .collapsed .left {
    display: none;
  }
  .right {
    flex: 1;
    min-width: 0;
    display: flex;
    flex-direction: column;
    background: var(--bg);
  }
  .editor-area {
    display: flex;
    flex-direction: column;
    min-height: 0;
  }
  .editor-area.hidden {
    display: none;
  }
  .tabs {
    flex: none;
    display: flex;
    align-items: stretch;
    height: 34px;
    border-bottom: 1px solid var(--line);
    background: var(--bg-dark);
    padding: 0 6px 0 0;
    overflow-x: auto;
  }
  .tab {
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 0 14px;
    font-size: 12.5px;
    color: var(--fg-dim);
    border-right: 1px solid var(--line);
    white-space: nowrap;
  }
  .tab:hover {
    color: var(--fg-soft);
  }
  .tab.on {
    color: var(--fg-bright);
    background: var(--bg);
    box-shadow: inset 0 -2px 0 var(--accent);
  }
  .badge {
    font-size: 10px;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    color: var(--yellow);
  }
  .mark {
    color: var(--fg-dim);
    font-size: 10px;
  }
  .mark.bad {
    color: var(--red);
    font-weight: 700;
    font-size: 12px;
  }
  .spacer {
    flex: 1;
  }
  .tool {
    align-self: center;
    font-size: 11.5px;
    color: var(--fg-dim);
    padding: 2px 8px;
    border-radius: 4px;
    margin-left: 4px;
    white-space: nowrap;
  }
  .tool:hover {
    color: var(--fg-soft);
    background: var(--hover);
  }
  .leader {
    align-self: center;
    font-size: 11.5px;
    color: var(--accent);
    margin-right: 6px;
    white-space: nowrap;
  }
  .tool.on {
    color: var(--accent);
  }
  .conflict {
    flex: none;
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 7px 12px;
    font-size: 12.5px;
    background: color-mix(in oklab, var(--yellow) 12%, var(--bg));
    border-bottom: 1px solid color-mix(in oklab, var(--yellow) 40%, var(--line));
    color: var(--fg-bright);
  }
  .conflict span {
    flex: 1;
  }
  .conflict .btn {
    height: 26px;
  }
  .cm {
    flex: 1;
    min-height: 0;
  }
  .cm :global(.cm-editor) {
    height: 100%;
  }
  .cm :global(.cm-editor.cm-focused) {
    outline: none;
  }
  .bottom {
    display: flex;
    flex-direction: column;
    min-height: 0;
  }
  .actions {
    flex: none;
    display: flex;
    align-items: center;
    gap: 8px;
    padding: 8px 12px;
    border-bottom: 1px solid var(--line);
    background: var(--bg);
  }
  .output {
    flex: 1;
    min-height: 0;
  }
  .manual {
    height: 100%;
    display: grid;
    place-items: center;
    color: var(--fg-dim);
    background: var(--bg-dark);
    font-size: 13px;
    padding: 20px;
    text-align: center;
  }
  .manual .ok {
    color: var(--green);
    font-size: 15px;
  }
  .loading {
    height: 100%;
  }
</style>

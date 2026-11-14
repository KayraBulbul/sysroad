<script lang="ts">
  import { onMount, tick, untrack } from "svelte";
  import { Service, Mode, ProjectState, errText, tilde, type Result } from "../lib/api";
  import { ui, loadLesson, openLesson, refresh, toast, fail, ask, pref, savePref, up, where } from "../lib/state.svelte";
  import { workspace, type Actions } from "../lib/editor.svelte";
  import LessonPane from "../components/LessonPane.svelte";
  import OutputPane from "../components/OutputPane.svelte";
  import QuizPane from "../components/QuizPane.svelte";
  import type { Quiz, Outcome } from "../../bindings/github.com/KayraBulbul/sysroad/internal/app/models";
  import Splitter from "../components/Splitter.svelte";

  let { id, active }: { id: string; active: boolean } = $props();

  const d = $derived(ui.lessons[id]);
  const hasCode = $derived(!!d?.hasCode);
  const graded = $derived(!!d?.graded);

  // A project lesson: you make its files, and only the lesson the project
  // is on can change them or run. The others show your files read-only.
  const proj = $derived(d?.project ?? null);
  const away = $derived(!!proj && proj.state !== ProjectState.ProjectCurrent);
  const canEdit = $derived(!!proj && !away);
  const orderLine = $derived.by(() => {
    if (!proj || !away) return "";
    if (!proj.onId) return `You've finished the ${proj.id} project.`;
    if (proj.state === ProjectState.ProjectPassed) {
      const how = d?.status === "skipped" ? "You caught up past this lesson." : "You've passed this lesson.";
      return `${how} The ${proj.id} project is on "${proj.onTitle}".`;
    }
    return `This lesson is locked. The ${proj.id} project is on "${proj.onTitle}".`;
  });
  const emptyProject = $derived(!!proj && workspace.lesson === id && !workspace.loading && workspace.fileTabs() === 0 && !workspace.solutionOpen());
  // Files but no header yet (only the lesson's demo files, say): keep
  // saying what to make, with the contract, until it exists.
  const noHeader = $derived(
    canEdit && !emptyProject && workspace.lesson === id && !workspace.loading && !workspace.tabs.some((t) => t.kind === "file" && t.name === proj?.header),
  );

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
  let quizRef: QuizPane | undefined = $state();

  // A run in flight anywhere in this app shows here; one from another window
  // (TUI, CLI) arrives as a new result.
  const running = $derived(mine || (ui.snap?.running?.lesson === id ? (ui.snap.running.mode as string) : ""));
  const since = $derived(mine ? mineSince : ui.snap?.running ? new Date(ui.snap.running.since).getTime() : 0);

  // Switching lessons: show its files at once, without waiting for the
  // lesson text to load.
  let shown = "";
  $effect(() => {
    const lid = id;
    untrack(() => {
      if (lid === shown) return;
      shown = lid;
      showUTH = false;
      solDiff = false;
      workspace.open(lid);
      lessonEl?.scrollTo({ top: 0 });
    });
  });
  // A project lesson's header sorts first among its tabs. When its place in
  // the project changes (you passed it, caught up), its files change between
  // editable and read-only: fetch them again.
  let placeOf = { id: "", state: "" };
  $effect(() => {
    const lid = d?.id ?? "";
    const st = d?.project?.state ?? "";
    untrack(() => {
      if (d?.project) workspace.setHeader(lid, d.project.header);
      if (lid === placeOf.id && st !== placeOf.state && lid === id) workspace.open(lid);
      placeOf = { id: lid, state: st };
    });
  });
  // Keep the active tab in sight when the strip scrolls.
  let tabsEl: HTMLDivElement | undefined = $state();
  $effect(() => {
    void workspace.active;
    void workspace.tabs.length;
    tick().then(() => tabsEl?.querySelector(".tab.on")?.scrollIntoView({ block: "nearest", inline: "nearest" }));
  });
  // A mouse wheel scrolls the tab strip sideways.
  $effect(() => {
    const el = tabsEl;
    if (!el) return;
    const wheel = (e: WheelEvent) => {
      if (Math.abs(e.deltaY) <= Math.abs(e.deltaX) || el.scrollWidth <= el.clientWidth) return;
      el.scrollLeft += e.deltaY;
      e.preventDefault();
    };
    el.addEventListener("wheel", wheel, { passive: false });
    return () => el.removeEventListener("wheel", wheel);
  });
  // Load the neighbours ahead, so <leader>n/p and [ ] land on a ready page.
  $effect(() => {
    for (const n of [d?.next, d?.prev]) if (n && !ui.lessons[n]) untrack(() => loadLesson(n));
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
    if (d.quiz) {
      toast("This is a quiz — pick an answer for each question");
      return;
    }
    if (!d.graded) {
      toast(d.written || d.body ? "Nothing to run here — mark it done when you've done it" : "This lesson isn't written yet");
      return;
    }
    if (away) {
      toast(orderLine);
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
      if (!files.length) {
        const gone = d.project?.removes ?? [];
        toast(gone.length ? `The reference only deletes ${gone.join(", ")}` : "The reference changes no files");
        return;
      }
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
    if (d.status === "done" || d.status === "skipped") return void openSolution(false);
    if (d.project) {
      ask({
        title: "Peek at the reference?",
        detail: "The course's reference for this lesson opens read-only next to your files: the files it adds or changes. You can still finish the lesson yourself.",
        yes: "Show reference",
        onYes: () => openSolution(false),
      });
      return;
    }
    ask({
      title: "Peek at the solution?",
      detail: "It opens read-only next to your files. You can still finish the lesson yourself.",
      yes: "Show solution",
      onYes: () => openSolution(false),
    });
  }

  function reset() {
    if (!d?.hasCode) return;
    if (away) return toast(orderLine);
    const lessonID = d.id;
    const project = !!d.project;
    ask({
      title: project ? "Reset the project to the start of this lesson?" : "Reset to the starter code?",
      detail: project
        ? "Your files move to .local/reset-backup/ in the data folder, and the project goes back to how it was when this lesson started."
        : "Your files move to .local/reset-backup/ in the data folder, so nothing is lost.",
      yes: "Reset",
      danger: true,
      onYes: async () => {
        try {
          const [backup, files] = await Service.Reset(lessonID);
          workspace.replaceAll(lessonID, files ?? []);
          solDiff = false;
          const done = project ? "Reset to the start of this lesson" : "Reset to the starter code";
          toast(backup ? `Reset. Your old files are in ${tilde(backup)}` : done, "good", 6000);
          refresh();
        } catch (e) {
          fail(e);
        }
      },
    });
  }

  // ---- a project's files ----

  // Catching up swaps your project for the course's reference as of the end
  // of this lesson, and skips the lesson.
  function catchUp() {
    if (!d?.project) return;
    if (away) return toast(orderLine);
    const lessonID = d.id;
    ask({
      title: "Catch up to the reference?",
      detail:
        "Your files move to .local/reset-backup/ in the data folder, and the course's reference as of the end of this lesson takes their place. The lesson is skipped, with no XP.",
      yes: "Catch up",
      danger: true,
      onYes: async () => {
        try {
          const r = await Service.CatchUp(lessonID);
          if (!r) return;
          workspace.replaceAll(lessonID, r.files ?? []);
          solDiff = false;
          if (r.lesson) ui.lessons[lessonID] = r.lesson;
          toast(r.backup ? `Caught up. Your old files are in ${tilde(r.backup)}` : "Caught up", "good", 6000);
          refresh();
        } catch (e) {
          fail(e);
        }
      },
    });
  }

  // Naming a file: a new one, or the active tab's new name. The field sits
  // in the tab strip; Enter does it, Escape lets it go.
  let naming = $state<{ from: string } | null>(null); // from "" is a new file
  let nameText = $state("");
  let nameErr = $state("");
  let nameEl: HTMLInputElement | undefined = $state();

  function startNaming(from: string, text: string) {
    if (!canEdit) return;
    naming = { from };
    nameText = text;
    nameErr = "";
    tick().then(() => {
      nameEl?.focus();
      // Select the name without its extension, as file managers do.
      const dot = text.lastIndexOf(".");
      const slash = text.lastIndexOf("/");
      nameEl?.setSelectionRange(slash + 1, dot > slash + 1 ? dot : text.length);
    });
  }

  function newFile() {
    startNaming("", "");
  }

  function renameFile() {
    const name = workspace.activeFile();
    if (name) startNaming(name, name);
  }

  function stopNaming() {
    naming = null;
    nameErr = "";
  }

  async function create(name: string) {
    if (!d) return;
    const lessonID = d.id;
    const started = d.status !== "todo";
    const f = await Service.CreateFile(lessonID, name);
    if (f) workspace.addFile(lessonID, f, true);
    // The editor was hidden behind the empty folder's note: focus it once
    // it shows.
    await tick();
    workspace.focus();
    if (!started) await Promise.all([loadLesson(lessonID), refresh()]);
  }

  async function commitName() {
    const n = naming;
    const name = nameText.trim();
    if (!n || !d) return;
    const lessonID = d.id;
    try {
      if (!n.from) {
        await create(name);
      } else if (name !== n.from) {
        if (!(await workspace.saveFile(lessonID, n.from))) {
          nameErr = "This file changed on disk: choose Reload or Keep mine first";
          return;
        }
        const f = await Service.RenameFile(lessonID, n.from, name);
        if (f) workspace.renamed(lessonID, n.from, f);
      }
      stopNaming();
      await tick();
      workspace.focus();
    } catch (e) {
      nameErr = errText(e);
    }
  }

  function nameKey(e: KeyboardEvent) {
    if (e.key === "Enter") {
      e.preventDefault();
      commitName();
    } else if (e.key === "Escape") {
      e.preventDefault();
      e.stopPropagation();
      stopNaming();
      workspace.focus();
    }
  }

  async function createHeader() {
    if (!proj) return;
    try {
      await create(proj.header);
    } catch (e) {
      fail(e);
    }
  }

  function deleteFile() {
    const name = workspace.activeFile();
    if (!d || !canEdit || !name) return;
    const lessonID = d.id;
    ask({
      title: `Delete ${name}?`,
      detail: "It moves to .local/reset-backup/ in the data folder, so nothing is lost.",
      yes: "Delete",
      danger: true,
      onYes: async () => {
        try {
          await workspace.saveFile(lessonID, name);
          const dst = await Service.DeleteFile(lessonID, name);
          workspace.removeFile(lessonID, name);
          toast(`Deleted. ${name} is in ${tilde(dst)}`, "good", 6000);
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

  // ---- quizzes ----

  async function answered(q: Quiz, o: Outcome | null) {
    if (!d) return;
    d.quiz = q;
    if (o?.completed) {
      toast(`✓ Quiz complete  +${o.xpGained} XP`, "xp", 5000);
      if (o.levelUp) setTimeout(() => toast(`★ Level ${o.level}!`, "xp", 5000), 400);
      await Promise.all([loadLesson(d.id), refresh()]);
      if (ui.lessons[d.id]?.hasUth) {
        showUTH = true;
        paneRef?.reveal("#uth");
      }
    }
  }

  function retake() {
    if (!d?.quiz) return;
    const lessonID = d.id;
    ask({
      title: "Take the quiz again?",
      detail: "Your answers are cleared. The lesson stays done.",
      yes: "Retake",
      onYes: async () => {
        try {
          const q = await Service.ResetQuiz(lessonID);
          if (q && ui.lessons[lessonID]) ui.lessons[lessonID].quiz = q;
          await loadLesson(lessonID);
        } catch (e) {
          fail(e);
        }
      },
    });
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
    const t = e.target as HTMLElement;
    const typing = !!t.closest?.("input, textarea") && !inEditor(t);
    if (!typing && workspace.handleLeader(e, inEditor(t))) {
      e.preventDefault();
      e.stopPropagation();
      return;
    }
    const mod = e.ctrlKey || e.metaKey;
    // Shift+Tab in the editor cycles its tabs (files, then solution or
    // diff). Elsewhere it stays the browser's reverse focus move.
    if (e.key === "Tab" && e.shiftKey && !mod && !e.altKey && inEditor(t) && workspace.tabs.length > 1) {
      e.preventDefault();
      e.stopPropagation();
      workspace.select((workspace.active + 1) % workspace.tabs.length);
      return;
    }
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
    if (d?.quiz && e.key >= "1" && e.key <= "9") {
      quizRef?.answerNext(Number(e.key) - 1);
      e.preventDefault();
      return;
    }
    if (d?.quiz && e.key === "R") {
      retake();
      e.preventDefault();
      return;
    }
    switch (e.key) {
      case "r": run(Mode.ModeRun); break;
      case "s": run(Mode.ModeSubmit); break;
      case "h": hint(); break;
      case "u": uth(); break;
      case "v": solution(); break;
      case "R": reset(); break;
      case "C": catchUp(); break;
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
        <div class="files" bind:this={tabsEl}>
          {#each workspace.tabs as t, i (t.key)}
            {#if naming && naming.from && t.kind === "file" && t.name === naming.from}
              <input class="name" bind:this={nameEl} bind:value={nameText} onkeydown={nameKey} onblur={stopNaming} spellcheck="false" aria-label="New name for {t.name}" />
            {:else}
              <button
                class="tab"
                class:on={i === workspace.active}
                class:ro={t.readOnly}
                onclick={() => { workspace.select(i); workspace.focus(); }}
                ondblclick={() => { if (t.kind === "file") renameFile(); }}
                title={canEdit && t.kind === "file" ? "Double-click to rename" : undefined}
              >
                {#if t.kind === "solution"}<span class="badge">{proj ? "reference" : "solution"}</span>{:else if t.kind === "diff"}<span class="badge">diff</span>{:else if t.readOnly && !proj}<span class="badge ro" title="Shared with the tests. You don't edit this file">read-only</span>{/if}
                {t.name}
                {#if t.conflict}<span class="mark bad" title={t.gone ? "Deleted on disk" : "Changed on disk"}>!</span>{:else if t.dirty}<span class="mark" title="Unsaved">●</span>{/if}
              </button>
            {/if}
          {/each}
          {#if naming && !naming.from}
            <input class="name" bind:this={nameEl} bind:value={nameText} onkeydown={nameKey} onblur={stopNaming} placeholder="src/name.cpp" spellcheck="false" aria-label="New file name" />
          {/if}
        </div>
        {#if canEdit && !naming}
          <button class="tool add" onclick={newFile} title="New file in the project folder">+</button>
        {/if}
        <span class="spacer"></span>
        {#if canEdit && workspace.activeFile() && !naming}
          <button class="tool" onclick={renameFile} title="Rename {workspace.activeFile()} (or double-click its tab)">rename</button>
          <button class="tool" onclick={deleteFile} title="Delete {workspace.activeFile()}">delete</button>
        {/if}
        {#if away && workspace.fileTabs()}<span class="ro-note">read-only</span>{/if}
        {#if workspace.solutionOpen()}
          <button class="tool" class:on={solDiff} onclick={() => openSolution(!solDiff)} title="Show the solution's changes against your code">diff</button>
          <button class="tool" onclick={() => workspace.hideSolution()} title={proj ? "Close the reference" : "Close the solution"}>{proj ? "close reference" : "close solution"}</button>
        {/if}
        {#if workspace.leaderPending}<span class="leader">␣ r run · s submit · h hint · cf format · v solution · n/p next/prev</span>{/if}
        <button class="tool" class:on={workspace.vimMode} onclick={toggleVim} title="Vim mode">vim</button>
      </div>
      {#if nameErr}
        <div class="bar bad">{nameErr}</div>
      {/if}
      {#if orderLine}
        <div class="bar">
          <span>{orderLine}</span>
          {#if proj?.onId}<button class="btn" onclick={() => proj && openLesson(proj.onId)}>Go to it</button>{/if}
        </div>
      {/if}
      {#if noHeader && proj}
        <div class="bar">
          <div class="what">
            <span>The tests include <code>{proj.header}</code>, which you haven't made yet. {#if proj.contract?.length}Declare in it:{/if}</span>
            {#if proj.contract?.length}<pre>{proj.contract.join("\n")}</pre>{/if}
          </div>
          <button class="btn" onclick={createHeader}>Create {proj.header}</button>
        </div>
      {/if}
      {#if proj && workspace.solutionOpen()}
        <div class="bar">
          <span>
            The course's reference: the files this lesson adds or changes.
            {#if proj.removes?.length}It deletes {proj.removes.join(", ")}.{/if}
          </span>
          {#if canEdit}<button class="btn" onclick={catchUp} title="Swap your project for the reference and skip this lesson">Catch up <span class="k">C</span></button>{/if}
        </div>
      {/if}
      {#if workspace.tabs[workspace.active]?.conflict}
        <div class="conflict">
          {#if workspace.tabs[workspace.active]?.gone}
            <span>This file was deleted on disk (another editor, or a sync).</span>
            <button class="btn" onclick={() => workspace.resolve("reload")}>Let it go</button>
            <button class="btn ghost" onclick={() => workspace.resolve("keep")}>Keep mine</button>
          {:else}
            <span>This file changed on disk (another machine, or another editor).</span>
            <button class="btn" onclick={() => workspace.resolve("reload")}>Reload</button>
            <button class="btn ghost" onclick={() => workspace.resolve("keep")}>Keep mine</button>
          {/if}
        </div>
      {/if}
      <div class="cm-wrap">
        <div class="cm" class:gone={emptyProject} bind:this={editorEl}></div>
        {#if emptyProject && proj}
          <div class="empty-project">
            <p>Your {proj.id} folder is empty. You make every file in it, the header and <code>main.cpp</code> included.</p>
            {#if proj.contract?.length}
              <p class="dim">The tests include <code>{proj.header}</code> and use:</p>
              <pre>{proj.contract.join("\n")}</pre>
            {/if}
            {#if canEdit && proj.starter?.length}
              <p class="dim">Your first file brings the lesson's {proj.starter.join(" and ")} with it, to try {proj.id} on.</p>
            {/if}
            {#if canEdit}
              <p class="row">
                <button class="btn primary" onclick={createHeader}>Create {proj.header}</button>
                <span class="dim">or <kbd>+</kbd> for another file</span>
              </p>
            {/if}
          </div>
        {/if}
      </div>
    </div>

    {#if hasCode}
      <Splitter dir="y" value={1 - outH} min={0.2} max={0.85} onchange={(v) => (outH = 1 - v)} ondone={(v) => savePref("outputH", 1 - v)} />
    {/if}

    <div class="bottom" style="height:{hasCode ? outH * 100 : 100}%">
      <div class="actions">
        {#if d?.quiz}
          <span class="quiz-label">{d.quiz.solved === d.quiz.total ? "Quiz complete" : "Pick an answer for each question"} <span class="k">1–9</span></span>
        {:else if graded}
          <button class="btn" onclick={() => run(Mode.ModeRun)} disabled={!!running || away}>Run <span class="k">Ctrl ⏎</span></button>
          <button class="btn primary" onclick={() => run(Mode.ModeSubmit)} disabled={!!running || away}>Submit <span class="k">Ctrl ⇧ ⏎</span></button>
          {#if running}<button class="btn ghost" onclick={() => Service.StopRun()}>Stop <span class="k">Ctrl .</span></button>{/if}
        {:else if d && (d.written || d.body || d.url)}
          <button class="btn primary" onclick={toggleDone}>{d.status === "done" ? "Mark not done" : "Mark done"} <span class="k">d</span></button>
        {/if}
        <span class="spacer"></span>
        {#if d?.hintsTotal}
          <button class="btn ghost" onclick={hint} disabled={(d.hints?.length ?? 0) >= d.hintsTotal}>Hint <span class="k">h</span></button>
        {/if}
        {#if d?.quiz}
          <button class="btn ghost" onclick={retake} disabled={!d.quiz.solved}>Retake <span class="k">R</span></button>
        {/if}
        {#if hasCode}
          <button class="btn ghost" onclick={solution}>{workspace.solutionOpen() ? (proj ? "Hide reference" : "Hide solution") : proj ? "Reference" : "Solution"} <span class="k">v</span></button>
          <button class="btn ghost" onclick={reset} disabled={away}>Reset <span class="k">R</span></button>
        {/if}
      </div>
      <div class="output">
        {#if d?.quiz}
          <QuizPane bind:this={quizRef} bind:el={outEl} id={d.id} quiz={d.quiz} onanswered={answered} />
        {:else if !hasCode && d && !graded}
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
  }
  /* The files scroll sideways on their own, so the tools stay in reach
     however many there are. */
  .files {
    flex: 0 1 auto;
    min-width: 0;
    display: flex;
    align-items: stretch;
    overflow-x: auto;
    overflow-y: hidden;
    scrollbar-width: thin;
  }
  .files::-webkit-scrollbar {
    height: 3px;
  }
  .files::-webkit-scrollbar-thumb {
    border: 0;
    border-radius: 0;
  }
  .name {
    align-self: center;
    flex: none;
    width: 200px;
    margin: 0 8px;
    padding: 3px 8px;
    font: inherit;
    font-size: 12.5px;
    color: var(--fg-bright);
    background: var(--bg);
    border: 1px solid var(--accent);
    border-radius: 4px;
    outline: none;
  }
  .name::placeholder {
    color: var(--fg-dim);
  }
  .tool.add {
    flex: none;
    font-size: 15px;
    line-height: 1;
    padding: 2px 8px 4px;
  }
  .ro-note {
    align-self: center;
    font-size: 11.5px;
    color: var(--fg-dim);
    margin: 0 6px;
    white-space: nowrap;
  }
  .tab {
    flex: none;
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
  .badge.ro {
    color: var(--fg-dim);
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
  .conflict .btn,
  .bar .btn {
    height: 26px;
  }
  .bar {
    flex: none;
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 7px 12px;
    font-size: 12.5px;
    background: var(--panel);
    border-bottom: 1px solid var(--line);
    color: var(--fg-soft);
  }
  .bar span,
  .bar .what {
    flex: 1;
    min-width: 0;
  }
  .bar code {
    font: inherit;
    color: var(--fg-bright);
  }
  .bar pre {
    margin: 6px 0 0;
    max-height: 9em;
    overflow: auto;
    font: inherit;
    color: var(--fg-bright);
    white-space: pre-wrap;
  }
  .bar.bad {
    color: var(--bright-red);
    background: color-mix(in oklab, var(--red) 10%, var(--bg));
    border-bottom-color: color-mix(in oklab, var(--red) 35%, var(--line));
  }
  .cm-wrap {
    flex: 1;
    min-height: 0;
    position: relative;
    display: flex;
    flex-direction: column;
  }
  .cm {
    flex: 1;
    min-height: 0;
  }
  .cm.gone {
    visibility: hidden;
  }
  .empty-project {
    position: absolute;
    inset: 0;
    overflow: auto;
    padding: 28px 32px;
    font-size: 13px;
    color: var(--fg-soft);
    background: var(--bg);
  }
  .empty-project p {
    margin: 0 0 12px;
    max-width: 60ch;
  }
  .empty-project .dim {
    color: var(--fg-dim);
  }
  .empty-project code {
    font: inherit;
    color: var(--fg-bright);
  }
  .empty-project pre {
    margin: 0 0 18px;
    padding: 10px 14px;
    font: inherit;
    font-size: 12.5px;
    line-height: 1.6;
    color: var(--fg-bright);
    background: var(--bg-dark);
    border: 1px solid var(--line);
    border-radius: 8px;
    white-space: pre-wrap;
  }
  .empty-project .row {
    display: flex;
    align-items: center;
    gap: 14px;
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
  .quiz-label {
    font-size: 12.5px;
    color: var(--fg-dim);
  }
  .quiz-label .k {
    margin-left: 6px;
    opacity: 0.7;
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

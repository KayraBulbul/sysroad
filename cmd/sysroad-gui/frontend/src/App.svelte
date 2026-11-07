<script lang="ts">
  import { onMount } from "svelte";
  import { Service, on, type Theme, type FileEvent, type ResultEvent } from "./lib/api";
  import { applyTheme } from "./lib/theme";
  import { ui, refresh, loadPrefs, loadLesson, go, up, back, forward, pref, savePref } from "./lib/state.svelte";
  import { workspace } from "./lib/editor.svelte";
  import TopBar from "./components/TopBar.svelte";
  import Palette from "./components/Palette.svelte";
  import Toasts from "./components/Toasts.svelte";
  import ConfirmDialog from "./components/ConfirmDialog.svelte";
  import Dashboard from "./views/Dashboard.svelte";
  import Phase from "./views/Phase.svelte";
  import Lesson from "./views/Lesson.svelte";

  let ready = $state(false);
  // The lesson view stays mounted once opened, so going back to it is instant
  // and the editor keeps its state.
  let lessonID = $state("");

  $effect(() => {
    if (ui.route.view === "lesson") lessonID = ui.route.id;
  });

  onMount(() => {
    const offs = [
      on<Theme>("theme", applyTheme),
      on<null>("state", () => {
        refresh();
        if (lessonID) loadLesson(lessonID);
      }),
      on<FileEvent>("file", (ev) => workspace.onFileEvent(ev)),
      // The mouse's side buttons on Linux, caught by the Go side.
      on<number>("nav", (dir) => {
        if (ui.palette || ui.confirm) return;
        if (dir < 0) back();
        else forward();
      }),
      on<ResultEvent>("result", (ev) => {
        if (ui.lessons[ev.lesson]) loadLesson(ev.lesson);
      }),
    ];
    Service.Theme().then(applyTheme);
    Promise.all([loadPrefs(), refresh()]).then(() => {
      ready = true;
      // Reopen the lesson you had open last time on this machine.
      const last = pref<string>("lastLesson", "");
      if (last && pref<string>("lastView", "") === "lesson") go({ view: "lesson", id: last });
    });
    return () => offs.forEach((f) => f());
  });

  $effect(() => {
    if (ready) {
      savePref("lastView", ui.route.view);
    }
  });

  let lastSpace = 0;

  // The mouse's back/forward side buttons (DOM buttons 3 and 4).
  function mouse(e: MouseEvent) {
    if (e.button !== 3 && e.button !== 4) return;
    e.preventDefault();
    e.stopPropagation();
    if (e.type !== "mouseup" || ui.palette || ui.confirm) return;
    if (e.button === 3) back();
    else forward();
  }

  function key(e: KeyboardEvent) {
    if (ui.confirm) return;
    const mod = e.ctrlKey || e.metaKey;
    if (mod && !e.shiftKey && !e.altKey && e.key.toLowerCase() === "k") {
      e.preventDefault();
      e.stopPropagation();
      ui.palette = !ui.palette;
      return;
    }
    if (ui.palette) return;
    if (e.altKey && !mod && (e.key === "ArrowLeft" || e.key === "ArrowRight")) {
      e.preventDefault();
      if (e.key === "ArrowLeft") back();
      else forward();
      return;
    }
    const t = e.target as HTMLElement;
    if (t.closest?.(".cm-editor, input, textarea")) return;
    if (e.key === " " && !mod && !e.shiftKey) {
      const now = performance.now();
      if (now - lastSpace < 450) {
        e.preventDefault();
        lastSpace = 0;
        ui.palette = true;
        return;
      }
      lastSpace = now;
      if (ui.route.view === "lesson") e.preventDefault();
      return;
    }
    if (e.key === "Escape" && ui.route.view !== "lesson") {
      e.preventDefault();
      up();
    }
  }
</script>

<svelte:window onkeydowncapture={key} onmousedowncapture={mouse} onmouseupcapture={mouse} onauxclickcapture={mouse} />

<div class="shell">
  <TopBar />
  <main>
    {#if ready}
      <div class="view" class:on={ui.route.view === "dashboard"}>
        <Dashboard active={ui.route.view === "dashboard"} />
      </div>
      {#if ui.route.view === "phase"}
        <div class="view on"><Phase id={ui.route.id} /></div>
      {/if}
      {#if lessonID}
        <div class="view" class:on={ui.route.view === "lesson"}>
          <Lesson id={lessonID} active={ui.route.view === "lesson"} />
        </div>
      {/if}
    {:else if ui.loadError}
      <p class="err">{ui.loadError}</p>
    {/if}
  </main>
</div>

<Palette />
<ConfirmDialog />
<Toasts />

<style>
  .shell {
    height: 100%;
    display: flex;
    flex-direction: column;
  }
  main {
    flex: 1;
    min-height: 0;
    position: relative;
  }
  .view {
    position: absolute;
    inset: 0;
    visibility: hidden;
    pointer-events: none;
  }
  .view.on {
    visibility: visible;
    pointer-events: auto;
  }
  .err {
    color: var(--red);
    padding: 24px;
  }
</style>

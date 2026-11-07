<script lang="ts">
  import { ui, go, where, phase } from "../lib/state.svelte";
  import { Service } from "../lib/api";

  const isMac = navigator.userAgent.includes("Mac");

  type Crumb = { label: string; onclick?: () => void };

  const crumbs = $derived.by((): Crumb[] => {
    const r = ui.route;
    const out: Crumb[] = [{ label: "Dashboard", onclick: () => go({ view: "dashboard" }) }];
    if (r.view === "phase") {
      const p = phase(r.id);
      if (p) out.push({ label: `${p.number} · ${p.title}` });
    } else if (r.view === "lesson") {
      const w = where(r.id);
      if (w) {
        out.push({ label: `${w.phase.number} · ${w.phase.title}`, onclick: () => go({ view: "phase", id: w.phase.id }) });
        out.push({ label: `${w.chapter.number} ${w.chapter.title}`, onclick: () => go({ view: "phase", id: w.phase.id }) });
        out.push({ label: w.row.title });
      }
    }
    return out;
  });

  const syncLabel = $derived.by(() => {
    const s = ui.snap?.sync;
    switch (s?.state) {
      case "syncing":
        return { text: "syncing", cls: "busy" };
      case "failed":
        return { text: "sync failed", cls: "bad" };
      case "unsynced":
        return { text: "unsynced", cls: "warn" };
      case "synced":
        return { text: "synced", cls: "ok" };
      case "committed":
        return { text: "committed", cls: "ok" };
      case "local":
        return { text: "local only", cls: "dim" };
      default:
        return { text: "", cls: "dim" };
    }
  });

  const pct = $derived(ui.snap && ui.snap.span > 0 ? (100 * ui.snap.into) / ui.snap.span : 0);
</script>

<header class="bar" class:mac={isMac}>
  <nav class="crumbs">
    <button class="logo" onclick={() => go({ view: "dashboard" })} title="Dashboard">sysroad</button>
    {#each crumbs as c, i}
      {#if i > 0}<span class="sep">/</span>{/if}
      {#if c.onclick && i < crumbs.length - 1}
        <button class="crumb" onclick={c.onclick}>{c.label}</button>
      {:else if i > 0 || crumbs.length === 1}
        <span class="crumb here">{c.label}</span>
      {/if}
    {/each}
  </nav>

  <div class="right">
    {#if syncLabel.text}
      <button class="sync {syncLabel.cls}" title={ui.snap?.sync?.error || "Sync now"} onclick={() => Service.SyncNow()}>
        <span class="dot"></span>{syncLabel.text}
      </button>
    {/if}
    {#if ui.snap}
      <span class="streak" class:cold={ui.snap.streak === 0} title="Day streak (best {ui.snap.best})">
        <svg width="12" height="14" viewBox="0 0 12 14" aria-hidden="true"
          ><path d="M6 0c.5 2.6 3.2 3.9 3.2 7.1A3.2 3.2 0 0 1 6 10.4a3.2 3.2 0 0 1-3.2-3.3c0-1.3.6-2.2 1.3-3 .1 1.2.7 2 1.5 2.1C5.4 4 5 2.2 6 0zM1 9.5C1 12 3.2 14 6 14s5-2 5-4.5c0-.6-.1-1.2-.4-1.7-.3 2.3-2.2 4-4.6 4s-4.3-1.7-4.6-4c-.3.5-.4 1.1-.4 1.7z" /></svg
        >
        {ui.snap.streak}
      </span>
      <span class="lvl" title="{ui.snap.xp} XP · {ui.snap.span - ui.snap.into} to level {ui.snap.level + 1}">
        <span class="n">Lv {ui.snap.level}</span>
        <span class="track"><span class="fill" style="width:{pct}%"></span></span>
        <span class="xp">{ui.snap.into}/{ui.snap.span}</span>
      </span>
    {/if}
  </div>
</header>

<style>
  .bar {
    height: 40px;
    flex: none;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 16px;
    padding: 0 14px;
    border-bottom: 1px solid var(--line);
    background: var(--bg);
    font-size: 12.5px;
    --wails-draggable: drag;
  }
  .bar.mac {
    padding-left: 84px;
  }
  .bar button,
  .bar .lvl {
    --wails-draggable: no-drag;
  }
  .crumbs {
    display: flex;
    align-items: center;
    gap: 8px;
    min-width: 0;
    overflow: hidden;
    white-space: nowrap;
  }
  .logo {
    font-weight: 700;
    color: var(--accent);
    letter-spacing: 0.02em;
  }
  .sep {
    color: var(--line-strong);
  }
  .crumb {
    color: var(--fg-dim);
    overflow: hidden;
    text-overflow: ellipsis;
  }
  button.crumb:hover {
    color: var(--fg-bright);
  }
  .crumb.here {
    color: var(--fg-soft);
  }
  .right {
    display: flex;
    align-items: center;
    gap: 16px;
    flex: none;
  }
  .sync {
    display: flex;
    align-items: center;
    gap: 6px;
    color: var(--fg-dim);
  }
  .sync:hover {
    color: var(--fg-soft);
  }
  .dot {
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: var(--fg-dim);
  }
  .sync.ok .dot {
    background: var(--green);
  }
  .sync.warn .dot {
    background: var(--yellow);
  }
  .sync.bad {
    color: var(--red);
  }
  .sync.bad .dot {
    background: var(--red);
  }
  .sync.busy .dot {
    background: var(--accent);
    animation: pulse 0.9s ease-in-out infinite alternate;
  }
  @keyframes pulse {
    to {
      opacity: 0.25;
    }
  }
  .streak {
    display: flex;
    align-items: center;
    gap: 5px;
    color: var(--fg-bright);
    font-weight: 600;
  }
  .streak svg {
    fill: var(--orange);
  }
  .streak.cold {
    color: var(--fg-dim);
    font-weight: 400;
  }
  .streak.cold svg {
    fill: var(--fg-dim);
  }
  .lvl {
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .n {
    color: var(--fg-bright);
    font-weight: 600;
  }
  .track {
    width: 120px;
    height: 6px;
    border-radius: 6px;
    background: var(--line);
    overflow: hidden;
  }
  .fill {
    display: block;
    height: 100%;
    background: var(--accent);
    border-radius: 6px;
    transition: width 0.6s var(--ease);
  }
  .xp {
    color: var(--fg-dim);
    font-size: 11.5px;
    min-width: 7ch;
  }
</style>

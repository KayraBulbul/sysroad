<script lang="ts">
  import { ui, go, openLesson, where } from "../lib/state.svelte";

  let { active }: { active: boolean } = $props();

  const cur = $derived(ui.snap?.current ? where(ui.snap.current) : undefined);
  const pct = $derived(ui.snap && ui.snap.span > 0 ? (100 * ui.snap.into) / ui.snap.span : 0);
  const coursePct = $derived(ui.snap && ui.snap.total > 0 ? (100 * ui.snap.done) / ui.snap.total : 0);

  function hm(min: number) {
    if (min < 60) return `${min}m`;
    return `${Math.floor(min / 60)}h ${min % 60}m`;
  }

  function key(e: KeyboardEvent) {
    if (!active || ui.palette || ui.confirm) return;
    const t = e.target as HTMLElement;
    if (t.closest("input, textarea, .cm-editor")) return;
    if (e.key === "Enter" && cur && t.tagName !== "BUTTON") {
      e.preventDefault();
      openLesson(cur.row.id);
    }
  }
</script>

<svelte:window onkeydown={key} />

<div class="page">
  <div class="col">
    {#if ui.snap}
      <section class="hero">
        <div class="streak" class:cold={ui.snap.streak === 0}>
          <svg class="flame" width="30" height="36" viewBox="0 0 12 14" aria-hidden="true"
            ><path d="M6 0c.5 2.6 3.2 3.9 3.2 7.1A3.2 3.2 0 0 1 6 10.4a3.2 3.2 0 0 1-3.2-3.3c0-1.3.6-2.2 1.3-3 .1 1.2.7 2 1.5 2.1C5.4 4 5 2.2 6 0zM1 9.5C1 12 3.2 14 6 14s5-2 5-4.5c0-.6-.1-1.2-.4-1.7-.3 2.3-2.2 4-4.6 4s-4.3-1.7-4.6-4c-.3.5-.4 1.1-.4 1.7z" /></svg
          >
          <div class="big">{ui.snap.streak}</div>
          <div class="lbl">day streak</div>
          <div class="sub">best {ui.snap.best}</div>
        </div>

        <div class="continue">
          {#if cur}
            <div class="eyebrow">
              Continue · Phase {cur.phase.number} · {cur.phase.title}
            </div>
            <button class="title" onclick={() => openLesson(cur.row.id)}>{cur.row.title}</button>
            <div class="chapter">
              <span>{cur.chapter.number} {cur.chapter.title}</span>
              <span class="dots">
                {#each cur.chapter.lessons ?? [] as l}
                  <span class="d {l.status}" class:here={l.id === cur.row.id} class:soon={!l.written} title={l.title}></span>
                {/each}
              </span>
            </div>
          {:else}
            <div class="eyebrow">All caught up</div>
            <div class="title static">Nothing left on the main path</div>
          {/if}

          <div class="foot">
            <div class="lvl">
              <span class="n">Level {ui.snap.level}</span>
              <span class="track"><span class="fill" style="width:{pct}%"></span></span>
              <span class="meta">{ui.snap.into}/{ui.snap.span} XP</span>
            </div>
            {#if cur}
              <button class="btn primary" onclick={() => openLesson(cur.row.id)}>Continue <span class="k">⏎</span></button>
            {/if}
          </div>
          <div class="stats">
            <span>{ui.snap.done}/{ui.snap.total} lessons</span>
            {#if ui.snap.today > 0}<span>{hm(ui.snap.today)} today</span>{/if}
            {#if ui.snap.eta}<span>{ui.snap.eta} left at your pace</span>{/if}
          </div>
        </div>
      </section>

      <div class="head">
        <h2>{ui.snap.title}</h2>
        <span class="meta">{Math.round(coursePct)}%</span>
      </div>

      <ol class="phases">
        {#each ui.snap.phases ?? [] as p (p.id)}
          {@const frac = p.total > 0 ? p.done / p.total : 0}
          {@const complete = p.total > 0 && p.done === p.total}
          {@const isCur = cur?.phase.id === p.id}
          <li>
            <button class="phase" class:current={isCur} class:complete class:soon={p.total === 0} onclick={() => go({ view: "phase", id: p.id })}>
              <span class="num">{p.number}</span>
              <span class="body">
                <span class="top">
                  <span class="pt">{p.title}</span>
                  {#if p.optional}<span class="opt">optional</span>{/if}
                  {#if p.total > 0}
                    <span class="count">{p.done}/{p.total}</span>
                  {:else}
                    <span class="count">{p.lessons ? `${p.lessons} lessons · soon` : "soon"}</span>
                  {/if}
                </span>
                {#if p.summary}<span class="sum">{p.summary}</span>{/if}
                {#if p.total > 0}
                  <span class="track"><span class="fill" style="width:{frac * 100}%"></span></span>
                {/if}
              </span>
            </button>
          </li>
        {/each}
      </ol>
    {:else if ui.loadError}
      <p class="err">{ui.loadError}</p>
    {/if}
  </div>
</div>

<style>
  .hero {
    display: grid;
    grid-template-columns: 150px 1fr;
    border: 1px solid var(--line);
    border-radius: 12px;
    background: var(--panel);
    overflow: hidden;
  }
  .streak {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    padding: 22px 10px;
    border-right: 1px solid var(--line);
    gap: 2px;
  }
  .flame {
    fill: var(--orange);
    filter: drop-shadow(0 0 12px color-mix(in oklab, var(--orange) 45%, transparent));
    margin-bottom: 6px;
  }
  .cold .flame {
    fill: var(--fg-dim);
    filter: none;
  }
  .big {
    font-size: 38px;
    line-height: 1;
    font-weight: 700;
    color: var(--fg-bright);
  }
  .lbl {
    font-size: 12px;
    color: var(--fg-soft);
  }
  .sub {
    font-size: 11px;
    color: var(--fg-dim);
  }
  .continue {
    padding: 20px 22px 16px;
    display: flex;
    flex-direction: column;
    min-width: 0;
  }
  .eyebrow {
    font-size: 11.5px;
    color: var(--fg-dim);
    text-transform: uppercase;
    letter-spacing: 0.06em;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }
  .title {
    margin-top: 6px;
    font-size: 21px;
    font-weight: 650;
    color: var(--fg-bright);
    text-align: left;
    line-height: 1.25;
  }
  button.title:hover {
    color: var(--accent);
  }
  .chapter {
    margin-top: 8px;
    display: flex;
    align-items: center;
    gap: 12px;
    color: var(--fg-soft);
    font-size: 12.5px;
    flex-wrap: wrap;
  }
  .dots {
    display: flex;
    gap: 5px;
  }
  .d {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    border: 1.5px solid var(--line-strong);
  }
  .d.doing {
    border-color: var(--accent);
  }
  .d.done {
    background: var(--accent);
    border-color: var(--accent);
  }
  .d.here {
    box-shadow: 0 0 0 2px var(--bg), 0 0 0 3.5px var(--accent);
  }
  .d.soon {
    opacity: 0.4;
  }
  .foot {
    margin-top: 22px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 16px;
  }
  .lvl {
    display: flex;
    align-items: center;
    gap: 10px;
    flex: 1;
    min-width: 0;
  }
  .n {
    font-weight: 650;
    color: var(--fg-bright);
    white-space: nowrap;
  }
  .lvl .track {
    flex: 1;
    max-width: 260px;
  }
  .meta {
    color: var(--fg-dim);
    font-size: 12px;
    white-space: nowrap;
  }
  .stats {
    display: flex;
    gap: 18px;
    margin-top: 12px;
    font-size: 12px;
    color: var(--fg-dim);
    flex-wrap: wrap;
  }
  .track {
    display: block;
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
  .head {
    display: flex;
    align-items: baseline;
    justify-content: space-between;
    margin: 40px 2px 12px;
  }
  h2 {
    margin: 0;
    font-size: 14px;
    font-weight: 600;
    color: var(--fg-soft);
  }
  .phases {
    list-style: none;
    margin: 0;
    padding: 0;
    display: flex;
    flex-direction: column;
    gap: 8px;
  }
  .phase {
    width: 100%;
    display: flex;
    gap: 16px;
    align-items: flex-start;
    text-align: left;
    padding: 14px 16px;
    border: 1px solid var(--line);
    border-radius: 10px;
    transition: border-color 0.12s, background 0.12s;
  }
  .phase:hover {
    background: var(--hover);
    border-color: var(--line-strong);
  }
  .phase.soon .pt,
  .phase.soon .num {
    color: var(--fg-dim);
  }
  .phase.current {
    border-color: color-mix(in oklab, var(--accent) 55%, var(--line));
  }
  .num {
    flex: none;
    width: 30px;
    height: 30px;
    border-radius: 50%;
    display: grid;
    place-items: center;
    font-weight: 650;
    font-size: 13px;
    color: var(--fg-soft);
    border: 1.5px solid var(--line-strong);
  }
  .current .num {
    border-color: var(--accent);
    color: var(--accent);
  }
  .complete .num {
    background: var(--accent);
    border-color: var(--accent);
    color: var(--accent-fg);
  }
  .body {
    flex: 1;
    min-width: 0;
    display: flex;
    flex-direction: column;
    gap: 4px;
  }
  .top {
    display: flex;
    align-items: baseline;
    gap: 10px;
  }
  .pt {
    font-weight: 600;
    color: var(--fg-bright);
    flex: 1;
    min-width: 0;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  .opt {
    font-size: 11px;
    color: var(--fg-dim);
    border: 1px solid var(--line-strong);
    padding: 0 6px;
    border-radius: 4px;
  }
  .count {
    font-size: 12px;
    color: var(--fg-dim);
  }
  .sum {
    font-size: 12.5px;
    color: var(--fg-dim);
    display: -webkit-box;
    -webkit-line-clamp: 2;
    line-clamp: 2;
    -webkit-box-orient: vertical;
    overflow: hidden;
  }
  .body .track {
    margin-top: 6px;
    height: 4px;
  }
  .err {
    color: var(--red);
  }
</style>

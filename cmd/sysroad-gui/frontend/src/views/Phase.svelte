<script lang="ts">
  import { ui, phase, openLesson } from "../lib/state.svelte";
  import StatusIcon from "../components/StatusIcon.svelte";

  let { id }: { id: string } = $props();

  const p = $derived(phase(id));
  const frac = $derived(p && p.total > 0 ? p.done / p.total : 0);

  let page: HTMLDivElement | undefined = $state();
  $effect(() => {
    void id;
    page?.scrollTo({ top: 0 });
  });

  const kindLabel: Record<string, string> = {
    lesson: "",
    project: "project",
    capstone: "capstone",
    quiz: "quiz",
    setup: "setup",
    lab: "lab",
    video: "video",
    read: "read",
  };
</script>

<div class="page" bind:this={page}>
  {#if p}
    <div class="col">
      <header>
        <div class="eyebrow">Phase {p.number}{p.optional ? " · optional" : ""}</div>
        <h1>{p.title}</h1>
        {#if p.summary}<p class="sum">{p.summary}</p>{/if}
        <div class="prog">
          <span class="track"><span class="fill" style="width:{frac * 100}%"></span></span>
          <span class="meta">{p.total > 0 ? `${p.done}/${p.total}` : `${p.lessons} lessons · soon`}</span>
        </div>
      </header>

      {#each p.chapters ?? [] as c (c.id)}
        <section class="chapter">
          <div class="ch-head">
            <span class="ch-num">{c.number}</span>
            <h2>{c.title}</h2>
            <span class="meta">{c.done}/{c.total}</span>
          </div>
          {#if c.summary || c.project}
            <p class="ch-sum">
              {c.summary}
              {#if c.project}<span class="proj">builds: {c.project}</span>{/if}
            </p>
          {/if}
          <ul>
            {#each c.lessons ?? [] as l (l.id)}
              <li>
                <button class="row" class:soon={!l.written} class:current={ui.snap?.current === l.id} onclick={() => openLesson(l.id)}>
                  <StatusIcon status={l.status} written={l.written} />
                  <span class="lt">{l.title}</span>
                  {#if kindLabel[l.kind]}<span class="kind">{kindLabel[l.kind]}</span>{/if}
                  {#if l.optional}<span class="kind">optional</span>{/if}
                  {#if !l.written}<span class="kind">soon</span>{/if}
                  <span class="m">{l.minutes ? `${l.minutes} min` : ""}</span>
                  <span class="m xp">{l.xp} XP</span>
                </button>
              </li>
            {/each}
          </ul>
        </section>
      {/each}
    </div>
  {/if}
</div>

<style>
  .page {
    height: 100%;
    overflow-y: auto;
    scrollbar-gutter: stable both-edges;
  }
  .col {
    max-width: 760px;
    margin: 0 auto;
    padding: 36px 24px 80px;
  }
  .eyebrow {
    font-size: 11.5px;
    color: var(--fg-dim);
    text-transform: uppercase;
    letter-spacing: 0.06em;
  }
  h1 {
    margin: 6px 0 6px;
    font-size: 24px;
    font-weight: 650;
    color: var(--fg-bright);
    line-height: 1.25;
  }
  .sum {
    margin: 0;
    color: var(--fg-soft);
    font-size: 13px;
  }
  .prog {
    display: flex;
    align-items: center;
    gap: 12px;
    margin-top: 16px;
  }
  .track {
    flex: 1;
    height: 6px;
    border-radius: 6px;
    background: var(--line);
    overflow: hidden;
  }
  .fill {
    display: block;
    height: 100%;
    background: var(--accent);
    transition: width 0.6s var(--ease);
  }
  .meta {
    color: var(--fg-dim);
    font-size: 12px;
  }
  .chapter {
    margin-top: 34px;
  }
  .ch-head {
    display: flex;
    align-items: baseline;
    gap: 10px;
  }
  .ch-num {
    color: var(--accent);
    font-size: 12.5px;
    font-weight: 600;
  }
  h2 {
    margin: 0;
    flex: 1;
    font-size: 15px;
    font-weight: 600;
    color: var(--fg-bright);
  }
  .ch-sum {
    margin: 4px 0 0;
    color: var(--fg-dim);
    font-size: 12.5px;
  }
  .proj {
    display: inline-block;
    margin-left: 6px;
    color: var(--fg-soft);
  }
  ul {
    list-style: none;
    margin: 10px 0 0;
    padding: 0;
    border: 1px solid var(--line);
    border-radius: 10px;
    overflow: hidden;
  }
  li + li {
    border-top: 1px solid var(--line);
  }
  .row {
    width: 100%;
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 10px 14px;
    text-align: left;
    transition: background 0.1s;
  }
  .row:hover {
    background: var(--hover);
  }
  .row.current {
    background: var(--accent-soft);
  }
  .lt {
    flex: 1;
    min-width: 0;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    color: var(--fg);
  }
  .row:hover .lt {
    color: var(--fg-bright);
  }
  .soon .lt {
    color: var(--fg-dim);
  }
  .kind {
    font-size: 11px;
    color: var(--fg-dim);
    border: 1px solid var(--line-strong);
    padding: 0 6px;
    border-radius: 4px;
  }
  .m {
    font-size: 12px;
    color: var(--fg-dim);
    width: 6ch;
    text-align: right;
  }
  .m.xp {
    width: 6ch;
  }
</style>

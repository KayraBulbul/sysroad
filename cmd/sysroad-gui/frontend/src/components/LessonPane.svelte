<script lang="ts">
  import type { LessonDetail } from "../lib/api";
  import { Service } from "../lib/api";
  import { renderMarkdown } from "../lib/markdown";
  import { ui, where, openLesson, fail } from "../lib/state.svelte";

  let {
    d,
    showUTH,
    el = $bindable(),
    onhint,
    onuth,
  }: { d: LessonDetail; showUTH: boolean; el?: HTMLDivElement; onhint: () => void; onuth: () => void } = $props();

  const w = $derived(where(d.id));
  const done = $derived(d.status === "done");
  const prevTitle = $derived(d.prev ? where(d.prev)?.row.title : "");
  const nextTitle = $derived(d.next ? where(d.next)?.row.title : "");

  const kinds: Record<string, string> = { lesson: "Lesson", project: "Project", capstone: "Capstone", quiz: "Quiz", setup: "Setup", lab: "Lab", video: "Video", read: "Reading" };

  function click(e: MouseEvent) {
    const a = (e.target as HTMLElement).closest("a[data-href]") as HTMLAnchorElement | null;
    if (!a) return;
    e.preventDefault();
    Service.OpenURL(a.dataset.href!).catch(fail);
  }

  function open(url: string) {
    Service.OpenURL(url).catch(fail);
  }

  // Scroll to a newly revealed hint or the unlocked section.
  export function reveal(sel: string) {
    queueMicrotask(() => el?.querySelector(sel)?.scrollIntoView({ behavior: "smooth", block: "start" }));
  }

  // The work dir, shortened to ~/… for display.
  const workShown = $derived(d.workDir ? d.workDir.replace(/^\/(home|Users)\/[^/]+/, "~") : "");
</script>

<!-- Link clicks inside the rendered markdown open in the browser. -->
<!-- svelte-ignore a11y_no_noninteractive_element_interactions, a11y_click_events_have_key_events -->
<div class="pane" bind:this={el} tabindex="-1" onclick={click} role="document">
  <div class="inner">
    {#if w}
      <div class="chap">
        <span class="cn">{w.chapter.number}</span>
        <span class="ct">{w.chapter.title}</span>
        <span class="dots">
          {#each w.chapter.lessons ?? [] as l}
            <button class="dot {l.status}" class:here={l.id === d.id} class:soon={!l.written} title={l.title} onclick={() => openLesson(l.id)} aria-label={l.title}></button>
          {/each}
        </span>
      </div>
    {/if}

    <h1>{d.title}</h1>
    <div class="meta">
      <span class="chip">{kinds[d.kind] ?? d.kind}</span>
      {#if d.minutes}<span>~{d.minutes} min</span>{/if}
      <span class="xp">+{d.xp} XP</span>
      {#if d.optional}<span>optional</span>{/if}
      {#if d.platform}<span class="warn">{d.platform}</span>{/if}
      {#if done}<span class="done">✓ done</span>{:else if d.status === "doing"}<span class="doing">in progress</span>{/if}
    </div>
    {#if workShown && d.files?.length}
      <div class="path" title={d.workDir}>{workShown}/{d.files.join(", ")}</div>
    {/if}

    {#if d.body}
      <div class="md">{@html renderMarkdown(d.body)}</div>
    {:else}
      <p class="dim">This lesson isn't written yet. It arrives in a later sysroad update.</p>
    {/if}

    {#if d.url}
      <p><button class="btn" onclick={() => open(d.url)}>Open {new URL(d.url).hostname} ↗</button></p>
    {/if}

    {#if d.hintsTotal > 0}
      <section class="hints">
        {#each d.hints ?? [] as h, i}
          <div class="hint" id="hint-{i + 1}">
            <div class="hl">Hint {i + 1}/{d.hintsTotal}</div>
            <div class="md">{@html renderMarkdown(h)}</div>
          </div>
        {/each}
        {#if (d.hints?.length ?? 0) < d.hintsTotal}
          <button class="btn reveal" onclick={onhint}>
            Reveal hint <span class="dim">({d.hintsTotal - (d.hints?.length ?? 0)} left)</span> <span class="k">h</span>
          </button>
        {/if}
      </section>
    {/if}

    {#if d.hasUth}
      <section class="uth" id="uth">
        {#if !done}
          <div class="locked">
            <svg width="12" height="14" viewBox="0 0 12 14" aria-hidden="true"><path d="M3 6V4a3 3 0 0 1 6 0v2h.5A1.5 1.5 0 0 1 11 7.5v5A1.5 1.5 0 0 1 9.5 14h-7A1.5 1.5 0 0 1 1 12.5v-5A1.5 1.5 0 0 1 2.5 6zm1.5 0h3V4a1.5 1.5 0 0 0-3 0z" /></svg>
            Under the hood unlocks when you pass Submit
          </div>
        {:else if showUTH}
          <button class="uth-head" onclick={onuth}><span>Under the hood</span><span class="k">u</span></button>
          <div class="md">{@html renderMarkdown(d.uth)}</div>
        {:else}
          <button class="unlocked" onclick={onuth}>✦ Under the hood is unlocked <span class="k">u</span></button>
        {/if}
      </section>
    {/if}

    {#if d.video || d.deeper?.length}
      <section class="deeper">
        <div class="dl">Go deeper</div>
        {#if d.video}
          <button class="link" onclick={() => open(d.video!.url)}><span class="ico">▶</span>{d.video.title}</button>
        {/if}
        {#each d.deeper ?? [] as l}
          <button class="link" onclick={() => open(l.url)}><span class="ico">↗</span>{l.title}</button>
        {/each}
      </section>
    {/if}

    {#if d.note}
      <section class="note">
        <div class="dl">Your notes</div>
        <p>{d.note}</p>
      </section>
    {/if}

    <nav class="pn">
      {#if d.prev}
        <button class="pnb" onclick={() => openLesson(d.prev)}><span class="dim">← <kbd>[</kbd></span><span class="pt">{prevTitle}</span></button>
      {:else}<span></span>{/if}
      {#if d.next}
        <button class="pnb next" onclick={() => openLesson(d.next)}><span class="dim"><kbd>]</kbd> →</span><span class="pt">{nextTitle}</span></button>
      {/if}
    </nav>
  </div>
</div>

<style>
  .pane {
    height: 100%;
    overflow-y: auto;
    outline: none;
  }
  .inner {
    max-width: 760px;
    margin: 0 auto;
    padding: 26px 28px 40px;
  }
  .chap {
    display: flex;
    align-items: center;
    gap: 10px;
    font-size: 12px;
    color: var(--fg-dim);
    flex-wrap: wrap;
  }
  .cn {
    color: var(--accent);
    font-weight: 600;
  }
  .dots {
    display: flex;
    gap: 4px;
    margin-left: 4px;
  }
  .dot {
    width: 8px;
    height: 8px;
    border-radius: 50%;
    border: 1.5px solid var(--line-strong);
    padding: 0;
  }
  .dot.doing {
    border-color: var(--accent);
  }
  .dot.done {
    background: var(--accent);
    border-color: var(--accent);
  }
  .dot.here {
    box-shadow: 0 0 0 2px var(--bg), 0 0 0 3.5px var(--accent);
  }
  .dot.soon {
    opacity: 0.4;
  }
  h1 {
    margin: 10px 0 8px;
    font-size: 23px;
    line-height: 1.25;
    font-weight: 650;
    color: var(--fg-bright);
  }
  .meta {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 12px;
    font-size: 12px;
    color: var(--fg-dim);
  }
  .chip {
    color: var(--fg-soft);
    border: 1px solid var(--line-strong);
    padding: 0 7px;
    border-radius: 4px;
  }
  .xp {
    color: var(--yellow);
  }
  .warn {
    color: var(--yellow);
  }
  .done {
    color: var(--green);
  }
  .doing {
    color: var(--accent);
  }
  .path {
    margin-top: 8px;
    font-size: 11.5px;
    color: var(--fg-dim);
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  .dim {
    color: var(--fg-dim);
  }

  /* Rendered lesson markdown. */
  .md {
    margin-top: 18px;
    font-size: 13.5px;
    line-height: 1.7;
    color: var(--fg);
  }
  .md :global(p) {
    margin: 0 0 12px;
  }
  .md :global(h1),
  .md :global(h2),
  .md :global(h3),
  .md :global(h4) {
    color: var(--fg-bright);
    font-weight: 650;
    line-height: 1.3;
    margin: 26px 0 10px;
  }
  .md :global(h2) {
    font-size: 16px;
    padding-bottom: 6px;
    border-bottom: 1px solid var(--line);
  }
  .md :global(h3) {
    font-size: 14.5px;
  }
  .md :global(strong) {
    color: var(--fg-bright);
    font-weight: 650;
  }
  .md :global(em) {
    color: var(--fg-soft);
  }
  .md :global(ul),
  .md :global(ol) {
    margin: 0 0 12px;
    padding-left: 22px;
  }
  .md :global(li) {
    margin: 3px 0;
  }
  .md :global(li::marker) {
    color: var(--fg-dim);
  }
  .md :global(li:has(> input[type="checkbox"])) {
    list-style: none;
    margin-left: -20px;
  }
  .md :global(input[type="checkbox"]) {
    appearance: none;
    width: 13px;
    height: 13px;
    border: 1.5px solid var(--line-strong);
    border-radius: 3px;
    vertical-align: -2px;
    margin: 0 8px 0 0;
  }
  .md :global(code.inline) {
    font: inherit;
    font-size: 12.5px;
    color: var(--fg-bright);
    background: var(--bg-light);
    border-radius: 4px;
    padding: 1px 5px;
  }
  .md :global(pre.code) {
    position: relative;
    margin: 0 0 14px;
    padding: 12px 14px;
    background: var(--bg-dark);
    border: 1px solid var(--line);
    border-radius: 8px;
    overflow-x: auto;
    font-size: 12.5px;
    line-height: 1.6;
    tab-size: 4;
  }
  .md :global(pre.code code) {
    font: inherit;
  }
  .md :global(pre.code .lang) {
    position: absolute;
    top: 6px;
    right: 10px;
    font-size: 10.5px;
    color: var(--fg-dim);
  }
  .md :global(blockquote) {
    margin: 0 0 12px;
    padding: 2px 14px;
    border-left: 2px solid var(--accent);
    color: var(--fg-soft);
  }
  .md :global(table) {
    border-collapse: collapse;
    margin: 0 0 14px;
    font-size: 12.5px;
  }
  .md :global(th),
  .md :global(td) {
    border: 1px solid var(--line);
    padding: 5px 10px;
    text-align: left;
  }
  .md :global(th) {
    color: var(--fg-bright);
    background: var(--bg-dark);
  }
  .md :global(hr) {
    border: 0;
    border-top: 1px solid var(--line);
    margin: 20px 0;
  }
  .md :global(a) {
    text-decoration: underline;
    text-decoration-color: color-mix(in oklab, var(--accent) 45%, transparent);
    text-underline-offset: 3px;
  }

  section {
    margin-top: 26px;
  }
  .hints {
    display: flex;
    flex-direction: column;
    gap: 10px;
    align-items: flex-start;
  }
  .hint {
    width: 100%;
    border: 1px solid color-mix(in oklab, var(--yellow) 35%, var(--line));
    background: color-mix(in oklab, var(--yellow) 5%, transparent);
    border-radius: 8px;
    padding: 10px 14px 0;
  }
  .hl {
    font-size: 11.5px;
    color: var(--yellow);
    font-weight: 600;
  }
  .hint .md {
    margin-top: 4px;
  }
  .uth .locked {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 12.5px;
    color: var(--fg-dim);
    padding: 12px 14px;
    border: 1px dashed var(--line-strong);
    border-radius: 8px;
  }
  .locked svg {
    fill: var(--fg-dim);
  }
  .unlocked {
    width: 100%;
    text-align: left;
    padding: 12px 14px;
    border: 1px solid color-mix(in oklab, var(--accent) 50%, var(--line));
    background: var(--accent-soft);
    border-radius: 8px;
    color: var(--fg-bright);
    display: flex;
    justify-content: space-between;
  }
  .uth-head {
    width: 100%;
    display: flex;
    justify-content: space-between;
    align-items: baseline;
    font-size: 16px;
    font-weight: 650;
    color: var(--accent);
    padding-bottom: 6px;
    border-bottom: 1px solid var(--line);
  }
  .k {
    font-size: 11px;
    color: var(--fg-dim);
    font-weight: 400;
  }
  .deeper,
  .note {
    border-top: 1px solid var(--line);
    padding-top: 16px;
  }
  .dl {
    font-size: 11.5px;
    text-transform: uppercase;
    letter-spacing: 0.06em;
    color: var(--fg-dim);
    margin-bottom: 8px;
  }
  .link {
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 4px 0;
    color: var(--fg-soft);
    text-align: left;
  }
  .link:hover {
    color: var(--accent);
  }
  .ico {
    color: var(--fg-dim);
    width: 1ch;
  }
  .note p {
    margin: 0;
    white-space: pre-wrap;
    color: var(--fg-soft);
    font-size: 13px;
  }
  .pn {
    margin-top: 36px;
    display: flex;
    justify-content: space-between;
    gap: 16px;
    border-top: 1px solid var(--line);
    padding-top: 16px;
  }
  .pnb {
    display: flex;
    flex-direction: column;
    gap: 2px;
    text-align: left;
    max-width: 48%;
    font-size: 12px;
  }
  .pnb.next {
    text-align: right;
    align-items: flex-end;
  }
  .pt {
    color: var(--fg-soft);
    font-size: 13px;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    max-width: 100%;
  }
  .pnb:hover .pt {
    color: var(--accent);
  }
</style>

<script lang="ts">
  import type { Result } from "../lib/api";
  import { ansiToHtml } from "../lib/ansi";

  let {
    result,
    running,
    since,
    graded,
    next = "",
    onnext,
    el = $bindable(),
  }: {
    result: Result | null | undefined;
    running: string;
    since: number;
    graded: boolean;
    next?: string;
    onnext?: () => void;
    el?: HTMLDivElement;
  } = $props();

  let now = $state(Date.now());
  $effect(() => {
    const t = setInterval(() => (now = Date.now()), running ? 200 : 15000);
    return () => clearInterval(t);
  });

  const html = $derived(result?.output ? ansiToHtml(result.output.replace(/\s+$/, "")) : "");

  function ago(at: string) {
    const s = Math.max(0, (now - new Date(at).getTime()) / 1000);
    if (s < 45) return "just now";
    if (s < 3600) return `${Math.round(s / 60)}m ago`;
    if (s < 86400) return `${Math.round(s / 3600)}h ago`;
    return new Date(at).toLocaleDateString();
  }
</script>

<div class="out" bind:this={el} tabindex="-1">
  {#if running}
    <div class="head running">
      <span class="spin"></span>
      <span class="sum">{running === "submit" ? "Submitting" : "Running"}…</span>
      <span class="meta">{((now - since) / 1000).toFixed(1)}s</span>
    </div>
  {:else if result}
    <div class="head" class:ok={result.ok} class:bad={!result.ok}>
      <span class="sum">{result.summary}</span>
      {#if result.total > 0}
        <span class="chip">{result.passed}/{result.total} tests</span>
      {/if}
      <span class="meta">{result.mode} · {ago(result.at)}</span>
    </div>
  {/if}

  {#if result?.ok && result.mode === "submit" && !running && next}
    <div class="next">
      <button class="btn primary" onclick={onnext}>Next: {next} <span class="k">]</span></button>
    </div>
  {/if}

  {#if result && html && !running}
    <pre class="body">{@html html}</pre>
  {:else if running && result && html}
    <pre class="body stale">{@html html}</pre>
  {:else if !result && !running}
    <div class="empty">
      {#if graded}
        <span><kbd>Ctrl</kbd> <kbd>⏎</kbd> run</span>
        <span><kbd>Ctrl</kbd> <kbd>⇧</kbd> <kbd>⏎</kbd> submit</span>
      {:else}
        <span>Nothing to run for this step</span>
      {/if}
    </div>
  {/if}
</div>

<style>
  .out {
    height: 100%;
    overflow: auto;
    background: var(--bg-dark);
    outline: none;
  }
  .head {
    position: sticky;
    top: 0;
    display: flex;
    align-items: center;
    gap: 12px;
    padding: 9px 16px;
    background: var(--bg-dark);
    border-bottom: 1px solid var(--line);
    font-size: 13px;
    z-index: 1;
  }
  .sum {
    font-weight: 600;
    color: var(--fg-bright);
  }
  .ok .sum {
    color: var(--bright-green);
  }
  .bad .sum {
    color: var(--bright-red);
  }
  .chip {
    font-size: 11.5px;
    color: var(--fg-soft);
    border: 1px solid var(--line-strong);
    border-radius: 4px;
    padding: 0 6px;
  }
  .meta {
    margin-left: auto;
    font-size: 11.5px;
    color: var(--fg-dim);
  }
  .spin {
    width: 12px;
    height: 12px;
    border-radius: 50%;
    border: 2px solid var(--line-strong);
    border-top-color: var(--accent);
    animation: spin 0.7s linear infinite;
  }
  @keyframes spin {
    to {
      transform: rotate(360deg);
    }
  }
  .body {
    margin: 0;
    padding: 12px 16px 20px;
    font: inherit;
    font-size: 12.5px;
    line-height: 1.55;
    white-space: pre-wrap;
    word-break: break-word;
    color: var(--fg);
    tab-size: 4;
  }
  .next {
    padding: 16px;
  }
  .stale {
    opacity: 0.4;
  }
  .empty {
    height: 100%;
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 22px;
    color: var(--fg-dim);
    font-size: 12px;
  }
  .empty span {
    display: flex;
    gap: 4px;
    align-items: center;
  }
  /* ANSI colours from the theme. */
  .body :global(.ansi-red) {
    color: var(--red);
  }
  .body :global(.ansi-green) {
    color: var(--green);
  }
  .body :global(.ansi-yellow) {
    color: var(--yellow);
  }
  .body :global(.ansi-blue) {
    color: var(--blue);
  }
  .body :global(.ansi-magenta) {
    color: var(--magenta);
  }
  .body :global(.ansi-cyan) {
    color: var(--cyan);
  }
  .body :global(.ansi-white) {
    color: var(--fg-bright);
  }
  .body :global(.ansi-black),
  .body :global(.ansi-bright-black) {
    color: var(--fg-dim);
  }
  .body :global(.ansi-bright-red) {
    color: var(--bright-red);
  }
  .body :global(.ansi-bright-green) {
    color: var(--bright-green);
  }
  .body :global(.ansi-bright-yellow) {
    color: var(--bright-yellow);
  }
  .body :global(.ansi-bright-blue) {
    color: var(--bright-blue);
  }
  .body :global(.ansi-bright-magenta) {
    color: var(--bright-magenta);
  }
  .body :global(.ansi-bright-cyan) {
    color: var(--bright-cyan);
  }
  .body :global(.ansi-bright-white) {
    color: var(--fg-bright);
  }
  .body :global(.ansi-bold) {
    font-weight: 700;
  }
  .body :global(.ansi-dim) {
    opacity: 0.7;
  }
  .body :global(.ansi-italic) {
    font-style: italic;
  }
  .body :global(.ansi-underline) {
    text-decoration: underline;
  }
</style>

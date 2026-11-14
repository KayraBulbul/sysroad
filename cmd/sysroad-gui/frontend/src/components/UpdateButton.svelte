<script lang="ts">
  import { onMount } from "svelte";
  import { Service, on, type UpdateInfo } from "../lib/api";
  import { fail } from "../lib/state.svelte";

  // ⟳ in the top bar: a dot when there's an update, a menu to check,
  // update and restart. The Go side checks on launch and every 6 hours.

  let info = $state<UpdateInfo | null>(null);
  let open = $state(false);
  let logEl: HTMLPreElement | undefined = $state();

  onMount(() => {
    Service.UpdateStatus().then((i) => (info = i));
    return on<UpdateInfo>("update", (i) => (info = i));
  });

  $effect(() => {
    void info?.log?.length;
    if (logEl) logEl.scrollTop = logEl.scrollHeight;
  });

  const busy = $derived(info?.state === "checking" || info?.state === "building");
  const hasUpdate = $derived(!!info && (info.behind > 0 || info.state === "ready"));

  async function check() {
    try {
      info = await Service.CheckUpdates();
    } catch {
      // the error is in info.error too
    }
  }

  function ago(t: string) {
    const d = (Date.now() - new Date(t).getTime()) / 60000;
    if (!t || t.startsWith("0001")) return "never";
    if (d < 1) return "just now";
    if (d < 60) return `${Math.round(d)}m ago`;
    return `${Math.round(d / 60)}h ago`;
  }

  function toggle() {
    open = !open;
    if (open && info?.managed && info.state === "idle" && !hasUpdate) check();
  }

  function outside(e: MouseEvent) {
    if (open && !(e.target as HTMLElement).closest(".upd")) open = false;
  }
</script>

<svelte:window onmousedown={outside} />

<div class="upd">
  <button class="icon" class:spin={busy} class:has={hasUpdate} title="Updates" onclick={toggle} aria-label="Updates">
    <svg width="15" height="15" viewBox="0 0 16 16" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
      <path d="M13.5 8a5.5 5.5 0 0 1-9.6 3.7M2.5 8a5.5 5.5 0 0 1 9.6-3.7" />
      <path d="M12.6 1.6v2.9H9.7M3.4 14.4v-2.9h2.9" />
    </svg>
    {#if hasUpdate}<span class="dot"></span>{/if}
  </button>

  {#if open && info}
    <div class="menu">
      {#if !info.managed}
        <p class="dim">This copy wasn't installed with <code>make app</code>, so it can't update itself. Run that once from your checkout.</p>
      {:else}
        <div class="row">
          <span class="branch">{info.branch}</span>
          <span class="dim">{info.running ? info.running.slice(0, 7) : ""}</span>
          <span class="spacer"></span>
          <span class="dim">checked {ago(info.checked)}</span>
        </div>

        {#if info.state === "ready"}
          <p class="ok">Updated. Restart to use it.</p>
          <button class="btn primary" onclick={() => Service.RestartToUpdate().catch(fail)}>Restart now</button>
        {:else if info.state === "building"}
          <p>Building the update…</p>
        {:else if info.state === "checking"}
          <p class="dim">Checking…</p>
        {:else if info.behind > 0}
          <p><strong>{info.behind}</strong> new commit{info.behind === 1 ? "" : "s"}</p>
          <ul>
            {#each info.commits ?? [] as c}<li>{c}</li>{/each}
          </ul>
          <button class="btn primary" onclick={() => Service.Update().catch(fail)}>Update</button>
        {:else}
          <p class="ok">Up to date</p>
        {/if}

        {#if info.error}<p class="err">{info.error}</p>{/if}
        {#if info.log?.length && (info.state === "building" || info.state === "failed")}
          <pre bind:this={logEl}>{info.log.join("\n")}</pre>
        {/if}

        {#if info.state === "idle" || info.state === "failed"}
          <button class="link" onclick={check}>Check now</button>
        {/if}
      {/if}
    </div>
  {/if}
</div>

<style>
  .upd {
    position: relative;
    --wails-draggable: no-drag;
  }
  .icon {
    position: relative;
    display: grid;
    place-items: center;
    width: 24px;
    height: 24px;
    border-radius: 6px;
    color: var(--fg-dim);
  }
  .icon:hover {
    color: var(--fg-bright);
    background: var(--hover);
  }
  .icon.has {
    color: var(--accent);
  }
  .icon.spin svg {
    animation: spin 1s linear infinite;
  }
  .dot {
    position: absolute;
    top: 3px;
    right: 3px;
    width: 6px;
    height: 6px;
    border-radius: 50%;
    background: var(--accent);
    box-shadow: 0 0 0 2px var(--bg);
  }
  .menu {
    position: absolute;
    right: 0;
    top: 32px;
    width: 340px;
    z-index: 70;
    background: var(--bg-dark);
    border: 1px solid var(--line-strong);
    border-radius: 10px;
    padding: 12px 14px;
    box-shadow: 0 16px 50px color-mix(in oklab, black 40%, transparent);
    font-size: 12.5px;
    display: flex;
    flex-direction: column;
    gap: 8px;
    align-items: flex-start;
  }
  .row {
    display: flex;
    align-items: baseline;
    gap: 8px;
    width: 100%;
  }
  .spacer {
    flex: 1;
  }
  .branch {
    color: var(--accent);
    font-weight: 600;
  }
  p {
    margin: 0;
    color: var(--fg-soft);
  }
  .dim {
    color: var(--fg-dim);
  }
  .ok {
    color: var(--green);
  }
  .err {
    color: var(--red);
  }
  ul {
    margin: 0;
    padding-left: 16px;
    max-height: 160px;
    overflow: auto;
    color: var(--fg);
    width: 100%;
  }
  li {
    margin: 2px 0;
  }
  pre {
    margin: 0;
    width: 100%;
    max-height: 160px;
    overflow: auto;
    background: var(--bg-darker);
    border-radius: 6px;
    padding: 8px;
    font: inherit;
    font-size: 11px;
    color: var(--fg-dim);
    white-space: pre-wrap;
    word-break: break-word;
  }
  code {
    font: inherit;
    color: var(--fg-bright);
  }
  .link {
    color: var(--fg-dim);
    font-size: 12px;
  }
  .link:hover {
    color: var(--accent);
  }
</style>

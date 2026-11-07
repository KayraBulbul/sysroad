<script lang="ts">
  import { ui } from "../lib/state.svelte";

  let yesBtn: HTMLButtonElement | undefined = $state();

  $effect(() => {
    if (ui.confirm) queueMicrotask(() => yesBtn?.focus());
  });

  function answer(yes: boolean) {
    const c = ui.confirm;
    ui.confirm = null;
    if (yes) c?.onYes();
  }

  function key(e: KeyboardEvent) {
    if (!ui.confirm) return;
    e.stopPropagation();
    if (e.key === "y" || e.key === "Y" || e.key === "Enter") {
      e.preventDefault();
      answer(true);
    } else if (e.key === "Escape" || e.key === "n" || e.key === "N") {
      e.preventDefault();
      answer(false);
    }
  }
</script>

<svelte:window onkeydowncapture={key} />

{#if ui.confirm}
  <div class="scrim" role="presentation" onclick={() => answer(false)}>
    <div class="dialog" role="dialog" aria-modal="true" tabindex="-1" onclick={(e) => e.stopPropagation()} onkeydown={() => {}}>
      <h3>{ui.confirm.title}</h3>
      {#if ui.confirm.detail}<p>{ui.confirm.detail}</p>{/if}
      <div class="row">
        <button class="btn ghost" onclick={() => answer(false)}>Cancel <span class="k">n</span></button>
        <button bind:this={yesBtn} class="btn primary" class:danger={ui.confirm.danger} onclick={() => answer(true)}>
          {ui.confirm.yes} <span class="k">y</span>
        </button>
      </div>
    </div>
  </div>
{/if}

<style>
  .scrim {
    position: fixed;
    inset: 0;
    background: color-mix(in oklab, var(--bg-darker) 60%, transparent);
    display: grid;
    place-items: center;
    z-index: 90;
  }
  .dialog {
    background: var(--bg-dark);
    border: 1px solid var(--line-strong);
    border-radius: 10px;
    padding: 20px 22px 16px;
    width: min(440px, 90vw);
    box-shadow: 0 20px 60px color-mix(in oklab, black 45%, transparent);
  }
  h3 {
    margin: 0 0 6px;
    font-size: 15px;
    color: var(--fg-bright);
  }
  p {
    margin: 0;
    color: var(--fg-dim);
    font-size: 13px;
  }
  .row {
    display: flex;
    justify-content: flex-end;
    gap: 8px;
    margin-top: 18px;
  }
  .danger {
    background: var(--red) !important;
    border-color: var(--red) !important;
  }
</style>

<script lang="ts">
  import { ui } from "../lib/state.svelte";
  import { fly } from "svelte/transition";
</script>

<div class="toasts">
  {#each ui.toasts as t (t.id)}
    <div class="toast {t.kind}" in:fly={{ y: 8, duration: 160 }} out:fly={{ y: 8, duration: 120 }}>{t.text}</div>
  {/each}
</div>

<style>
  .toasts {
    position: fixed;
    bottom: 18px;
    left: 50%;
    transform: translateX(-50%);
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 8px;
    z-index: 100;
    pointer-events: none;
  }
  .toast {
    background: var(--bg-dark);
    border: 1px solid var(--line-strong);
    color: var(--fg-bright);
    padding: 8px 14px;
    border-radius: var(--radius);
    font-size: 13px;
    box-shadow: 0 8px 30px color-mix(in oklab, black 35%, transparent);
    max-width: 70vw;
  }
  .toast.good {
    border-color: color-mix(in oklab, var(--green) 60%, var(--line));
  }
  .toast.bad {
    border-color: color-mix(in oklab, var(--red) 65%, var(--line));
    color: var(--bright-red);
  }
  .toast.xp {
    border-color: var(--accent);
    color: var(--fg-bright);
  }
</style>

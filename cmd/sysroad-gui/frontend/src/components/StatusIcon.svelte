<script lang="ts">
  // ○ todo · ◐ in progress · ● done · – skipped; dimmed when not written yet.
  let { status, written = true, size = 14 }: { status: string; written?: boolean; size?: number } = $props();
</script>

<svg class="si {status}" class:soon={!written} width={size} height={size} viewBox="0 0 16 16" aria-label={status}>
  {#if status === "done"}
    <circle cx="8" cy="8" r="7" class="fill" />
    <path d="M4.8 8.3l2.1 2.1 4.3-4.6" class="tick" />
  {:else if status === "doing"}
    <circle cx="8" cy="8" r="6.25" class="ring" />
    <path d="M8 1.75a6.25 6.25 0 0 1 0 12.5z" class="half" />
  {:else if status === "skipped"}
    <circle cx="8" cy="8" r="6.25" class="ring dashed" />
    <path d="M5.2 8h5.6" class="ring" />
  {:else}
    <circle cx="8" cy="8" r="6.25" class="ring" />
  {/if}
</svg>

<style>
  .si {
    flex: none;
    display: block;
  }
  .ring {
    fill: none;
    stroke: var(--line-strong);
    stroke-width: 1.5;
  }
  .dashed {
    stroke-dasharray: 2 2;
  }
  .doing .ring {
    stroke: var(--accent);
  }
  .half {
    fill: var(--accent);
  }
  .fill {
    fill: var(--accent);
  }
  .tick {
    fill: none;
    stroke: var(--accent-fg);
    stroke-width: 1.8;
    stroke-linecap: round;
    stroke-linejoin: round;
  }
  .soon .ring {
    stroke: var(--line);
  }
</style>

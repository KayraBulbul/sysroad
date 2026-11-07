<script lang="ts">
  import { ui, go, openLesson } from "../lib/state.svelte";
  import { fuzzyMatch } from "../lib/fuzzy";
  import StatusIcon from "./StatusIcon.svelte";

  type Item = { kind: "phase" | "lesson" | "page"; id: string; title: string; sub: string; status?: string; written?: boolean; hay: string };

  let query = $state("");
  let sel = $state(0);
  let input: HTMLInputElement | undefined = $state();
  let list: HTMLDivElement | undefined = $state();

  const items = $derived.by(() => {
    const out: Item[] = [{ kind: "page", id: "dashboard", title: "Dashboard", sub: "home", hay: "dashboard home" }];
    for (const p of ui.snap?.phases ?? []) {
      out.push({ kind: "phase", id: p.id, title: p.title, sub: `Phase ${p.number}`, hay: `phase ${p.number} ${p.title}` });
      for (const c of p.chapters ?? []) {
        for (const l of c.lessons ?? []) {
          out.push({ kind: "lesson", id: l.id, title: l.title, sub: `${c.number} ${c.title}`, status: l.status, written: l.written, hay: `${l.title} ${c.number} ${c.title} ${l.id}` });
        }
      }
    }
    return out;
  });

  const results = $derived.by(() => {
    if (!query.trim()) {
      const cur = ui.snap?.current;
      const curItem = items.find((i) => i.kind === "lesson" && i.id === cur);
      const rest = items.filter((i) => i !== curItem && (i.kind !== "lesson" || i.written));
      return (curItem ? [curItem, ...rest] : rest).slice(0, 60);
    }
    return items
      .map((i) => ({ i, s: fuzzyMatch(query, i.title, i.hay) }))
      .filter((x) => x.s > 0)
      .sort((a, b) => b.s - a.s)
      .slice(0, 60)
      .map((x) => x.i);
  });

  $effect(() => {
    if (ui.palette) {
      query = "";
      sel = 0;
      queueMicrotask(() => input?.focus());
    }
  });

  $effect(() => {
    void query;
    sel = 0;
  });

  $effect(() => {
    list?.querySelector(`[data-i="${sel}"]`)?.scrollIntoView({ block: "nearest" });
  });

  function choose(it: Item | undefined) {
    if (!it) return;
    ui.palette = false;
    if (it.kind === "lesson") openLesson(it.id);
    else if (it.kind === "phase") go({ view: "phase", id: it.id });
    else go({ view: "dashboard" });
  }

  function key(e: KeyboardEvent) {
    const n = results.length;
    if (e.key === "Escape") {
      e.preventDefault();
      ui.palette = false;
    } else if (e.key === "ArrowDown" || (e.ctrlKey && (e.key === "j" || e.key === "n"))) {
      e.preventDefault();
      sel = (sel + 1) % Math.max(n, 1);
    } else if (e.key === "ArrowUp" || (e.ctrlKey && (e.key === "k" || e.key === "p"))) {
      e.preventDefault();
      sel = (sel - 1 + n) % Math.max(n, 1);
    } else if (e.key === "Enter") {
      e.preventDefault();
      choose(results[sel]);
    }
    e.stopPropagation();
  }
</script>

{#if ui.palette}
  <div class="scrim" role="presentation" onclick={() => (ui.palette = false)}>
    <div class="pal" role="dialog" aria-modal="true" tabindex="-1" onclick={(e) => e.stopPropagation()} onkeydown={key}>
      <input bind:this={input} bind:value={query} placeholder="Jump to a lesson or phase…" spellcheck="false" />
      <div class="list" bind:this={list}>
        {#each results as it, i (it.kind + it.id)}
          <button class="item" class:sel={i === sel} data-i={i} onmousemove={() => (sel = i)} onclick={() => choose(it)}>
            <span class="ic">
              {#if it.kind === "lesson"}
                <StatusIcon status={it.status ?? "todo"} written={it.written} size={13} />
              {:else}
                <span class="tag">{it.kind === "phase" ? "P" : "⌂"}</span>
              {/if}
            </span>
            <span class="t" class:soon={it.kind === "lesson" && !it.written}>{it.title}</span>
            <span class="s">{it.sub}</span>
          </button>
        {:else}
          <div class="empty">Nothing matches</div>
        {/each}
      </div>
    </div>
  </div>
{/if}

<style>
  .scrim {
    position: fixed;
    inset: 0;
    z-index: 80;
    background: color-mix(in oklab, var(--bg-darker) 45%, transparent);
    display: flex;
    justify-content: center;
    align-items: flex-start;
    padding-top: 12vh;
  }
  .pal {
    width: min(640px, 92vw);
    background: var(--bg-dark);
    border: 1px solid var(--line-strong);
    border-radius: 10px;
    box-shadow: 0 24px 70px color-mix(in oklab, black 45%, transparent);
    overflow: hidden;
  }
  input {
    width: 100%;
    background: transparent;
    border: 0;
    border-bottom: 1px solid var(--line);
    color: var(--fg-bright);
    font: inherit;
    font-size: 15px;
    padding: 14px 16px;
    outline: none;
    caret-color: var(--accent);
  }
  input::placeholder {
    color: var(--fg-dim);
  }
  .list {
    max-height: 52vh;
    overflow: auto;
    padding: 6px;
  }
  .item {
    display: flex;
    align-items: center;
    gap: 10px;
    width: 100%;
    padding: 7px 10px;
    border-radius: 6px;
    text-align: left;
  }
  .item.sel {
    background: var(--accent-soft);
  }
  .item.sel .t {
    color: var(--fg-bright);
  }
  .ic {
    width: 16px;
    display: grid;
    place-items: center;
  }
  .tag {
    font-size: 10px;
    color: var(--fg-dim);
    border: 1px solid var(--line-strong);
    border-radius: 3px;
    width: 14px;
    height: 14px;
    display: grid;
    place-items: center;
  }
  .t {
    flex: 1;
    min-width: 0;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  .t.soon {
    color: var(--fg-dim);
  }
  .s {
    color: var(--fg-dim);
    font-size: 12px;
    white-space: nowrap;
    max-width: 45%;
    overflow: hidden;
    text-overflow: ellipsis;
  }
  .empty {
    padding: 14px;
    color: var(--fg-dim);
  }
</style>

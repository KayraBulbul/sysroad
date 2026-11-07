<script lang="ts">
  import type { Quiz, Outcome } from "../../bindings/github.com/KayraBulbul/sysroad/internal/app/models";
  import { Service } from "../lib/api";
  import { renderMarkdown } from "../lib/markdown";
  import { toast, fail } from "../lib/state.svelte";

  let {
    id,
    quiz,
    el = $bindable(),
    onanswered,
  }: { id: string; quiz: Quiz; el?: HTMLDivElement; onanswered: (q: Quiz, o: Outcome | null) => void } = $props();

  let busy = false;
  // The last wrong pick, so it can shake.
  let wrong = $state("");

  const items = $derived(quiz.items ?? []);
  const next = $derived(items.findIndex((it) => !it.solved));

  async function answer(q: number, pick: number) {
    const it = items[q];
    if (busy || !it || it.solved || it.tried?.includes(pick)) return;
    busy = true;
    try {
      const res = await Service.AnswerQuiz(id, q, pick);
      if (!res) return;
      if (!res.correct) {
        wrong = `${q}:${pick}`;
        setTimeout(() => (wrong = ""), 450);
      }
      onanswered(res.quiz, res.outcome);
      if (res.correct && !res.outcome.completed) {
        const n = res.quiz.items?.findIndex((x) => !x.solved) ?? -1;
        if (n >= 0) queueMicrotask(() => el?.querySelector(`#q-${n}`)?.scrollIntoView({ block: "nearest", behavior: "smooth" }));
      }
    } catch (e) {
      fail(e);
    } finally {
      busy = false;
    }
  }

  // answerNext picks option n (from 0) for the first unsolved question: the
  // 1–9 keys.
  export function answerNext(n: number) {
    if (next < 0) return toast("All answered — Retake to go again");
    if (n >= (items[next].options?.length ?? 0)) return;
    answer(next, n);
  }

  function code(src: string) {
    return renderMarkdown("```cpp\n" + src.replace(/\n+$/, "") + "\n```");
  }
</script>

<div class="quiz" bind:this={el} tabindex="-1">
  <div class="progress">
    <span class="bar"><span class="fill" style="width:{quiz.total ? (100 * quiz.solved) / quiz.total : 0}%"></span></span>
    <span class="count">{quiz.solved}/{quiz.total}</span>
  </div>
  {#each items as it, q (q)}
    <section class="card" id="q-{q}" class:solved={it.solved} class:current={q === next} class:later={next >= 0 && q > next}>
      <div class="qh">
        <span class="qn">Q{q + 1}</span>
        {#if it.solved}<span class="ok">✓</span>{:else if q === next}<span class="hint">press 1–{it.options?.length}</span>{/if}
      </div>
      <div class="md">{@html renderMarkdown(it.q)}</div>
      {#if it.code}<div class="md">{@html code(it.code)}</div>{/if}
      <div class="opts">
        {#each it.options ?? [] as opt, j (j)}
          {@const right = it.solved && j === it.answer}
          {@const tried = !right && it.tried?.includes(j)}
          <button class="opt" class:right class:tried class:shake={wrong === `${q}:${j}`} disabled={it.solved || tried} onclick={() => answer(q, j)}>
            <span class="key">{right ? "✓" : tried ? "✗" : j + 1}</span>
            <span class="md inline">{@html renderMarkdown(opt)}</span>
          </button>
        {/each}
      </div>
      {#if it.solved && it.why}
        <div class="why md">{@html renderMarkdown(it.why)}</div>
      {/if}
    </section>
  {/each}
  {#if next < 0}
    <p class="done">✓ Quiz complete</p>
  {/if}
</div>

<style>
  .quiz {
    height: 100%;
    overflow-y: auto;
    padding: 16px 20px 40px;
    background: var(--bg-dark);
    outline: none;
  }
  .progress {
    display: flex;
    align-items: center;
    gap: 10px;
    margin-bottom: 14px;
    font-size: 12px;
    color: var(--fg-dim);
  }
  .bar {
    flex: 1;
    height: 4px;
    border-radius: 2px;
    background: var(--line);
    overflow: hidden;
  }
  .fill {
    display: block;
    height: 100%;
    background: var(--accent);
    transition: width 0.3s var(--ease);
  }
  .card {
    border: 1px solid var(--line);
    border-radius: 8px;
    background: var(--bg);
    padding: 12px 16px 14px;
    margin-bottom: 12px;
  }
  .card.current {
    border-color: color-mix(in oklab, var(--accent) 55%, var(--line));
  }
  .card.later {
    opacity: 0.55;
  }
  .qh {
    display: flex;
    align-items: center;
    gap: 10px;
    font-size: 11.5px;
    color: var(--fg-dim);
    margin-bottom: 4px;
  }
  .qn {
    color: var(--accent);
    font-weight: 650;
  }
  .solved .qn,
  .ok {
    color: var(--green);
  }
  .opts {
    display: flex;
    flex-direction: column;
    gap: 6px;
    margin-top: 6px;
  }
  .opt {
    display: flex;
    align-items: baseline;
    gap: 10px;
    text-align: left;
    padding: 7px 10px;
    border: 1px solid var(--line);
    border-radius: 6px;
    color: var(--fg);
    font-size: 13px;
  }
  .opt:hover:not(:disabled) {
    background: var(--hover);
    border-color: var(--line-strong);
  }
  .opt .key {
    flex: none;
    width: 18px;
    text-align: center;
    color: var(--fg-dim);
    font-size: 12px;
  }
  .opt.right {
    border-color: color-mix(in oklab, var(--green) 60%, var(--line));
    background: color-mix(in oklab, var(--green) 10%, transparent);
  }
  .opt.right .key {
    color: var(--green);
  }
  .opt.tried {
    opacity: 0.6;
    border-color: color-mix(in oklab, var(--red) 45%, var(--line));
  }
  .opt.tried .key {
    color: var(--red);
  }
  .opt:disabled {
    cursor: default;
  }
  .shake {
    animation: shake 0.4s;
  }
  @keyframes shake {
    20%,
    60% {
      transform: translateX(-4px);
    }
    40%,
    80% {
      transform: translateX(4px);
    }
  }
  .why {
    margin-top: 10px;
    padding: 8px 12px;
    border-left: 2px solid var(--green);
    color: var(--fg-soft);
  }
  .done {
    text-align: center;
    color: var(--green);
    font-size: 15px;
    margin-top: 18px;
  }
  .md {
    font-size: 13.5px;
    line-height: 1.65;
  }
  .md :global(p) {
    margin: 0 0 8px;
  }
  .md.inline :global(p),
  .why :global(p:last-child) {
    margin: 0;
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
    margin: 4px 0 8px;
    padding: 10px 12px;
    background: var(--bg-dark);
    border: 1px solid var(--line);
    border-radius: 6px;
    overflow-x: auto;
    font-size: 12.5px;
    line-height: 1.55;
    tab-size: 4;
  }
  .md :global(pre.code code) {
    font: inherit;
  }
  .md :global(pre.code .lang) {
    display: none;
  }
  .md :global(strong) {
    color: var(--fg-bright);
  }
</style>

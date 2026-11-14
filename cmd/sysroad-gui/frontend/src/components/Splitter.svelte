<script lang="ts">
  // A drag handle between two panes. It reports the new size as a fraction
  // of its container (the parent element) along its axis.
  let {
    dir,
    value,
    min = 0.15,
    max = 0.85,
    onchange,
    ondone,
  }: {
    dir: "x" | "y";
    value: number;
    min?: number;
    max?: number;
    onchange: (v: number) => void;
    ondone?: (v: number) => void;
  } = $props();

  let el: HTMLDivElement;
  let dragging = $state(false);

  function down(e: PointerEvent) {
    e.preventDefault();
    const box = el.parentElement!.getBoundingClientRect();
    dragging = true;
    el.setPointerCapture(e.pointerId);
    let v = value;
    const move = (ev: PointerEvent) => {
      const f = dir === "x" ? (ev.clientX - box.left) / box.width : (ev.clientY - box.top) / box.height;
      v = Math.min(max, Math.max(min, f));
      onchange(v);
    };
    const upFn = () => {
      dragging = false;
      el.removeEventListener("pointermove", move);
      el.removeEventListener("pointerup", upFn);
      ondone?.(v);
    };
    el.addEventListener("pointermove", move);
    el.addEventListener("pointerup", upFn);
  }
</script>

<div
  bind:this={el}
  class="split {dir}"
  class:dragging
  role="separator"
  aria-orientation={dir === "x" ? "vertical" : "horizontal"}
  onpointerdown={down}
></div>

<style>
  .split {
    position: relative;
    flex: none;
    z-index: 5;
  }
  .split::after {
    content: "";
    position: absolute;
    inset: 0;
    background: var(--line);
    transition: background 0.12s;
  }
  .split.x {
    width: 1px;
    cursor: col-resize;
  }
  .split::before {
    content: "";
    position: absolute;
  }
  .split.x::before {
    inset: 0 -4px;
  }
  .split.y::before {
    inset: -4px 0;
  }
  .split.y {
    height: 1px;
    cursor: row-resize;
  }
  .split:hover::after,
  .split.dragging::after {
    background: var(--accent);
  }
</style>

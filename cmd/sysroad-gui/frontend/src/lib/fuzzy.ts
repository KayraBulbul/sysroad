// A small subsequence matcher for the palette: every query character must
// appear in order. Contiguous runs and word starts score higher.
export function fuzzyScore(query: string, text: string): number {
  if (!query) return 1;
  const q = query.toLowerCase();
  const s = text.toLowerCase();
  let score = 0;
  let qi = 0;
  let prev = -2;
  for (let i = 0; i < s.length && qi < q.length; i++) {
    if (s[i] !== q[qi]) continue;
    let pts = 1;
    if (i === prev + 1) pts += 4;
    if (i === 0 || /[\s\-_./:·]/.test(s[i - 1])) pts += 3;
    score += pts;
    prev = i;
    qi++;
  }
  if (qi < q.length) return 0;
  return score + (s.startsWith(q) ? 10 : 0) - s.length * 0.01;
}

// fuzzyMatch scores each space-separated word of the query on its own, so
// "wc fl" finds "wc flags"; every word has to match somewhere.
export function fuzzyMatch(query: string, title: string, hay: string): number {
  let total = 0;
  for (const word of query.trim().split(/\s+/)) {
    const s = Math.max(fuzzyScore(word, title) * 1.5, fuzzyScore(word, hay));
    if (s <= 0) return 0;
    total += s;
  }
  return total;
}

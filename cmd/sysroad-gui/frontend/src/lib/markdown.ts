import { Marked, type Tokens } from "marked";
import { escape, highlightCpp, isCpp } from "./highlight";

// Lesson markdown → HTML. Code blocks get the editor's highlighting; links
// carry data-href so the click handler opens them in the browser.
const md = new Marked({ gfm: true, breaks: false });

md.use({
  renderer: {
    code({ text, lang }: Tokens.Code) {
      const l = (lang ?? "").trim().split(/\s+/)[0];
      const body = l && isCpp(l) ? highlightCpp(text) : escape(text);
      const label = l && l !== "text" ? `<span class="lang">${escape(l)}</span>` : "";
      return `<pre class="code">${label}<code>${body}</code></pre>`;
    },
    codespan({ text }: Tokens.Codespan) {
      // marked hands over the raw text: `<cctype>` must not become a tag.
      return `<code class="inline">${escape(text)}</code>`;
    },
    link({ href, title, tokens }: Tokens.Link) {
      const inner = this.parser.parseInline(tokens);
      const t = title ? ` title="${escape(title)}"` : "";
      return `<a href="#" data-href="${escape(href)}"${t}>${inner}</a>`;
    },
  },
});

const cache = new Map<string, string>();

export function renderMarkdown(src: string): string {
  let html = cache.get(src);
  if (html === undefined) {
    html = md.parse(src, { async: false }) as string;
    if (cache.size > 400) cache.clear();
    cache.set(src, html);
  }
  return html;
}

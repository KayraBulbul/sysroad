import { tags as t, tagHighlighter, highlightCode } from "@lezer/highlight";
import { parser as cppParser } from "@lezer/cpp";
import { cppLanguage } from "@codemirror/lang-cpp";

// One highlighter for the editor and for code in lessons, so both colour
// C++ the same way. It emits tok-* classes styled in app.css from the theme.
export const highlighter = tagHighlighter([
  { tag: [t.keyword, t.controlKeyword, t.modifier, t.operatorKeyword, t.definitionKeyword, t.moduleKeyword], class: "tok-keyword" },
  { tag: [t.typeName, t.standard(t.typeName)], class: "tok-typeName" },
  { tag: [t.className, t.namespace], class: "tok-className" },
  { tag: [t.function(t.variableName), t.function(t.propertyName), t.function(t.definition(t.variableName))], class: "tok-function" },
  { tag: [t.macroName], class: "tok-macroName" },
  { tag: [t.string, t.character], class: "tok-string" },
  { tag: [t.escape, t.special(t.string)], class: "tok-escape" },
  { tag: [t.number, t.bool, t.atom, t.null], class: "tok-number" },
  { tag: [t.comment, t.lineComment, t.blockComment], class: "tok-comment" },
  { tag: [t.meta, t.processingInstruction], class: "tok-meta" },
  { tag: [t.operator, t.derefOperator, t.arithmeticOperator, t.logicOperator, t.compareOperator, t.updateOperator], class: "tok-operator" },
  { tag: [t.punctuation, t.bracket, t.separator], class: "tok-punctuation" },
  { tag: [t.propertyName], class: "tok-propertyName" },
  { tag: [t.definition(t.variableName)], class: "tok-definition" },
  { tag: [t.invalid], class: "tok-invalid" },
]);

// The cpp language's parser carries its highlighting props.
const parser = cppLanguage.parser ?? cppParser;

function escape(s: string) {
  return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

const cppLangs = new Set(["cpp", "c++", "cc", "c", "h", "hpp", "cxx"]);

export function isCpp(lang: string) {
  return cppLangs.has(lang.toLowerCase());
}

// highlightCpp returns HTML for a C++ snippet.
export function highlightCpp(code: string): string {
  let html = "";
  const tree = parser.parse(code);
  highlightCode(
    code,
    tree,
    highlighter,
    (text, classes) => {
      html += classes ? `<span class="${classes}">${escape(text)}</span>` : escape(text);
    },
    () => {
      html += "\n";
    },
  );
  return html;
}

export { escape };

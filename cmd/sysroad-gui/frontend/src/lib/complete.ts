import type { Completion, CompletionContext, CompletionResult } from "@codemirror/autocomplete";
import { syntaxTree } from "@codemirror/language";
import type { EditorState } from "@codemirror/state";

// Completion without a language server (clangd comes in G3): names already
// in the lesson's files, classified from the syntax tree, plus C++ keywords
// and the standard library names the course uses. Like a buffer source in
// blink.cmp: instant, no project knowledge.

type Kind = "variable" | "function" | "type" | "namespace" | "keyword" | "property";

const keywords =
  "alignas alignof auto bool break case catch char char8_t class concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept nullptr operator override private protected public register reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while final";

const stdTypes =
  "std string string_view vector array map unordered_map set unordered_set pair tuple optional variant span unique_ptr shared_ptr weak_ptr size_t ptrdiff_t uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t byte istream ostream iostream ifstream ofstream fstream stringstream istringstream ostringstream deque list queue stack priority_queue function thread mutex atomic chrono filesystem error_code exception runtime_error invalid_argument out_of_range";

const stdFunctions =
  "cout cin cerr clog endl getline move forward swap make_unique make_shared make_pair size begin end min max sort find find_if count count_if accumulate transform copy fill reverse to_string stoi stol stoul stod printf puts fputs fprintf fopen fclose fread fwrite fgets malloc calloc realloc free memcpy memmove memset memcmp strlen strcmp strncmp strchr strcpy exit abort assert push_back emplace_back pop_back insert erase clear empty resize reserve substr front back data at c_str append starts_with ends_with contains";

const builtins: Completion[] = [
  ...keywords.split(" ").map((label) => ({ label, type: "keyword" as Kind, boost: -2 })),
  ...stdTypes.split(" ").map((label) => ({ label, type: "type" as Kind, boost: -1 })),
  ...stdFunctions.split(" ").map((label) => ({ label, type: "function" as Kind, boost: -1 })),
];

// Names in one document, by kind. Cached per document, so typing only
// rescans the file that changed.
const cache = new WeakMap<object, Map<string, Kind>>();

function namesIn(state: EditorState): Map<string, Kind> {
  const doc = state.doc;
  let names = cache.get(doc);
  if (names) return names;
  names = new Map();
  const tree = syntaxTree(state);
  tree.iterate({
    enter(node) {
      let kind: Kind | null = null;
      switch (node.name) {
        case "TypeIdentifier":
          kind = "type";
          break;
        case "NamespaceIdentifier":
          kind = "namespace";
          break;
        case "FieldIdentifier":
          kind = "property";
          break;
        case "Identifier": {
          const p = node.node.parent?.name;
          kind = p === "FunctionDeclarator" || p === "CallExpression" ? "function" : "variable";
          break;
        }
      }
      if (!kind) return;
      const text = doc.sliceString(node.from, node.to);
      if (text.length < 2) return;
      const had = names!.get(text);
      // A name seen as a function or type anywhere is that, not a variable.
      if (!had || had === "variable") names!.set(text, kind);
    },
  });
  cache.set(doc, names);
  return names;
}

// lessonCompletions completes from the open file and the lesson's other
// files (others() returns their current states).
export function lessonCompletions(others: () => EditorState[]) {
  return (ctx: CompletionContext): CompletionResult | null => {
    const word = ctx.matchBefore(/[A-Za-z_][A-Za-z0-9_]*/);
    if (!word || (word.from === word.to && !ctx.explicit)) return null;
    // Not inside comments or strings.
    const node = syntaxTree(ctx.state).resolveInner(ctx.pos, -1);
    if (/Comment|String|Char/.test(node.name)) return null;
    const typed = word.text;
    const seen = new Map<string, Kind>();
    for (const st of [ctx.state, ...others()]) {
      for (const [name, kind] of namesIn(st)) {
        if (name === typed) continue; // the word being typed
        if (!seen.has(name) || seen.get(name) === "variable") seen.set(name, kind);
      }
    }
    // Your own names win over a built-in with the same spelling (a local
    // count is a variable, not std::count).
    const options: Completion[] = [];
    for (const [label, type] of seen) options.push({ label, type });
    for (const b of builtins) if (!seen.has(b.label)) options.push(b);
    return { from: word.from, options, validFor: /^[A-Za-z0-9_]*$/ };
  };
}

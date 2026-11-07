import type { Completion, CompletionContext, CompletionResult } from "@codemirror/autocomplete";
import { syntaxTree } from "@codemirror/language";
import type { EditorState } from "@codemirror/state";
import type { SyntaxNode } from "@lezer/common";

// Completion without a language server (clangd comes in G3). Like a buffer
// source in blink.cmp, but C++-aware enough to be useful:
//
// - after `x.` or `x->` it offers members: the fields of your own structs,
//   or the methods of the std type x was declared with;
// - otherwise names from the lesson's files (classified by the syntax tree),
//   C++ keywords and common std names.

type Kind = "variable" | "function" | "type" | "namespace" | "keyword" | "property" | "method";

const keywords =
  "alignas alignof auto bool break case catch char char8_t class concept const consteval constexpr constinit const_cast continue co_await co_return co_yield decltype default delete do double dynamic_cast else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept nullptr operator override private protected public register reinterpret_cast requires return short signed sizeof static static_assert static_cast struct switch template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while final";

const stdTypes =
  "std string string_view vector array map unordered_map set unordered_set pair tuple optional variant span unique_ptr shared_ptr weak_ptr size_t ptrdiff_t uint8_t uint16_t uint32_t uint64_t int8_t int16_t int32_t int64_t byte istream ostream iostream ifstream ofstream fstream stringstream istringstream ostringstream deque list queue stack priority_queue function thread mutex atomic chrono filesystem error_code exception runtime_error invalid_argument out_of_range";

const stdFunctions =
  "cout cin cerr clog endl getline move forward swap make_unique make_shared make_pair size begin end min max sort find find_if count count_if accumulate transform copy fill reverse to_string stoi stol stoul stod printf puts fputs fprintf fopen fclose fread fwrite fgets malloc calloc realloc free memcpy memmove memset memcmp strlen strcmp strncmp strchr strcpy exit abort assert isspace isalpha isdigit isalnum isupper islower toupper tolower";

const builtins: Completion[] = [
  ...keywords.split(" ").map((label) => ({ label, type: "keyword" as Kind, boost: -2 })),
  ...stdTypes.split(" ").map((label) => ({ label, type: "type" as Kind, boost: -1 })),
  ...stdFunctions.split(" ").map((label) => ({ label, type: "function" as Kind, boost: -1 })),
];

// Members of the std types the course uses, by type name.
const seq = "size empty clear begin end cbegin cend rbegin rend front back swap";
const vec = `${seq} push_back emplace_back pop_back insert emplace erase resize reserve capacity shrink_to_fit data at assign`;
const str = `${vec} length c_str substr find rfind find_first_of find_last_of find_first_not_of find_last_not_of append compare starts_with ends_with contains replace npos`;
const assoc = `${seq.replace(" front back", "")} insert emplace try_emplace insert_or_assign erase find count contains at lower_bound upper_bound equal_range extract merge`;
const stream = "good eof fail bad clear get getline read gcount peek ignore unget putback write put flush tellg seekg tellp seekp rdbuf";
const members: Record<string, string> = {
  vector: vec,
  deque: `${vec} push_front emplace_front pop_front`,
  list: `${seq} push_back push_front pop_back pop_front emplace_back emplace_front insert erase remove remove_if sort unique reverse splice merge`,
  array: "size empty begin end front back data at fill swap",
  string: str,
  string_view: "size length empty begin end front back data at substr find rfind find_first_of find_last_of starts_with ends_with contains compare remove_prefix remove_suffix copy npos",
  map: assoc,
  unordered_map: `${assoc} bucket_count load_factor rehash reserve`,
  set: assoc,
  unordered_set: `${assoc} bucket_count load_factor rehash reserve`,
  pair: "first second swap",
  optional: "has_value value value_or reset emplace swap",
  unique_ptr: "get reset release swap",
  shared_ptr: "get reset swap use_count",
  span: "size size_bytes empty begin end front back data first last subspan",
  istream: stream,
  ostream: stream,
  iostream: stream,
  ifstream: `${stream} open close is_open`,
  ofstream: `${stream} open close is_open`,
  fstream: `${stream} open close is_open`,
  stringstream: `${stream} str`,
  istringstream: `${stream} str`,
  ostringstream: `${stream} str`,
};
const anyMember = [...new Set(Object.values(members).join(" ").split(" "))];

// ---- what's in a document (cached per document) ----

type Names = {
  names: Map<string, { kind: Kind; count: number }>;
  structs: Map<string, Map<string, Kind>>; // struct/class name → its members
};

const cache = new WeakMap<object, Names>();

function namesIn(state: EditorState): Names {
  const doc = state.doc;
  const hit = cache.get(doc);
  if (hit) return hit;
  const out: Names = { names: new Map(), structs: new Map() };
  const text = (n: SyntaxNode) => doc.sliceString(n.from, n.to);
  syntaxTree(state).iterate({
    enter(ref) {
      const node = ref.node;
      if (node.name === "StructSpecifier" || node.name === "ClassSpecifier") {
        const name = node.getChild("TypeIdentifier");
        const body = node.getChild("FieldDeclarationList");
        if (name && body) {
          const fields = new Map<string, Kind>();
          body.cursor().iterate((c) => {
            if (c.name === "FieldIdentifier") {
              const method = c.node.parent?.name === "FunctionDeclarator";
              fields.set(text(c.node), method ? "method" : "property");
            }
          });
          out.structs.set(text(name), fields);
        }
      }
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
          const p = node.parent?.name;
          kind = p === "FunctionDeclarator" || p === "CallExpression" ? "function" : "variable";
          break;
        }
      }
      if (!kind) return;
      const name = text(node);
      if (name.length < 2) return;
      const had = out.names.get(name);
      // A name seen as a function or type anywhere is that, not a variable.
      if (!had) out.names.set(name, { kind, count: 1 });
      else {
        had.count++;
        if (had.kind === "variable") had.kind = kind;
      }
    },
  });
  cache.set(doc, out);
  return out;
}

// typeOf finds the type a variable was declared with, from the text before
// the cursor: `std::vector<int> v`, `const std::string& s`, `Counts c`.
function typeOf(src: string, name: string): string | null {
  const re = new RegExp(`(?:\\bstd::)?\\b([A-Za-z_]\\w*)\\s*(?:<[^;{}()]*>)?\\s*(?:const\\s*)?[&*]*\\s*\\b${name}\\b\\s*(?=[;,=({)\\[]|$)`, "g");
  let found: string | null = null;
  for (let m; (m = re.exec(src)); ) {
    if (!/^(return|else|delete|new|throw|case|goto)$/.test(m[1])) found = m[1];
  }
  return found;
}

// ---- the completion source ----

// lessonCompletions completes from the open file and the lesson's other
// files (others() returns their current states).
export function lessonCompletions(others: () => EditorState[]) {
  return (ctx: CompletionContext): CompletionResult | null => {
    const word = ctx.matchBefore(/[A-Za-z_][A-Za-z0-9_]*/);
    const from = word ? word.from : ctx.pos;
    const typed = word ? word.text : "";
    const before = ctx.state.sliceDoc(Math.max(0, from - 200), from);
    const member = /([A-Za-z_]\w*)\s*(?:\.|->)\s*$/.exec(before);
    if (!word && !member && !ctx.explicit) return null;
    // Not inside comments or strings.
    const node = syntaxTree(ctx.state).resolveInner(ctx.pos, -1);
    if (/Comment|String|Char/.test(node.name)) return null;

    const docs = [ctx.state, ...others()].map(namesIn);
    // Skip the word being typed itself, unless it's also used elsewhere.
    const isTyped = (name: string, count: number) => name === typed && count <= 1;

    if (member) {
      const options: Completion[] = [];
      const seen = new Set<string>();
      const add = (label: string, type: Kind, boost: number) => {
        if (seen.has(label) || label === typed) return;
        seen.add(label);
        options.push({ label, type, boost });
      };
      const type = typeOf(ctx.state.sliceDoc(0, from), member[1]);
      let known = false;
      if (type) {
        for (const d of docs) {
          for (const [f, k] of d.structs.get(type) ?? []) {
            add(f, k, 3);
            known = true;
          }
        }
        for (const m of members[type]?.split(" ") ?? []) {
          add(m, m === "npos" ? "property" : "method", 3);
          known = true;
        }
      }
      // A type we know lists only its own members; otherwise offer anything
      // used as a member in the lesson, then the common std methods.
      if (!known) {
        for (const d of docs) for (const [n, v] of d.names) if (v.kind === "property" && !isTyped(n, v.count)) add(n, "property", 0);
        for (const m of anyMember) add(m, "method", -1);
      }
      return { from, options };
    }

    const merged = new Map<string, Kind>();
    for (const d of docs) {
      for (const [name, v] of d.names) {
        if (isTyped(name, v.count) || v.kind === "property") continue;
        if (!merged.has(name) || merged.get(name) === "variable") merged.set(name, v.kind);
      }
    }
    // Your own names win over a built-in with the same spelling (a local
    // count is a variable, not std::count).
    const options: Completion[] = [];
    for (const [label, type] of merged) options.push({ label, type });
    for (const b of builtins) if (!merged.has(b.label)) options.push(b);
    return { from, options };
  };
}

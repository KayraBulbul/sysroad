`std::vector<std::string_view> files(args.begin() + 1, args.end());`
copies everything after the pattern. If it's empty, push `"-"`.
---
Keep two flags, `bool matched = false, error = false;`. After the loop:
`if (error) return 2; return matched ? 0 : 1;`.
---
The prefix for a file is `names ? std::string(name) + ":" : ""`, where
`bool names = files.size() > 1;`.

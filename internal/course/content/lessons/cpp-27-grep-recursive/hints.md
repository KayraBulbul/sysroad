Shape: collect the paths first, sort, then search each. A helper lambda
`search_file(const fs::path& p, const std::string& shown)` that opens,
reads, checks for binary and calls `grep_stream` keeps the loop short.
---
Use `recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)`,
so one unreadable folder doesn't stop the search.
---
For a binary file: `std::replace(data.begin(), data.end(), '\0', '\n');`,
then run `grep_stream` into a throwaway `std::ostringstream` to see
whether anything matched.

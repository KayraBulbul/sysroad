The `const` goes after the parameter list, in **both** places:
`int count(const std::string& name) const;` in the class, and
`int Inventory::count(const std::string& name) const { ... }` in the
`.cpp`. If they don't match, they're two different functions.
---
```cpp
auto it = items_.find(name);
return it == items_.end() ? 0 : it->second;
```
`it->second` is the value. A map's elements are `std::pair<const std::string, int>`.
---
`add` is the one member that stays non-const, since it changes the map.

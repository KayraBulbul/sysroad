`get` tries the cache first:

```cpp
if (auto it = entries_.find(name); it != entries_.end()) {
    if (auto alive = it->second.lock()) return alive;
}
auto fresh = std::make_shared<const std::string>(load_(name));
++loads_;
entries_[name] = fresh;     // a weak_ptr made from the shared_ptr
return fresh;
```
---
`in_use` with C++20's `std::erase_if` for maps:
`std::erase_if(entries_, [](const auto& kv) { return kv.second.expired(); });`
then return `entries_.size()`.
---
In `family.hpp`, change `std::shared_ptr<Person> parent` to
`std::weak_ptr<Person> parent`. Then `adopt` is
`parent->children.push_back(child); child->parent = parent;`.
---
`lineage` walks up with `lock()`:

```cpp
std::string out = p.name;
for (auto up = p.parent.lock(); up; up = up->parent.lock())
    out += " < " + up->name;
return out;
```

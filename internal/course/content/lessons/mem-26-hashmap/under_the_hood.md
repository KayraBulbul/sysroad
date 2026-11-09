**Clustering.** Linear probing's weakness is that collisions build runs
of full slots, and anything that hashes into a run has to walk to its
end. Runs merge and grow. At load 3/4, an unsuccessful lookup probes
about 8 slots on average, and at 0.9 it's about 50. That's why the load
limit matters. *Robin Hood hashing* (move entries that are closer to
home out of the way) and *quadratic probing* both spread things out.

**Bad hashes hurt more.** libstdc++'s `std::hash<int>` is the identity
function, so keys 0, 8, 16, 24... all land in slot 0 of an 8-slot
table. With the `&` mask, only the low bits of the hash count. Serious
tables mix the hash first, by multiplying by a large odd constant and
taking the high bits ("Fibonacci hashing").

**Swiss tables.** Google's `absl::flat_hash_map` and Rust's `HashMap`
store one byte of metadata per slot (7 bits of the hash plus
empty/deleted) in a separate array, and compare 16 of them at once with
SIMD instructions. It's still open addressing, just much faster.

**Why not `std::vector<Slot>` with `std::optional`?** You could. This
`Slot` requires `K` and `V` to be default-constructible, which keeps
the code short. Real tables use raw storage and placement new, like
your `Vector`, so empty slots hold no objects at all.

**Hash flooding.** If attackers can choose your keys (HTTP headers, JSON
fields), they can pick keys that all collide and turn every lookup into
a scan. That's a real denial-of-service attack. The defence is a hash
with a random secret seed, like SipHash, which Python and Rust use by
default.

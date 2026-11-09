**Allocators.** `std::vector<T, Alloc>` gets its raw memory from an
allocator object rather than calling `::operator new` itself. That's
how a vector can live in shared memory, in an arena, or in a pool you
write in phase 7. `std::allocator_traits` wraps construct/destroy so
custom allocators can hook those steps too.

**Exception safety.** If a copy throws halfway through growing, the
real `std::vector` destroys the copies it already made, frees the new
buffer, and leaves the original untouched. The strong guarantee from
2.3.5. Yours can skip that, but it's why `std::uninitialized_copy` and
`std::uninitialized_move` exist: they clean up after themselves when a
constructor throws.

**Trivially relocatable types.** For many types (ints, `unique_ptr`,
most strings) moving then destroying is the same as `memcpy`ing the
bytes. Libraries like Folly and Qt detect these and move a whole buffer
with one `memcpy`. C++26 is standardising the idea as "trivially
relocatable".

**`std::launder` and friends.** Reusing storage with placement new has
some sharp corners in the object model (pointers to the old object
don't automatically refer to the new one). For a vector that only uses
`data_[i]` freshly each time, it doesn't come up.

**Why `size_` and `cap_` instead of three pointers?** libstdc++ stores
`begin`, `end` and `end_of_storage` pointers. `size()` is then a
subtraction, and `push_back` compares two pointers. It's the same
information, and either layout makes the type 24 bytes.

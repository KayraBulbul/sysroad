**Reading your mountain.** On an Intel i9-14900K the
stride-1 column sits around 75 GB/s up to 32 KB, drops to about 68 up
to 2 MB, to under 60 from 4 MB to 16 MB, and to about 25 past the 36
MB L3. Those steps are the L1, L2, L3 and main memory. The drop down
each row, from s1 to s8, is spatial locality running out: by s8,
every read is a new line. Past s8 it stops falling, because a miss
can't cost more than one line per read.

**Why s1 stays high even in main memory.** The prefetcher. A stride-1
loop is the easiest pattern there is to predict, so the hardware
fetches lines ahead of the loop and many of its misses never stall it.
Prefetchers handle a fixed stride too, but the bigger the stride, the
less a prefetched line has in it for you.

**Your Mac's mountain looks different.** Apple Silicon has a 128 KB
L1 on the performance cores, a 12 MB or larger L2 shared by a cluster,
a system-level cache in front of memory, and 128-byte lines, so the
slope keeps falling until s16. Unified memory gives it unusually high
bandwidth for a laptop, especially with several cores reading at
once. This program uses one core.

**One core can't saturate memory.** A single core can only have so
many misses in flight (10 to 20 *line fill buffers* on recent x86).
At 80 ns each, that caps one core's bandwidth well below what the
memory can deliver. Servers with dozens of cores run into the opposite
problem: they share the bandwidth, so a loop that scales perfectly on
a laptop stops scaling at 8 threads.

**Writes cost more than reads.** Writing to a line you don't have
means reading it in first (a *read for ownership*), then writing it
back later. A copy loop therefore moves three lines of traffic for
every two you asked for. `memset` and `memcpy` use special
"non-temporal" stores for big buffers that skip that read.

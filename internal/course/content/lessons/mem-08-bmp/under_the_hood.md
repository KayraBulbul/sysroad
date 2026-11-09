**Why bottom-up?** BMP comes from OS/2 and Windows, whose graphics
coordinates put (0, 0) at the bottom-left, as in maths. The negative
height for top-down was added later and is rarer.

**Why pad rows to 4 bytes?** So every row starts on a 4-byte boundary.
On 1990s hardware that made copying a row to the screen faster, and it's
the alignment lesson from earlier in this chapter showing up in a file
format.

**Bitfields.** Compression 3 means the header is followed by masks
saying which bits are red, green and blue. That allows formats like
RGB565 in 16 bits. For 32-bit images the masks are almost always the
standard ones, so this lesson reads them as B, G, R, A. A full decoder
would read the masks.

**Image decoders are classic attack surface.** Integer overflow in
`width * height * bytes_per_pixel` is a bug pattern behind many real
CVEs in image libraries: the product wraps to a small number, a small
buffer gets allocated, and the copy writes past it. That's why your
size maths runs in 64 bits and why the truncation check exists.

**Try it on a real image.** `magick photo.jpg -resize 8x8 small.bmp`
(ImageMagick) makes a BMP. Build your program with
`clang++ -std=c++20 *.cpp -o bmp` and point it at the file.

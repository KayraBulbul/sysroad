; The same six functions, compiled by clang -O1 for arm64 (your Mac).
;
; The destination comes first. Arguments arrive in x0, x1, x2, in that
; order. The result is whatever is in x0 at the ret. macOS puts an
; underscore in front of every function name.

_first:
	add	x0, x1, x0
	ret

_second:
	sub	x0, x0, x1, lsl #1
	ret

_third:
	add	x8, x0, x0, lsl #2
	add	x0, x8, #7
	ret

_fourth:
	sub	x8, x0, x1
	mul	x0, x8, x2
	ret

_fifth:
	add	w8, w0, w0, lsl #1
	sub	w8, w8, w1
	add	w0, w8, #100
	ret

_sixth:
	asr	x8, x1, #2
	add	x0, x8, x0, lsl #4
	ret

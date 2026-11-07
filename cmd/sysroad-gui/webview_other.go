//go:build gui && (!linux || server)

package main

import "unsafe"

// instantScroll is Linux-only: macOS scrolls natively, with the trackpad's
// momentum, and server builds have no window.
func instantScroll(unsafe.Pointer) int { return -1 }

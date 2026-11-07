//go:build gui && (!linux || server)

package main

import "unsafe"

// tuneWebView is Linux-only: macOS scrolls natively, with the trackpad's
// momentum, and passes the side buttons to the page itself; server builds
// have no window.
func tuneWebView(unsafe.Pointer) int { return -1 }

var mouseNav = func(int) {}

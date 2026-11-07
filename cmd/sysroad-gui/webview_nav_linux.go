//go:build gui && linux && !server

package main

import "C"

// mouseNav is called with -1 (back) or +1 (forward) when a mouse side button
// is pressed; main sets it.
var mouseNav = func(int) {}

//export sysroadMouseNav
func sysroadMouseNav(dir C.int) {
	go mouseNav(int(dir))
}

//go:build gui && (!darwin || server)

package main

import "github.com/wailsapp/wails/v3/pkg/application"

// fitTitleBar is macOS-only: elsewhere the window has its own title bar.
func fitTitleBar(*application.WebviewWindow) {}

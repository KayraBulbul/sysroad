//go:build gui && darwin && !server

package main

/*
#cgo CFLAGS: -x objective-c
#cgo LDFLAGS: -framework Cocoa
#import <Cocoa/Cocoa.h>

// Where macOS put the traffic lights: the vertical centre of the close button
// and the right edge of the zoom button, in points from the window's top left.
static int traffic_lights(void *w, double *centre, double *right) {
	NSWindow *win = (NSWindow *)w;
	NSButton *close = [win standardWindowButton:NSWindowCloseButton];
	NSButton *zoom = [win standardWindowButton:NSWindowZoomButton];
	if (close == nil || zoom == nil) return 0;
	NSRect c = [close convertRect:close.bounds toView:nil];
	NSRect z = [zoom convertRect:zoom.bounds toView:nil];
	*centre = win.frame.size.height - NSMidY(c);
	*right = NSMaxX(z);
	return 1;
}
*/
import "C"

import (
	"fmt"

	"github.com/wailsapp/wails/v3/pkg/application"
	"github.com/wailsapp/wails/v3/pkg/events"
)

// fitTitleBar lines the top bar up with the traffic lights. Their place
// depends on the macOS version and toolbar style, so the page gets the real
// numbers instead of guessing: the bar is as tall as twice the lights' centre,
// which centres them in it, and its content starts after the zoom button.
// In full screen the lights leave the window, and the bar goes back to normal.
func fitTitleBar(win *application.WebviewWindow) {
	var centre, right C.double
	application.InvokeSync(func() {
		if C.traffic_lights(win.NativeWindow(), &centre, &right) == 0 {
			centre = 0
		}
	})
	if centre <= 0 {
		return
	}
	win.ExecJS(fmt.Sprintf(
		`document.documentElement.style.setProperty("--lights-y", "%.1fpx");`+
			`document.documentElement.style.setProperty("--lights-x", "%.1fpx");`,
		float64(centre), float64(right)))

	fullscreen := func(on bool) func(*application.WindowEvent) {
		return func(*application.WindowEvent) {
			win.ExecJS(fmt.Sprintf(`document.documentElement.classList.toggle("fullscreen", %t);`, on))
		}
	}
	win.OnWindowEvent(events.Mac.WindowDidEnterFullScreen, fullscreen(true))
	win.OnWindowEvent(events.Mac.WindowDidExitFullScreen, fullscreen(false))
}

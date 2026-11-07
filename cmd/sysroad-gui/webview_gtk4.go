//go:build gui && linux && !gtk3 && !server

package main

/*
#cgo pkg-config: gtk4 webkitgtk-6.0
#include <gtk/gtk.h>
#include <webkit/webkit.h>

// Turn off WebKitGTK's animated wheel scrolling on every web view under w.
static int instant_scroll(GtkWidget *w) {
	if (WEBKIT_IS_WEB_VIEW(w)) {
		WebKitSettings *s = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(w));
		webkit_settings_set_enable_smooth_scrolling(s, FALSE);
		return 1;
	}
	int n = 0;
	for (GtkWidget *c = gtk_widget_get_first_child(w); c; c = gtk_widget_get_next_sibling(c))
		n += instant_scroll(c);
	return n;
}
*/
import "C"

import "unsafe"

// instantScroll disables smooth scrolling in the window's web view, so the
// page follows the wheel exactly. Run it on the GTK main thread.
func instantScroll(window unsafe.Pointer) int {
	if window == nil {
		return 0
	}
	return int(C.instant_scroll((*C.GtkWidget)(window)))
}

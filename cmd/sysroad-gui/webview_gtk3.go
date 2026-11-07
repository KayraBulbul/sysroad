//go:build gui && linux && gtk3 && !server

package main

/*
#cgo pkg-config: gtk+-3.0 webkit2gtk-4.1
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

// Turn off WebKitGTK's animated wheel scrolling on every web view under w.
static int instant_scroll(GtkWidget *w) {
	if (WEBKIT_IS_WEB_VIEW(w)) {
		WebKitSettings *s = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(w));
		webkit_settings_set_enable_smooth_scrolling(s, FALSE);
		return 1;
	}
	int n = 0;
	if (GTK_IS_CONTAINER(w)) {
		GList *kids = gtk_container_get_children(GTK_CONTAINER(w));
		for (GList *l = kids; l; l = l->next) n += instant_scroll(GTK_WIDGET(l->data));
		g_list_free(kids);
	}
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

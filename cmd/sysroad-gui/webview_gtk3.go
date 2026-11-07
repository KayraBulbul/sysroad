//go:build gui && linux && gtk3 && !server

package main

/*
#cgo pkg-config: gtk+-3.0 webkit2gtk-4.1
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

extern void sysroadMouseNav(int dir);

// WebKitGTK doesn't pass the mouse's back/forward buttons (8 and 9) to the
// page, so catch them on the widget.
static gboolean side_buttons(GtkWidget *w, GdkEventButton *e, gpointer data) {
	if (e->button != 8 && e->button != 9) return FALSE;
	if (e->type == GDK_BUTTON_PRESS) sysroadMouseNav(e->button == 8 ? -1 : 1);
	return TRUE;
}

// Set up every web view under w: no animated wheel scrolling, so the page
// follows the wheel exactly, and the side buttons for back/forward.
static int tune_webviews(GtkWidget *w) {
	if (WEBKIT_IS_WEB_VIEW(w)) {
		WebKitSettings *s = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(w));
		webkit_settings_set_enable_smooth_scrolling(s, FALSE);
		g_signal_connect(w, "button-press-event", G_CALLBACK(side_buttons), NULL);
		g_signal_connect(w, "button-release-event", G_CALLBACK(side_buttons), NULL);
		return 1;
	}
	int n = 0;
	if (GTK_IS_CONTAINER(w)) {
		GList *kids = gtk_container_get_children(GTK_CONTAINER(w));
		for (GList *l = kids; l; l = l->next) n += tune_webviews(GTK_WIDGET(l->data));
		g_list_free(kids);
	}
	return n;
}
*/
import "C"

import "unsafe"

// tuneWebView sets up the window's web view (see tune_webviews) and returns
// how many it found. Run it on the GTK main thread.
func tuneWebView(window unsafe.Pointer) int {
	if window == nil {
		return 0
	}
	return int(C.tune_webviews((*C.GtkWidget)(window)))
}

//go:build gui && linux && !gtk3 && !server

package main

/*
#cgo pkg-config: gtk4 webkitgtk-6.0
#include <gtk/gtk.h>
#include <webkit/webkit.h>

extern void sysroadMouseNav(int dir);

// WebKitGTK doesn't pass the mouse's back/forward buttons (8 and 9) to the
// page, so catch them with a capture-phase click gesture.
static void side_buttons(GtkGestureClick *g, int n, double x, double y, gpointer data) {
	guint b = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(g));
	if (b != 8 && b != 9) return;
	gtk_gesture_set_state(GTK_GESTURE(g), GTK_EVENT_SEQUENCE_CLAIMED);
	sysroadMouseNav(b == 8 ? -1 : 1);
}

// Set up every web view under w: no animated wheel scrolling, so the page
// follows the wheel exactly, and the side buttons for back/forward.
static int tune_webviews(GtkWidget *w) {
	if (WEBKIT_IS_WEB_VIEW(w)) {
		WebKitSettings *s = webkit_web_view_get_settings(WEBKIT_WEB_VIEW(w));
		webkit_settings_set_enable_smooth_scrolling(s, FALSE);
		GtkGesture *g = gtk_gesture_click_new();
		gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(g), 0);
		gtk_event_controller_set_propagation_phase(GTK_EVENT_CONTROLLER(g), GTK_PHASE_CAPTURE);
		g_signal_connect(g, "pressed", G_CALLBACK(side_buttons), NULL);
		gtk_widget_add_controller(w, GTK_EVENT_CONTROLLER(g));
		return 1;
	}
	int n = 0;
	for (GtkWidget *c = gtk_widget_get_first_child(w); c; c = gtk_widget_get_next_sibling(c))
		n += tune_webviews(c);
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

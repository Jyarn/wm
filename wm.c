#include <X11/Xlib.h>
#include <assert.h>
#include "wm.h"
#include "event.h"
#include <stdio.h>

Display* dpy;
screen defaultScreen;





void wm_grabInputs (void);
void wm_start (void);




/*
 * Grab keys (keyboard/mouse) needed for input (user control)
*/
void wm_grabInputs (void) {
	// ないよ
}

/*
 * Initialize default screen, and set the event mask for the event mask
*/
void wm_start (void) {
	// open connection to x
	dpy = XOpenDisplay (NULL);

	// init default screen
	defaultScreen.screen = DefaultScreen (dpy);
	defaultScreen.root = RootWindow (dpy, defaultScreen.screen);
	defaultScreen.w = DisplayWidth (dpy, defaultScreen.screen);
	defaultScreen.h = DisplayHeight (dpy, defaultScreen.screen);

	// configure event masks
	XSetWindowAttributes winAttrib;
	winAttrib.event_mask = SubstructureRedirectMask;
	XSelectInput (dpy, defaultScreen.root, winAttrib.event_mask);
}



int main (int argc, char** argv) {
	wm_start ();
	evt_eventHandler ();
	return 1;
}
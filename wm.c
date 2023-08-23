#include <X11/Xlib.h>
#include <assert.h>
#include "wm.h"
#include "event.h"
#include "config.h"
#include "util.h"

Display* dpy;
screen defaultScreen;

void wm_grabInputs (void);
void wm_grabKeys (Window win, int sync);
void wm_start (void);




/*
 * Grab keys (keyboard/mouse) needed for input (user control)
*/

/*
 * Grab keys specified in keyBinds (config.c)
*/
void wm_grabKeys (Window win, int sync) {
	for (int i = 0; i < N_KEY_BINDS; i++) {
		XGrabKey (dpy,
				  STR_TO_KEYSYM (keyBinds[i].key),
				  keyBinds[i].modifier,
				  win,
				  False,
				  GrabModeAsync,
				  sync);
	}
}

void wm_grabInputs (void) {
	XGrabButton (dpy, AnyButton, Mod4Mask, defaultScreen.root, True, ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
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
	winAttrib.event_mask = SubstructureRedirectMask | KeyPressMask;
	XSelectInput (dpy, defaultScreen.root, winAttrib.event_mask);
}



int main (int argc, char** argv) {
	wm_start ();
	wm_grabInputs ();
	wm_grabKeys (defaultScreen.root, GrabModeAsync);
	evt_eventHandler ();
	return 1;
}
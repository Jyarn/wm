#include <X11/Xlib.h>
#include <assert.h>
#include <stdlib.h>

#include "wm.h"
#include "event.h"
#include "config.h"
#include "util.h"

Display* dpy;
screen defaultScreen;
static client* activeClients;

void wm_grabMouse (Window win, int sync);
void wm_grabKeys (Window win, int sync);
void wm_start (void);
void wm_cleanup (void);




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
				  GrabModeAsync, sync);
	}
}

/*
 * Grab mouse (config.c)
*/
void wm_grabMouse (Window win, int sync) {
	XGrabButton (dpy,
	AnyButton,
	Mod4Mask,
	win,
	True,
	ButtonPressMask | ButtonReleaseMask,
	sync, GrabModeAsync,
	None, None);
}

/*
 * Initialize default screen, and set the event mask for the event mask
*/
void wm_start (void) {
	activeClients = NULL;

	// open connection to x
	dpy = XOpenDisplay (NULL);

	// init default screen
	defaultScreen.screen = DefaultScreen (dpy);
	defaultScreen.root = RootWindow (dpy, defaultScreen.screen);
	defaultScreen.w = DisplayWidth (dpy, defaultScreen.screen);
	defaultScreen.h = DisplayHeight (dpy, defaultScreen.screen);

	// configure event masks
	XSetWindowAttributes winAttrib;
	winAttrib.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask;
	XSelectInput (dpy, defaultScreen.root, winAttrib.event_mask);
}

void wm_cleanup (void) {
	// free active clients
	client* cur = activeClients;
	client* prev = cur;

	while (cur) {
		prev = cur;
		cur = cur->next;
		free (prev);
	}

	free (cur);
	XCloseDisplay (dpy);
}

bool wm_shouldbeManaged (Window w) {
	XWindowAttributes attrib;
	if (!XGetWindowAttributes (dpy, w, &attrib) || attrib.override_redirect)
		return false;
	return true;
}

void wm_manage (Window w) {
	// attach new Client to list
	client* newClient = malloc (sizeof (client));
	newClient->next = activeClients;
	activeClients = newClient;

	// init new client
	newClient->window = w;
}

void wm_unmanage (Window w) {
	if (activeClients == NULL)
		return;

	client* cur = activeClients->next;
	client* prev = activeClients;

	do {
		if (cur->window == w) {
			// remove client from list
			prev->next = cur->next;
			free (cur);
			return;
		}

		prev = cur;
		cur = cur->next;
	} while (cur != NULL);
}

bool wm_killClient (Window w) {
	XGrabServer (dpy);
	XSetCloseDownMode (dpy, DestroyAll);
	XKillClient (dpy, w);
	XUngrabServer (dpy);
	XSync (dpy, false);
	return true; // return true so it can be used as key bind
}

int main (int argc UNUSED, char** argv UNUSED) {
#ifdef __DEBUG__
	volatile int a = 1;
	while (a) { }
#endif

	wm_start ();
	wm_grabMouse (defaultScreen.root, GrabModeAsync);
	wm_grabKeys (defaultScreen.root, GrabModeAsync);
	evt_eventHandler ();
	wm_cleanup ();
	return 1;
}
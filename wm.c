#include <X11/Xlib.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "wm.h"
#include "event.h"
#include "config.h"
#include "util.h"
#include "debug.h"

Display* dpy;
screen defaultScreen;
FILE* wm_log;
client* wm_focus;

static client* activeClients;

void start_wm (void);
void cleanup (void);




/*
 * Initialize default screen, and set the event mask for the event mask
*/
void start_wm (void) {
	activeClients = NULL;

	// open connection to x1
	dpy = XOpenDisplay (NULL);

	// init default screen
	defaultScreen.screen = DefaultScreen (dpy);
	defaultScreen.root = RootWindow (dpy, defaultScreen.screen);
	defaultScreen.w = DisplayWidth (dpy, defaultScreen.screen);
	defaultScreen.h = DisplayHeight (dpy, defaultScreen.screen);

	// configure event masks
	XSetWindowAttributes winAttrib;
	winAttrib.event_mask = ROOT_MASK;
	XSelectInput (dpy, defaultScreen.root, winAttrib.event_mask);
}

/*
 * free active clients
*/
void cleanup (void) {
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

client* wm_fetchClient (Window w) {
	if (w == defaultScreen.root)
		return NULL;
	for (client* c = activeClients; c; c = c->next)
		if (w == c->window)
			return c;
	return NULL;
}

/*
 * Grab keys (keyboard/mouse) needed for input (user control)
*/

/*
 * Grab keys specified in keyBinds (config.c)
*/
void wm_grabKeys (Window win, int sync) {
	for (int i = 0; i < N_KEY_BINDS; i++)
		XGrabKey (dpy,
				  STR_TO_KEYSYM (keyBinds[i].key),
				  keyBinds[i].modifier,
				  win,
				  False,
				  GrabModeAsync, sync);
}

/*
 * Grab mouse (config.c)
*/
void wm_grabMouse (Window win, int sync) {
	XUngrabButton (dpy, AnyButton, AnyModifier, win);
	for (int i = 0; i < N_MOUSE_BINDS; i++)
		XGrabButton (
			dpy,
			mouseBinds[i].buttons,
			mouseBinds[i].modifier,
			win,
			True,
			MOUSE_MASK,
			sync, GrabModeAsync,
			None, None
		);
}

/*
 * Grab all mouse/key bindings for the ones in moveBinds
*/
void wm_grabPointerBinds (Window win, int sync) {
	for (int i = 0; i < N_MOVE_BINDS; i++)
		XGrabButton (
			dpy,
			moveBinds[i].buttons,
			moveBinds[i].modifier,
			win,
			True,
			MOUSE_MASK | ButtonReleaseMask,
			sync, GrabModeAsync,
			None, None
		);
}

/*
 * Ungrab grabbed things from a window
*/
void wm_ungrab (Window w) {
	/*
		* https://stackoverflow.com/questions/44025639/how-can-i-check-in-xlib-if-window-exists

		* Because X is asynchronous, there isn't a guarantee that the window would still exist
		* between the time that you got the ID and the time you sent an event to the window or
		* otherwise manipulated it. What you should do is send the event without checking, but
		* install an error handler to catch any BadWindow errors, which would indicate that the
		* window no longer exists. This scheme will work except on the [rare] occasion that the
		* original window has been destroyed and its ID reallocated to another window.

		* You can use this scheme to make a function which checks the validity of a window; you
		* can make this operation almost synchronous by calling XSync() after the request,
		* although there is still no guarantee that the window will exist after the result
		* (unless the sterver is grabbed). On the 　whole, catching the error rather than pre-checking
		* is preferable.

		* life is pain
	*/

	XGrabServer (dpy);
	dbg_log ("[ INFO ] wm_ungrab\n");
	dbg_handlerOff (); // ignore xlib funny buissness
	XSelectInput (dpy, w, NoEventMask);
	XUngrabButton (dpy, AnyButton, AnyModifier, w);
	XUngrabKey (dpy, AnyKey, AnyModifier, w);
	XUngrabServer (dpy);
	XSync (dpy, False);
	dbg_handlerOn ();
}

/*
 * check if w should be managed (if override_redirect == true)
*/
bool wm_shouldbeManaged (Window w) {
	XWindowAttributes attrib;
	if (!XGetWindowAttributes (dpy, w, &attrib) || attrib.override_redirect)
		return false;
	return true;
}

/*
 * attach w to active client list
*/
void wm_manage (Window w) {
	if (wm_fetchClient (w)) // check if window is unmanaged
		return;
	// attach new Client to list
	client* newClient = malloc (sizeof (client));
	newClient->next = activeClients;
	activeClients = newClient;

	// fetch geometry of window
	Window root;
	unsigned int borderWidth, depth;

	XGetGeometry (dpy, w, &root, &newClient->x, &newClient->y, &newClient->w, &newClient->h, &borderWidth, &depth);

	// init new client
	newClient->window = w;

	// set event mask
	XSelectInput (dpy, w, WIN_MASK);
}

/*
 * remove w from active client list
*/
void wm_unmanage (Window w) {
	client* cur = activeClients;
	client* prev = NULL;

	while (cur) {
		if (cur->window == w) {
			if (prev)
				prev->next = cur->next;
			else
				activeClients = cur->next;

			dbg_log ("[ INFO ] wm_unmanage\n");
			free (cur);
			return;
		}

		prev = cur;
		cur = cur->next;
	}
}

void wm_killClient (Window w) {
	if (w == (Window)defaultScreen.root)
		return;

	wm_unmanage (w);
	XSetCloseDownMode (dpy, DestroyAll);
	XKillClient (dpy, w);
}

void wm_setFocus (Window w) {
	XSetInputFocus (dpy, w, RevertToPointerRoot, CurrentTime);

	if (w == (Window)defaultScreen.root)
		return;

	wm_focus = wm_fetchClient (w);
}


int main (int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
#ifdef __DEBUG__
	if (strcmp (argv[i], "--debug") == 0) {
		int a = 1;
		while (a) {}
		dbg_handlerOn ();
	}
#endif
	}

	dbg_init ();
	dbg_log ("[ INFO ]	wm starting\n");

	start_wm ();
	wm_grabMouse (defaultScreen.root, GrabModeAsync);
	wm_grabKeys (defaultScreen.root, GrabModeAsync);
	evt_eventHandler ();
	cleanup ();
	dbg_log ("[ INFO ]	normal exit");
	dbg_close ();
	return 0;
}
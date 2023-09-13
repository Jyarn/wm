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

static client* activeClients;
static client* wm_focus;

void wm_grabMouse (Window win, int sync);
void wm_grabKeys (Window win, int sync);
void wm_start (void);
void wm_cleanup (void);
client* wm_fetchClient (Window w);




/*
 * Initialize default screen, and set the event mask for the event mask
*/
void wm_start (void) {
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
void wm_cleanup (void) {
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
		XGrabButton (dpy,
		mouseBinds[i].buttons,
		keyBinds[i].modifier,
		win,
		True,
		MOUSE_MASK,
		sync, GrabModeAsync,
		None, None);
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

	// init new client
	newClient->window = w;

	// set event mask
	//XSelectInput (dpy, w, ROOT_MASK);
	wm_grabMouse (w, GrabModeAsync);
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
			free (cur);
			return;
		}

		prev = cur;
		cur = cur->next;
	}
}

bool wm_killClient (void* args) {
	Window w = (args ? *(Window* )args : wm_focus->window);
	if (w == (Window)defaultScreen.root)
		return true;

	XGrabServer (dpy);
	XSetCloseDownMode (dpy, DestroyAll);
	XKillClient (dpy, w);
	XUngrabServer (dpy);
	XSync (dpy, false);
	return true; // return true so it can be used as key bind
}

void wm_setFocus (Window w) {
	XSetInputFocus (dpy, w, RevertToPointerRoot, CurrentTime);

	if (w == (Window)defaultScreen.root)
		return;

	client* cur = activeClients;

	while (cur) {
		if (cur->window == w) {
			wm_focus = cur;
			return;
		}
		cur = cur->next;
	}
}


int main (int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
#ifdef __DEBUG__
	if (strcmp (argv[i], "--debug") == 0) {
		int a = 1;
		while (a) {}
	}
#endif
	}

	dbg_init ();
	dbg_log ("[ INFO ]	wm starting\n");

	wm_start ();
	wm_grabMouse (defaultScreen.root, GrabModeAsync);
	wm_grabKeys (defaultScreen.root, GrabModeAsync);
	evt_eventHandler ();
	wm_cleanup ();
	dbg_log ("[ INFO ]	normal exit");
	dbg_close ();
	return 0;
}
#include <X11/Xlib.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <X11/extensions/Xrandr.h>
#include <sys/stat.h>

#include "wm.h"
#include "event.h"
#include "config.h"
#include "util.h"
#include "debug.h"
#include "tl.h"

Display* dpy;
wm_screen defaultScreen;
FILE* wm_log;
Client* wm_focus;
Client* activeClients;
unsigned int workspacenum;


void start_wm (void);
void cleanup (void);
void detectMonitors (void);
void processcmdargs (int argc, char** argv);

void
detectMonitors (void)
{
	XRRScreenResources* srec = XRRGetScreenResourcesCurrent (dpy, defaultScreen.root);
	for (int i = 0; i < srec->noutput; i++) {
		XRROutputInfo* oi = XRRGetOutputInfo (dpy, srec, srec->outputs[i]);
		if (oi->connection == RR_Connected) {
			XRRCrtcInfo* cinfo = XRRGetCrtcInfo (dpy, srec, oi->crtc);
			dbg_log ("[ INFO ] %s: (x:%d,y:%d)\n", oi->name, cinfo->width, cinfo->height);
			XRRFreeCrtcInfo (cinfo);
		}

		XRRFreeOutputInfo (oi);
	}

	XRRFreeScreenResources (srec);
}

/*
 * Initialize default screen, and set the event mask for the event mask
*/
void
start_wm (void)
{
	activeClients = NULL;

	// open connection to x1
	dpy = XOpenDisplay (NULL);

	// init default screen
	defaultScreen.screen = DefaultScreen (dpy);
	defaultScreen.root = RootWindow (dpy, defaultScreen.screen);

	// configure event masks
	XSetWindowAttributes winAttrib;
	winAttrib.event_mask = ROOT_MASK;
	XSelectInput (dpy, defaultScreen.root, winAttrib.event_mask);
    workspacenum = 0;
    workspacenum = 1;
    workspacenum = 0;
	detectMonitors ();
}

/*
 * free active clients
*/
void
cleanup (void)
{
	Client* cur = activeClients;
	Client* prev = cur;

	while (cur) {
		prev = cur;
		cur = cur->next;
		free (prev);
	}

	free (cur);
	XCloseDisplay (dpy);
}

void
wm_changeGeomRelative (Client* cl, int relx, int rely, int relw, int relh)
{
	if (!cl)
		return;

	cl->x += relx;
	cl->y += rely;
	cl->w += relw;
	cl->h += relh;

	XMoveResizeWindow (dpy, cl->window, cl->x, cl->y, cl->w, cl->h);
}

Client*
wm_fetchClient (Window w)
{
	if (w == defaultScreen.root)
		return NULL;
	for (Client* c = activeClients; c; c = c->next)
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
void
wm_grabKeys (Window win)
{
	for (int i = 0; i < N_KEY_BINDS; i++)
		XGrabKey (dpy,
				  STR_TO_KEYSYM (keyBinds[i].key),
				  keyBinds[i].modifier,
				  win,
				  False,
				  GrabModeAsync, GrabModeAsync);
}

/*
 * Grab mouse (config.c)
*/
void
wm_grabMouse (Window win) {
	XUngrabButton (dpy, AnyButton, AnyModifier, win);
	for (int i = 0; i < N_MOUSE_BINDS; i++)
		XGrabButton (
			dpy,
			mouseBinds[i].buttons,
			mouseBinds[i].modifier,
			win,
			False,
			MOUSE_MASK,
			GrabModeAsync, GrabModeAsync,
			None, None
		);
}


/*
 * Ungrab grabbed things from a window
*/
void
wm_ungrab (Window w) {
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
bool
wm_shouldbeManaged (Window w) {
	XWindowAttributes attrib;
	if (w == (Window)defaultScreen.root)
		return false;
	if (wm_fetchClient (w))
		return false;
	if (!XGetWindowAttributes (dpy, w, &attrib) || attrib.override_redirect)
		return false;
	return true;
}

/*
 * attach w to active client list
*/
Client*
wm_manage (Window w) {
	if (wm_fetchClient (w)) // check if window is unmanaged
		return NULL;

	dbg_log ("[ INFO ] managing window %d\n", w);
	// attach new Client to list
	Client* newClient = malloc (sizeof (Client));
	newClient->next = activeClients;
	activeClients = newClient;
	newClient->minimized = false;

	// fetch geometry of window
	Window root;
	unsigned int borderWidth, depth, t_w, t_h;

	XGetGeometry (dpy, w, &root, &newClient->x, &newClient->y, &t_w, &t_h, &borderWidth, &depth);
	newClient->w = (int)t_w;
	newClient->h = (int)t_h;
	newClient->workspace = workspacenum;

	// init new client
	newClient->window = w;
	newClient->type = floating;

	// set event mask
	XSelectInput (dpy, w, WIN_MASK);
	XSetWindowBorderWidth (dpy, w, BORDERWIDTH);
	return newClient;
}

/*
 * remove w from active client list
*/
void
wm_unmanage (Window w) {
	Client* cur = activeClients;
	Client* prev = NULL;

	while (cur) {
		if (cur->window == w) {
			if (prev)
				prev->next = cur->next;
			else
				activeClients = cur->next;

			dbg_log ("[ INFO ] wm_unmanage\n");
			tl_remove (cur);
			free (cur);
			return;
		}

		prev = cur;
		cur = cur->next;
	}
}

void
wm_killClient (Window w) {
	if (w == (Window)defaultScreen.root)
		return;

	wm_focusNext (false);
	wm_unmanage (w);
	XSetCloseDownMode (dpy, DestroyAll);
	XKillClient (dpy, w);
}

void
wm_focusNext (bool focusMinimized) {
	if (!activeClients) {
		wm_setFocus (NULL);
		return;
	}
	else {
		Client* temp = (wm_focus != NULL) ? wm_focus->next : NULL;
		do {
			if (!temp)
				temp = activeClients;
			else if ((workspacenum == temp->workspace || temp->workspace == WORKSPACE_ALWAYSON) && (focusMinimized || !temp->minimized)) {
                XMoveWindow (dpy, temp->window, temp->x, temp->y);
                temp->minimized = false;
				wm_setFocus (temp);
				return;
			}
			else
				temp = temp->next;
		} while (temp != wm_focus);

		wm_setFocus (NULL);
	}
}

void
wm_setFocus (Client* cl) {
	if (cl == NULL) {
		dbg_log ("[ INFO ] wm_setfocus w = NULL\n");
		wm_focus = NULL;
		XSetInputFocus (dpy, defaultScreen.root, RevertToPointerRoot, CurrentTime);
		return;
	}
	dbg_log ("[ INFO ] wm_setfocus w = %d\n", cl->window);
	wm_focus = cl;
	XSetInputFocus (dpy, cl->window, RevertToPointerRoot, CurrentTime);
}

void
wm_show (Client* cl) {
    assert (cl != NULL);
    if (!cl->minimized)
	return;

    XMoveWindow (dpy, cl->window, cl->x, cl->y);
	cl->minimized = false;
}

void
wm_minimize (Client* cl) {
    assert (cl != NULL);
    assert (cl->window != defaultScreen.root);
    assert (wm_fetchClient (cl->window));

    dbg_log ("[ INFO] minimize\n");
    XMoveWindow (dpy, cl->window, -2*cl->w, -2*cl->h);
    cl->minimized = true;
}

void
processcmdargs (int argc, char** argv)
{
    char* logfile = NULL;

	for (int i = 1; i < argc; i++) {
        if (!strcmp (argv[i], "--debug")) {
            int a = 1;
            while (a) {}
        } else if (!strncmp (argv[i], "--log", 6)) {
            if (i + 1 >= argc || argv[i+1][0] == '-')
                continue;

            struct stat tempstat;
            if (stat (argv[i+1], &tempstat) >= 0  && S_ISREG(tempstat.st_mode))
                logfile = argv[i+1];
            else {
                fprintf (stderr, "[ ERROR ]: invalid log file\n");
                exit (1);
            }
        }
    }

    dbg_init (logfile);
    dbg_handlerOn ();
}


int
main (int argc, char** argv) {
    processcmdargs (argc, argv);
	dbg_log ("[ INFO ]	wm starting\n");
	start_wm ();
	dbg_log ("[ INFO ] root = %d\n", defaultScreen.root);
	evt_eventHandler ();
	cleanup ();
	dbg_log ("[ INFO ] normal exit\n");
	dbg_close ();
	return 0;
}

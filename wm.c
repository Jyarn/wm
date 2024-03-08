#include <X11/Xlib.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/extensions/Xrandr.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/cursorfont.h>

#include "wm.h"
#include "event.h"
#include "config.h"
#include "util.h"
#include "debug.h"

Display* dpy;
wm_screen defaultScreen;
FILE* wm_log;
Client* wm_focus;
Client* activeClients;
unsigned int workspacenum;
unsigned int currentmon;

void start_wm (void);
void cleanup (void);
void processcmdargs (int argc, char** argv);

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
    workspacenum = 1;
    currentmon = 0;

    // get list of active windows
    Window rootroot, parent;
    Window* children;
    unsigned int nchild;

    XQueryTree (dpy, defaultScreen.root, &rootroot, &parent, &children, &nchild);
    for (unsigned int i = 0; i < nchild; i++) {
        wm_manage (children[i]);
        wm_grabKeys (children[i]);
    }

    if (children)
        XFree (children);
    wm_setFocus (activeClients);
    // set cursor
    Cursor cursor = XCreateFontCursor (dpy, XC_sailboat);
    XDefineCursor (dpy, defaultScreen.root, cursor);
    XFreeCursor (dpy, cursor);
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

void
wm_changegeomclamp (Client* cl, int x, int y, int w, int h)
{
    if (!cl)
        return;

    dbg_log ("[ INFO ] clamp req, x: %d, y: %d, w: %d, h: %d\n", x, y, w, h);
    cl->w = CLAMP((int)monitors[cl->monnum].w, w, 1);
    cl->h = CLAMP((int)monitors[cl->monnum].h, h, 1);
    cl->x = CLAMP((int)monitors[cl->monnum].w + (int)monitors[cl->monnum].x - cl->w, x, (int)monitors[cl->monnum].x - BORDERWIDTH);
    cl->y = CLAMP((int)monitors[cl->monnum].h + (int)monitors[cl->monnum].y - cl->h, y, (int)monitors[cl->monnum].y - BORDERWIDTH);
    XMoveResizeWindow (dpy, cl->window, cl->x, cl->y, cl->w, cl->h);

    dbg_log ("[ INFO ] clamp res, x: %d, y: %d, w: %d, h: %d\n", cl->x, cl->y, cl->w, cl->h);
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
			GrabModeSync, GrabModeAsync,
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
	if (!XGetWindowAttributes (dpy, w, &attrib) || attrib.override_redirect
     || XGetTransientForHint (dpy, w, &w))
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
    newClient->fullscreen = false;
    newClient->pin = false;

	// init new client
	newClient->window = w;

    // set initial geometry
	newClient->workspace = workspacenum;
    newClient->monnum = currentmon;
    if (wm_shouldbeManaged (w)) {
        int winwidth = monitors[currentmon].w / 2;
        int winheight = monitors[currentmon].h / 2;
        int centerx = monitors[currentmon].x + monitors[currentmon].w / 4;
        int centery = monitors[currentmon].y + monitors[currentmon].h / 4;
        wm_changegeomclamp (newClient, centerx, centery, winwidth, winheight);
        newClient->transient = false;
        dbg_log ("[ INFO ] not transient\n");

    } else {
        newClient->transient = true;
        unsigned int depth, borderwidth;
        Window root;
        XGetGeometry (dpy, w, &root,
                      &newClient->w, &newClient->y,
                     (unsigned int*)&newClient->w, (unsigned int*)&newClient->h,
                      &borderwidth, &depth);
        dbg_log ("[ INFO ] transient\n");
    }

	// set event mask
	XSelectInput (dpy, w, WIN_MASK);
	XSetWindowBorderWidth (dpy, w, BORDERWIDTH);

    // set cursor
    Cursor cursor = XCreateFontCursor (dpy, XC_sailboat);
    XDefineCursor (dpy, w, cursor);
    XFreeCursor (dpy, cursor);
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
			dbg_log ("[ INFO ] wm_unmanage\n");
            if (cur == wm_focus)
                wm_focusNext (false);

			if (prev)
				prev->next = cur->next;
			else
				activeClients = cur->next;

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

    if (!wm_sendatom (wm_fetchClient (w), "WM_DELETE_WINDOW")) {
        XSetCloseDownMode (dpy, DestroyAll);
        XKillClient (dpy, w);
    }

	wm_unmanage (w);
}

void
wm_focusNext (bool focusMinimized) {
	if (!activeClients) {
		wm_setFocus (NULL);
		return;

	} else {
		Client* temp = (wm_focus != NULL) ? wm_focus->next : NULL;
		do {
			if (!temp)
				temp = activeClients;
			else if ((temp->monnum == currentmon)
                  && (workspacenum == temp->workspace || temp->workspace == WORKSPACE_ALWAYSON)
                  && (focusMinimized ^ !temp->minimized)) {
                if (temp->fullscreen){
                    Monitor mon = monitors[temp->monnum];
                    XMoveWindow (dpy, temp->window, mon.x -BORDERWIDTH + FULLSCREENGAP, mon.y -BORDERWIDTH + FULLSCREENGAP);
                } else
                    XMoveWindow (dpy, temp->window, temp->x, temp->y);
                temp->minimized = false;
				wm_setFocus (temp);
				return;
			}
			else
				temp = temp->next;
		} while (temp != wm_focus && (wm_focus || temp));

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

    Client* temp = activeClients;

    if (cl->pin)
        XRaiseWindow (dpy, cl->window);
    else {
        while (temp) {
            if ((temp->pin && temp->monnum == cl->monnum)
             && (!cl->monnum || temp->workspace == cl->workspace)
             && (temp->x <= cl->x + cl->w && cl->x <= temp->x + temp->w)
             && (temp->y <= cl->y + cl->h && cl->y <= temp->y + temp->h))
                break;

            temp = temp->next;
        }

        if (!temp)
            XRaiseWindow (dpy, cl->window);
    }

	wm_focus = cl;
    currentmon = cl->monnum;
	XSetInputFocus (dpy, cl->window, RevertToPointerRoot, CurrentTime);
}

void
wm_show (Client* cl) {
    assert (cl != NULL);
    if (!cl->minimized)
	    return;

    if (cl->fullscreen) {
        Monitor mon = monitors[cl->monnum];
        XMoveWindow (dpy, cl->window, mon.x -BORDERWIDTH + FULLSCREENGAP, mon.y -BORDERWIDTH + FULLSCREENGAP);
    } else {
        XMoveWindow (dpy, cl->window, cl->x, cl->y);
    }

	cl->minimized = false;
}

void
wm_minimize (Client* cl) {
    assert (cl != NULL);
    assert (cl->window != defaultScreen.root);
    assert (wm_fetchClient (cl->window));

    dbg_log ("[ INFO] minimize\n");
    XMoveWindow (dpy, cl->window, -3840, -1080);
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
            if (i + 1 >= argc)
                continue;

            struct stat tempstat;
            if (stat (argv[i+1], &tempstat) >= 0)
                logfile = argv[i+1];
            else {
                fprintf (stderr, "[ ERROR ]: invalid log file\n");
            }
        }
    }

    dbg_init (logfile);
    dbg_handlerOn ();
}


bool
wm_sendatom (Client* cl, char* atomname)
{
    int navailable_atoms;
    Atom* available_atoms;
    Atom send = XInternAtom (dpy, atomname, False);
    XGetWMProtocols (dpy, cl->window, &available_atoms, &navailable_atoms);

    for (int i = 0; i <= navailable_atoms; i++) {
        if (available_atoms[i] == send) {
            XEvent msg;
            msg.type = ClientMessage;


            msg.xclient.display = dpy;
            msg.xclient.window = cl->window;
            msg.xclient.message_type = XInternAtom (dpy, "WM_PROTOCOLS", False);
            msg.xclient.format = 32;
            msg.xclient.data.l[0] = send;
            msg.xclient.data.l[1] = CurrentTime;
            XSendEvent (dpy, cl->window, False, NoEventMask, &msg);
            XFree (available_atoms);
            return true;
        }
    }

    XFree (available_atoms);
    return false;
}


int
main (int argc, char** argv) {
	start_wm ();
    processcmdargs (argc, argv);
	dbg_log ("[ INFO ] wm starting\n");
	dbg_log ("[ INFO ] root = %d\n", defaultScreen.root);
	evt_eventHandler ();
	cleanup ();
	dbg_log ("[ INFO ] normal exit\n");

	dbg_close ();
	return 0;
}

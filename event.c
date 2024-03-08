#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "event.h"
#include "wm.h"
#include "util.h"
#include "config.h"
#include "debug.h"

void onMapReq (void);
void onCircReq (void);
void onWinConfig (void);
void onDestroy (void);
void onUnmap (void);
void onKeyPress (void);
void onButtonPress (void);


bool evt_run = true;
XEvent evt_currentEvent;
handler evt_handlers[LASTEvent] = {
	[MapRequest] = onMapReq,
	[CirculateRequest] = onCircReq,
	[ConfigureRequest] = onWinConfig,
	[DestroyNotify] = onDestroy,
	[UnmapNotify] = onUnmap,
	[KeyPress] = onKeyPress,
	[ButtonPress] = onButtonPress
};

bool ignorenextunmap;


void
onMapReq (void) {
	XMapRequestEvent* ev = &evt_currentEvent.xmaprequest;
	dbg_log ("\n[ INFO ] map request %d\n", ev->window);
	XMapWindow (dpy, ev->window);

    Client* cl = wm_manage (ev->window);
    wm_grabKeys (ev->window);
    wm_grabMouse (ev->window);
    wm_setFocus (cl);
}

void
onCircReq (void) {

}

void
onWinConfig (void) {
    XConfigureRequestEvent* ev = &evt_currentEvent.xconfigurerequest;
    dbg_log ("[ INFO ] config request (x: %d, y: %d, w: %d, h: %d)\n", ev->x, ev->y, ev->width, ev->height);
    Client* cl = wm_fetchClient (ev->window);
    if (cl && cl->transient) {
        cl->monnum = currentmon;
        wm_changegeomclamp (cl, ev->x, ev->y, ev->width, ev->height);
        dbg_log ("[ INFO ] config req (win set to x: %d, y: %d, w: %d, h: %d)\n", cl->x, cl->y, cl->w, cl->h);
    }
}

void
onDestroy (void) {
	XDestroyWindowEvent* ev = &evt_currentEvent.xdestroywindow;
	dbg_log ("\n[ INFO ] destroy window event received\n");
	wm_unmanage (ev->window);
}

void
onUnmap (void) {
	XUnmapEvent* ev = &evt_currentEvent.xunmap;
	Client* cl;
	if ((cl = wm_fetchClient (ev->window)) != NULL && ignorenextunmap) {
        ignorenextunmap = 0;
		return;
    }
	dbg_log ("\n[ INFO ] unmap request\n");
	wm_unmanage (ev->window);
	wm_ungrab (ev->window);
}

void
onKeyPress (void) {
	XKeyPressedEvent* ev = &evt_currentEvent.xkey;
	for (int i = 0; i < N_KEY_BINDS; i++)
		if (ev->keycode == STR_TO_KEYSYM (keyBinds[i].key) && ev->state == keyBinds[i].modifier) {
			keyBinds[i].cmd (keyBinds[i].args);
			return;
		}
}

void
onButtonPress (void) {
	XButtonPressedEvent* ev = &evt_currentEvent.xbutton;
	for (int i = 0; i < N_MOUSE_BINDS; i++)
		if (ev->button == mouseBinds[i].buttons && ev->state == mouseBinds[i].modifier) {
            dbg_log ("[ INFO ] button press\n");
			mouseBinds[i].cmd (mouseBinds[i].args);
            XAllowEvents (dpy, ReplayPointer, CurrentTime);
            XSync (dpy, False);
			return;
		}
}

void
evt_eventHandler (void) {
	// don't wait for X to process requests, since we process
	// them here
	XSync (dpy, False);
	handler h;
    ignorenextunmap = false;

	while (evt_run) {
		XNextEvent (dpy, &evt_currentEvent);
		assert (evt_currentEvent.type < LASTEvent);

		h = evt_handlers[evt_currentEvent.type];
		if (h)
			h ();
	}
}

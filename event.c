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






void
onMapReq (void) {
	XMapRequestEvent* ev = &evt_currentEvent.xmaprequest;
	dbg_log ("\n[ INFO ] map request %d\n", ev->window);

	if (wm_shouldbeManaged (ev->window)) {
		wm_manage (ev->window);
		wm_grabKeys (ev->window);
		wm_grabMouse (ev->window);
		XMapWindow (dpy, ev->window);
		wm_setFocus (ev->window);
	}
}

void
onCircReq (void) {

}

void
onWinConfig (void) {

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
			mouseBinds[i].cmd (mouseBinds[i].args);
			return;
		}
}

void
evt_eventHandler (void) {
	// don't wait for X to process requests, since we process
	// them here
	XSync (dpy, False);
	handler h;

	while (evt_run) {
		XNextEvent (dpy, &evt_currentEvent);
		assert (evt_currentEvent.type < LASTEvent);

		h = evt_handlers[evt_currentEvent.type];
		if (h)
			h ();
	}
}
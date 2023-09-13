#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "event.h"
#include "wm.h"
#include "util.h"
#include "config.h"
#include "debug.h"


typedef bool (*handler)(XEvent*);
bool onMapReq (XEvent* event);
bool onCircReq (XEvent* event);
bool onWinConfig (XEvent* event);
bool onCreate (XEvent* event);
bool onDestroy (XEvent* event);
bool onUnmap (XEvent* event);
bool onKeyPress (XEvent* event);
bool onButtonPress (XEvent* event);




bool onMapReq (XEvent* event) {
	XMapRequestEvent* ev = (XMapRequestEvent* )event;
	dbg_log ("[ INFO ] map request\n");
	if (wm_shouldbeManaged (ev->window)) {
		wm_manage (ev->window);
		wm_grabKeys (ev->window, GrabModeAsync);
		wm_grabMouse (ev->window, GrabModeAsync);
		XMapWindow (dpy, ev->window);
	}

	return true;
}

bool onCircReq (XEvent* event UNUSED) {
	return true;
}

bool onWinConfig (XEvent* event UNUSED) {
	return true;
}

bool onCreate (XEvent* event) {
	XCreateWindowEvent* ev = ((XCreateWindowEvent* )event);
	return true;
}

bool onDestroy (XEvent* event) {
	XDestroyWindowEvent* ev = (XDestroyWindowEvent* )event;
	dbg_log ("[ INFO ]	destroy window event received\n");
	wm_unmanage (ev->window);
	dbg_log (" [ INFO ] window destroyed\n");
	return true;
}

bool onUnmap (XEvent* event) {
	XUnmapEvent* ev = (XUnmapEvent* )event;
	dbg_log ("[ INFO ] unmap request\n");
	wm_unmanage (ev->window);

	return true;
}

bool onKeyPress (XEvent* event) {
	XKeyPressedEvent* ev = (XKeyPressedEvent* )event;
	for (int i = 0; i < N_KEY_BINDS; i++)
		if (ev->keycode == STR_TO_KEYSYM (keyBinds[i].key) && ev->state == keyBinds[i].modifier) {
			return keyBinds[i].cmd (keyBinds[i].args, ev->window);
		}

	return true;
}

bool onButtonPress (XEvent* event) {
	XButtonPressedEvent* ev = (XButtonPressedEvent* )event;
	for (int i = 0; i < N_MOUSE_BINDS; i++) {
		if (ev->button == mouseBinds[i].buttons && ev->state == mouseBinds[i].modifier) {
			return mouseBinds[i].cmd (mouseBinds[i].args, ev->window);
		}
	}

	return true;
}

void evt_eventHandler (void) {
	// don't wait for X to process requests, since we process
	// them here
	XEvent event;
	handler func;
	XSync (dpy, False);

	for (;;) {
		XNextEvent (dpy, &event);
		switch (event.type) {
			case MapRequest:
				func = onMapReq;
				break;
			case CirculateRequest:
				func = onCircReq;
				break;
			case ConfigureRequest:
				func = onWinConfig;
				break;
			case CreateNotify:
				func = onCreate;
				break;
			case DestroyNotify:
				func = onDestroy;
				break;
			case UnmapNotify:
				func = onUnmap;
				break;
			case KeyPress:
				func = onKeyPress;
				break;
			case ButtonPress:
				func = onButtonPress;
				break;
			default:
				continue;
		}

		if (func (&event) == false)
			return;
	}
}
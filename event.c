#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "event.h"
#include "wm.h"
#include "util.h"
#include "config.h"
#include "debug.h"


static const moveBind* activeMotionBind = NULL;
static MotionEvent motionEventData = DEFAULT_MOTION_EVENT;


bool onMapReq (XEvent* event);
bool onCircReq (XEvent* event);
bool onWinConfig (XEvent* event);
bool onDestroy (XEvent* event);
bool onUnmap (XEvent* event);
bool onKeyPress (XEvent* event);
bool onButtonPress (XEvent* event);
bool onMotion (XEvent* event);




bool onMapReq (XEvent* event) {
	XMapRequestEvent* ev = (XMapRequestEvent* )event;
	dbg_log ("\n[ INFO ] map request\n");
	if (wm_shouldbeManaged (ev->window)) {
		wm_manage (ev->window);
		wm_grabKeys (ev->window, GrabModeAsync);
		wm_grabMouse (ev->window, GrabModeAsync);
		XMapWindow (dpy, ev->window);
		wm_setFocus (ev->window);
	}

	return true;
}

bool onCircReq (XEvent* event UNUSED) {
	return true;
}

bool onWinConfig (XEvent* event UNUSED) {
	return true;
}

bool onDestroy (XEvent* event) {
	XDestroyWindowEvent* ev = (XDestroyWindowEvent* )event;
	dbg_log ("\n[ INFO ] destroy window event received\n");
	wm_unmanage (ev->window);
	return true;
}

bool onUnmap (XEvent* event) {
	XUnmapEvent* ev = (XUnmapEvent* )&event->xunmap;
	dbg_log ("\n[ INFO ] unmap request\n");
	wm_unmanage (ev->window);
	wm_ungrab (ev->window);

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
		if (ev->button == mouseBinds[i].buttons && ev->state == mouseBinds[i].modifier)
			return mouseBinds[i].cmd (mouseBinds[i].args, ev->window);
	}

	for (int i = 0; i < N_MOVE_BINDS; i++) {
		if (ev->button == moveBinds[i].buttons && ev->state == moveBinds[i].modifier) {
			activeMotionBind = &moveBinds[i];
			motionEventData.x = 0;
			motionEventData.y = 0;
			motionEventData.firstCall = false;
			motionEventData.w = ev->window;

			WM_UNGRABPOINTER (ev->window);
			WM_GRABPOINTER (ev->window);

			dbg_log ("[ INFO ] disabling key/mouse button handling\n");
		}
	}
	return true;
}

bool onMotion (XEvent* event) {
	if (!activeMotionBind)
		return true;

	XMotionEvent* curEvent = (XMotionEvent* )event;
	bool reset = false;
	motionEventData.x = curEvent->x_root;
	motionEventData.y = curEvent->y_root;

	bool ret = activeMotionBind->cmd (activeMotionBind->args, &motionEventData, &reset);
	motionEventData.firstCall = true;

	if (reset)
		activeMotionBind = NULL;
	return ret;
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
			case DestroyNotify:
				func = onDestroy;
				break;
			case UnmapNotify:
				func = onUnmap;
				break;
			case KeyPress:
				func = activeMotionBind ? NULL : onKeyPress;
				break;
			case ButtonPress:
				func = activeMotionBind ? NULL : onButtonPress;
				break;
			case ButtonRelease:
				func = NULL;
				activeMotionBind = NULL;
				break;
			case MotionNotify:
				func = onMotion;
				break;
			default:
				continue;
		}

		if (func && func (&event) == false)
			return;
	}
}
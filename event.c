#include <X11/Xlib.h>
#include <stdbool.h>
#include "event.h"
#include "wm.h"
#include "util.h"
#include "config.h"



typedef bool (*handler)(XEvent*);
bool onMapReq (XEvent* event);
bool onCircReq (XEvent* event);
bool onWinConfig (XEvent* event);
bool onKeyPress (XEvent* event);
bool onKeyRelease (XEvent* event);
bool onButtonPress (XEvent* event);
bool onButtonRelease (XEvent* event);




bool onMapReq (XEvent* event) {
	XMapWindow (dpy, ((XMapRequestEvent* )event)->window);
	return true;
}

bool onCircReq (XEvent* event) {
	return true;
}

bool onWinConfig (XEvent* event) {
	return true;
}


bool onKeyPress (XEvent* event) {
	XKeyPressedEvent* ev = (XKeyPressedEvent* )event;
	for (int i = 0; i < N_KEY_BINDS; i++)
		if (ev->keycode == STR_TO_KEYSYM (keyBinds[i].key) && ALLOWMASK (ev->state) == ALLOWMASK (keyBinds[i].modifier)) {
			return keyBinds[i].cmd (keyBinds[i].args);
		}

	return true;
}

bool onButtonPress (XEvent* event) {
	return false;
}

bool onButtonRelease (XEvent* event) {
	return false;
}


void evt_eventHandler (void) {
	// don't wait for X to process requests, since we process
	// them here
	XEvent event;
	handler func;
	XSync (dpy, False);

	for (;;) {
		if (XNextEvent (dpy, &event))
			return;
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
			case KeyPress:
				func = onKeyPress;
				break;
			case ButtonPress:
				func = onButtonPress;
				break;
			case ButtonRelease:
				func = onButtonRelease;
				break;
			default:
				continue;
		}

		if (func (&event) == false)
			return;
	}
}
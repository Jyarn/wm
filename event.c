#include <X11/Xlib.h>
#include "event.h"
#include "wm.h"

#define EVENT_HANDLER_EXIT 0
#define EVENT_HANDLER_CONT 1

typedef int (*handler)(XEvent*);
int onMapReq (XEvent* event);
int onCircReq (XEvent* event);
int onWinConfig (XEvent* event);




int onMapReq (XEvent* event) {
	return EVENT_HANDLER_CONT;
}

int onCircReq (XEvent* event) {
	return EVENT_HANDLER_CONT;
}

int onWinConfig (XEvent* event) {
	return EVENT_HANDLER_CONT;
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
			default:
				continue;
		}

		if (func (&event) == EVENT_HANDLER_EXIT)
			return;
	}
}
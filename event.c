#include <X11/Xlib.h>
#include <stdbool.h>
#include <assert.h>

#include "event.h"
#include "wm.h"
#include "util.h"
#include "dbg.h"

void onmap(XEvent*);
void onwinconfig(XEvent*);
void ondestroy(XEvent*);
void onunmap(XEvent*);
void onkeypress(XEvent*);
void runevent_handler(void);

bool evt_run = true;
handler evt_handlers[LASTEvent] = {
	[MapRequest] = onmap,
	[ConfigureRequest] = onwinconfig,
	[DestroyNotify] = ondestroy,
	[UnmapNotify] = onunmap,
	[KeyPress] = onkeypress,
};

void
onmap(XEvent* currentevent)
{
    XMapWindow(dpy, currentevent->xmaprequest.window);
    manage(currentevent->xmaprequest.window);
}

void
onwinconfig(XEvent* currentevent)
{
    ClientIndex c = fetchclient(currentevent->xconfigure.window);
    if (c != -1 && clients.clients[c].mask & TRANSIENT_MASK) {
        assert(clients.clients[c].window == currentevent->xconfigure.window);
        XMoveResizeWindow(dpy, currentevent->xconfigure.window,
                currentevent->xconfigure.x, currentevent->xconfigure.y,
                currentevent->xconfigure.width, currentevent->xconfigure.height);
    }
}

void
ondestroy(XEvent* currentevent)
{
    unmanage(currentevent->xdestroywindow.window);
}

void
onkeypress(XEvent* currentevent)
{

}

void
runevent_handler(void)
{
    XSync(dpy, False);
    handler h;
    // TODO remove evt_run flag and replace with signal or something
    evt_run = true;
    XEvent curevent;

    while(evt_run) {
        XNextEvent(dpy, &curevent);
        assert(curevent.type < LASTEvent);
        h = handlers[curevent.type];
        if (h)
            h(&curevent);
    }
}

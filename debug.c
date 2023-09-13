#include <stdio.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include "debug.h"
#include "util.h"


static FILE* logFile;
static XErrorHandler handlerDefault;

int handlerUser (Display*, XErrorEvent*);

void dbg_log (const char* str) {
    fprintf (logFile, str);
    fflush (logFile);
}

void dbg_init (void) {
    logFile = fopen ("/home/r3st/.repos/wm/wm.log", "w");
    handlerDefault = XSetErrorHandler (handlerUser);
}

void dbg_close (void) {
    fclose (logFile);
}

int handlerUser (Display* dpy, XErrorEvent* event) {
    switch (event->type) {
        case BadAccess:
            dbg_log ("[ ERROR ] Bad Access");
            break;
        case BadValue:
            dbg_log ("[ Error ] Bad Value");
            break;
        case BadWindow:
            dbg_log ("[ ERROR ] Bad Window");
            break;
    }

    return handlerDefault (dpy, event);
}
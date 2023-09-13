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
    dbg_log ("[ ERROR ] Xerror\n");
    return handlerDefault (dpy, event);
}
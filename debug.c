#include <stdio.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdbool.h>

#include "debug.h"
#include "util.h"


static FILE* logFile;
static XErrorHandler handlerDefault = NULL;


int handlerUser (Display*, XErrorEvent*);
int handlerIgnore (Display*, XErrorEvent*);

void dbg_log (const char* str) {
    fprintf (logFile, str);
    fflush (logFile);
}

void dbg_init (void) {
    logFile = fopen ("/home/flt/REPOS/wm/wm.log", "w");
    handlerDefault = XSetErrorHandler (handlerUser);
}

void dbg_close (void) {
    fclose (logFile);
}

int handlerUser (Display* dsp, XErrorEvent* event) {
    char bff[512];
    XGetErrorText (dsp, event->error_code, bff, 512);
    fprintf (logFile, "[ ERROR ] xerror: %s\n", bff);
    return handlerDefault (dsp, event);
}

int handlerOff (Display* dsp, XErrorEvent* event) {
    char bff[512];
    XGetErrorText (dsp, event->error_code, bff, 512);
    fprintf (logFile, "[ WARNING ] xerror: %s\n", bff);
    return 0;
}

void dbg_handlerOn (void) {
    XSetErrorHandler (handlerUser);
    fprintf (logFile, "[ INFO ] handler on\n");
}

void dbg_handlerOff (void) {
    XSetErrorHandler (handlerOff);
    fprintf (logFile, "[ INFO ] handler off\n");
}
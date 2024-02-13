#include <stdio.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "debug.h"
#include "util.h"


static FILE* logFile;
static XErrorHandler handlerDefault = NULL;


int handlerUser (Display*, XErrorEvent*);
int handlerIgnore (Display*, XErrorEvent*);
int handlerOff (Display* dsp, XErrorEvent* event);

void dbg_log (const char* str, ...) {
    if (logFile == NULL)
        return;

    va_list args;
    va_start (args, str);

    vfprintf (logFile, str, args);
    fflush (logFile);
}

void dbg_init (char* file) {
    if (file != NULL) {
        logFile = fopen (file, "w");
        dbg_log ("[ INFO ] begin log\n");
    } else
        logFile = NULL;

    handlerDefault = XSetErrorHandler (handlerUser);
}

void dbg_close (void) {
    if (logFile)
        fclose (logFile);
}

int handlerUser (Display* dsp, XErrorEvent* event) {
    char bff[512];
    XGetErrorText (dsp, event->error_code, bff, 512);
    dbg_log ("[ ERROR ] xerror: %s\n", bff);
    return handlerDefault (dsp, event);
}

int handlerOff (Display* dsp, XErrorEvent* event) {
    char bff[512];
    XGetErrorText (dsp, event->error_code, bff, 512);
    dbg_log ("[ WARNING ] xerror: %s\n", bff);
    return 0;
}

void dbg_handlerOn (void) {
    XSetErrorHandler (handlerUser);
    dbg_log ("[ INFO ] handler on\n");
}

void dbg_handlerOff (void) {
    XSetErrorHandler (handlerOff);
    dbg_log ("[ INFO ] handler off\n");
}

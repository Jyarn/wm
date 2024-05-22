#include <unistd.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "util.h"
#include "dbg.h"

static XErrorHandler defaulthandler = NULL;

int handleruser(Display*, XErrorEvent*);
int handlerignore(Display*, XErrorEvent*);
int handleroff(Display*, XErrorEvent*);

#ifdef __DEBUG
void
dbg_log(const char* str, ...)
{
    char b[128];

    va_list args;
    va_start(args, str);

    int n = vsnprintf(b, 128, str, args);
    write(STDOUT_FILENO, b, n < 128 ? n : 128);
}
#endif

void
inithandler()
{
    defaulthandler = XSetErrorHandler(handleruser);
}

int
handleruser(Display* dsp, XErrorEvent* event)
{
    char bff[64];
    XGetErrorText(dsp, event->error_code, bff, 512);
    dbg_log("(E) xerror: %s\n", bff);
    return defaulthandler(dsp, event);
}

int
handlerignore(Display* dsp, XErrorEvent* event)
{
    char bff[64];
    XGetErrorText (dsp, event->error_code, bff, 64);
    dbg_log ("(W) xerrror: %s\n");
    return 0;
}

void
handlerOn (void)
{
    XSetErrorHandler(handleruser);
    dbg_log ("(I) handler on\n");
}

void
handlerOff (void)
{
    XSetErrorHandler(handlerignore);
    dbg_log ("(I) handler off\n");
}


#ifndef __EVENT__H
#define __EVENT__H

#include <X11/Xlib.h>


typedef void (*handler)(XEvent*);
void evt_eventHandler (void);

extern bool evt_run;
extern handler handlers[LASTEvent];

#endif

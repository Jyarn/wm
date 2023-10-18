#ifndef __EVENT__H
#define __EVENT__H

#include <X11/Xlib.h>


typedef void (*handler)(void);
void evt_eventHandler (void);

extern bool evt_run;
extern XEvent evt_currentEvent;
extern handler evt_handlers[LASTEvent];

#endif
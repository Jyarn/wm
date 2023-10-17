#ifndef __EVENT__H
#define __EVENT__H

#include <X11/Xlib.h>


typedef struct {
    Window w;
    bool firstCall;

    unsigned int x;
    unsigned int y;
    unsigned int prevX;
    unsigned int prevY;

} MotionEvent;

typedef bool (*handler)(XEvent*);
void evt_eventHandler (void);

#endif
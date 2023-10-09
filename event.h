#ifndef __EVENT__H
#define __EVENT__H

#include <X11/Xlib.h>

#define DEFAULT_MOTION_EVENT { 0, false, 0, 0 }

typedef struct {
    Window w;
    bool firstCall;

    int x;
    int y;
    int prevX;
    int prevY;

} MotionEvent;

typedef bool (*handler)(XEvent*);
void evt_eventHandler (void);

#endif
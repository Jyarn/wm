#ifndef __WM__H
#define __WM__H
#include <X11/Xlib.h>

typedef struct {
    int screen;
    int w;
    int h;
    int root;
} screen;

extern Display* dpy;
extern screen defaultScreen;

#endif
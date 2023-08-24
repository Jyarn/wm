#ifndef __WM__H
#define __WM__H
#include <X11/Xlib.h>
#include <stdbool.h>

typedef struct {
    int screen;
    int w;
    int h;
    int root;
} screen;

typedef struct s_client {
    int w;
    int h;
    int x;
    int y;
    struct s_client* next;
    Window window;
} client;

extern Display* dpy;
extern screen defaultScreen;


bool wm_shouldbeManaged (Window w);
void wm_manage (Window w);
void wm_unmanage (Window w);
bool wm_killClient (Window w);

#endif
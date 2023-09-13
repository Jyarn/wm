#ifndef __WM__H
#define __WM__H
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define ROOT_MASK SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask | ButtonPressMask
#define WIN_MASK KeyPressMask | ButtonPressMask
#define MOUSE_MASK ButtonPressMask


typedef struct {
    int screen;
    int w;
    int h;
    Window root;
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
bool wm_killClient (void* args);
void wm_setFocus (Window w);
void wm_grabKeys (Window win, int sync);
void wm_grabMouse (Window win, int sync);

#endif
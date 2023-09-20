#ifndef __WM__H
#define __WM__H
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define ROOT_MASK SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask | ButtonPressMask | PointerMotionMask
#define WIN_MASK StructureNotifyMask
#define MOUSE_MASK ButtonPressMask
#define MOTION_MASK MOUSE_MASK | PointerMotionMask

typedef struct {
    int screen;
    int w;
    int h;
    Window root;
} screen;

typedef struct s_client {
    unsigned int w;
    unsigned int h;
    int x;
    int y;
    struct s_client* next;
    Window window;
} client;

extern Display* dpy;
extern screen defaultScreen;
extern client* wm_focus;

bool wm_shouldbeManaged (Window w);
void wm_manage (Window w);
void wm_unmanage (Window w);
void wm_killClient (Window w);
void wm_setFocus (Window w);
void wm_grabKeys (Window win, int sync);
void wm_grabMouse (Window win, int sync);
void wm_setFocus (Window w);
void wm_ungrab (Window w);
client* wm_fetchClient (Window w);

#endif
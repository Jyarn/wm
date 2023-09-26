#ifndef __WM__H
#define __WM__H
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define ROOT_MASK SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask | ButtonPressMask | PointerMotionMask
#define WIN_MASK StructureNotifyMask
#define MOUSE_MASK ButtonPressMask
#define MOTION_MASK PointerMotionMask | ButtonPressMask | ButtonReleaseMask
#define IN_MOTION_MASK SubstructureRedirectMask | SubstructureNotifyMask | PointerMotionMask

#define WM_GRABPOINTER(win) XGrabPointer (dpy, defaultScreen.root, False, MOTION_MASK, GrabModeAsync, GrabModeAsync, None, None, CurrentTime)
#define WM_UNGRABPOINTER(win) XUngrabPointer (dpy, CurrentTime);

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

bool wm_shouldbeManaged (Window);
void wm_manage (Window);
void wm_unmanage (Window);
void wm_killClient (Window);
void wm_setFocus (Window);
void wm_grabKeys (Window, int);
void wm_grabMouse (Window, int);
void wm_grabPointerBinds (Window, int);
void wm_setFocus (Window);
void wm_ungrab (Window);
void wm_moveWindow (Window, int, int);
client* wm_fetchClient (Window);

#endif
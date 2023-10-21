#ifndef __WM__H
#define __WM__H
#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define ROOT_MASK SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask | ButtonPressMask | PointerMotionMask
#define WIN_MASK StructureNotifyMask
#define MOUSE_MASK ButtonPressMask | ButtonReleaseMask
#define MOTION_MASK PointerMotionMask | ButtonPressMask | ButtonReleaseMask

#define WM_GRABPOINTER() XGrabPointer (dpy, defaultScreen.root, False, MOTION_MASK, GrabModeAsync, GrabModeAsync, None, None, CurrentTime)
#define WM_UNGRABPOINTER() XUngrabPointer (dpy, CurrentTime);

typedef struct {
    int screen;
    int w;
    int h;
    Window root;
} wm_screen;

typedef struct s_client {
    unsigned int w;
    unsigned int h;
    int x;
    int y;
    struct s_client* next;
    Window window;
} Client;

extern Display* dpy;
extern wm_screen defaultScreen;
extern Client* wm_focus;

bool wm_shouldbeManaged (Window);
void wm_manage (Window);
void wm_unmanage (Window);
void wm_killClient (Window);
void wm_setFocus (Window);
void wm_grabKeys (Window);
void wm_grabMouse (Window);
void wm_grabPointerBinds (Window);
void wm_setFocus (Window);
void wm_ungrab (Window);
void wm_moveWindow (Client*, int, int);
void wm_changeGeomRelative (Client*, int, int, int, int);
Client* wm_fetchClient (Window);

#endif
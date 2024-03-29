#ifndef __WM__H
#define __WM__H

#include <X11/Xlib.h>
#include <stdbool.h>
#include <stdarg.h>


#define ROOT_MASK SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | PointerMotionMask
#define WIN_MASK StructureNotifyMask
#define MOUSE_MASK ButtonPressMask | ButtonReleaseMask
#define MOTION_MASK PointerMotionMask | ButtonPressMask | ButtonReleaseMask

#define WM_GRABPOINTER(cursor) { \
        Cursor _cursor = XCreateFontCursor (dpy, cursor); \
        XGrabPointer (dpy, defaultScreen.root, False, MOTION_MASK, GrabModeAsync, GrabModeAsync, None, _cursor, CurrentTime); \
        XFreeCursor (dpy, _cursor); \
    }

#define WM_UNGRABPOINTER() XUngrabPointer (dpy, CurrentTime)
#define CURRENT_WINDOW evt_currentEvent.xany.window

typedef struct {
    int screen;
    Window root;
    unsigned int nmon;
} wm_screen;

typedef struct s_client {
    int w;
    int h;
    int x;
    int y;
    struct s_client* next;
    Window window;
    bool fullscreen;
    bool minimized;
    bool transient;
    bool pin;
    unsigned int monnum;
    unsigned int workspace;
} Client;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} Monitor;

extern Display* dpy;
extern wm_screen defaultScreen;
extern Client* wm_focus;
extern Client* activeClients;
extern unsigned int workspacenum;
extern unsigned int currentmon;

bool wm_shouldbeManaged (Window);
Client* wm_manage (Window);
void wm_unmanage (Window);
void wm_killClient (Window);
void wm_setFocus (Client*);
void wm_focusNext (bool);
void wm_minimize (Client*);
void wm_show (Client* cl);
void wm_grabKeys (Window);
void wm_grabMouse (Window);
void wm_ungrab (Window);
void wm_changeGeomRelative (Client*, int, int, int, int);
Client* wm_fetchClient (Window);
void wm_changegeomclamp (Client* cl, int x, int y, int w, int h);
bool wm_sendatom (Client* cl, char* atomname);

#endif

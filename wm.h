#ifndef __WM__H
#define __WM__H

#include <X11/Xlib.h>

#include "util.h"

#define PIN_MASK 1
#define MINIMIZED_MASK 2
#define TRANSIENT_MASK 4
#define FULLSCREEN_MASK 8
#define SEEN_MASK 16

#define THRESHOLD 2

#define ROOT (RootWindow(dpy, DefaultScreen(dpy)))
#define COLOURMAP (DefaultColormap(dpy, DefaultScreen(dpy)))

typedef int ClientIndex;

typedef struct {
    char mask;
    char workspace;
    char monitor;
    Window window;
#ifdef __DEBUG
    int id;           // id value to distinguish clients for debugging. -1 means Client is uninitialized
#endif
} Client;

typedef struct {
    int size;
    int capacity;
    Client* clients;
} Clients;

extern Clients clients;
extern Display* dpy;

#ifdef __DEBUG
void shiftleft(ClientIndex at);
void shiftright(ClientIndex at);
void insert(Client* c, ClientIndex at);
void delete(ClientIndex at);
#endif

void start(void);
void cleanup(void);
ClientIndex fetchclient(Window w);
ClientIndex manage(Window w);
void unmanage(Window c);
void killclient(Window w);
void focusnext(bool focusMinimized);
void setfocus(ClientIndex c);
#endif

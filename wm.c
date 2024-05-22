#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <X11/cursorfont.h>

#include "dbg.h"
#include "util.h"
#include "wm.h"

Clients clients;
Display* dpy;
ClientIndex wmfocus;

typedef struct {
    unsigned char value[3];
    XColor colour;
} Colour;

typedef enum {
    Inactive = 0,
    Default = 1,
    Pinned = 2,
} ColourEnum;

Colour colours[] = {
    { .value = 0x000000 },
    { .value = 0x6082B6 },
    { .value = 0x228B22 },
};


#ifdef __DEBUG
int id = 0;
#endif

static inline void colourwinborder(Window w, ColourEnum cl);
bool shouldbemanaged(Window w);
void shiftleft(ClientIndex at);
void shiftright(ClientIndex at);
void insert(Client* c, ClientIndex at);
void delete(ClientIndex at);
void start(void);
void cleanup(void);
ClientIndex fetchclient(Window w);
ClientIndex manage(Window w);
void unmanage(Window c);
void killclient(Window w);
void focusnext(bool focusMinimized);
void setfocus(ClientIndex c);
static inline int hashclient(Client* cl);
static inline void stack(ClientIndex c);
bool sendatom(Window w, char* name);

#ifdef __DEBUG
static inline void assertclients(void);
#endif
#ifndef __DEBUG
#define assertclients()
#endif

void
shiftleft(ClientIndex at)
{
    assert(at < clients.size);
    clients.size--;
    for (int i = at; i < clients.size; i++) {
    #ifdef __DEBUG
        // check the values we write to are uninitialized
        assert(clients.clients[i].id == -1);
    #endif

        clients.clients[i] = clients.clients[i+1];

    #ifdef __DEBUG
        clients.clients[i+1].id = -1;
    #endif
    }

    assert(clients.size < clients.capacity);
    if (clients.capacity - clients.size == THRESHOLD) {
    #ifdef __DEBUG
        // check all values before are uninitialized
        for (int i = clients.size+1; i < clients.capacity; i++)
            assert(clients.clients[i].id == -1);
    #endif

        if (clients.capacity == THRESHOLD) {
            free(clients.clients);
            clients.clients = NULL;
        } else
            clients.clients = realloc(clients.clients, (clients.capacity-THRESHOLD)*sizeof(Client));

        clients.capacity -= THRESHOLD;
    }

}

void
shiftright(ClientIndex at)
{
    assert(clients.size <= clients.capacity);
    if (clients.capacity == clients.size) {
        clients.clients = realloc(clients.clients, (clients.capacity+THRESHOLD)*sizeof(Client));
    #ifdef __DEBUG
        // set each value uninitialized Client to -1
        for (int i = 0; i < THRESHOLD; i++)
            clients.clients[i+clients.capacity].id = -1;
    #endif

        clients.capacity += THRESHOLD;
    }

    clients.size++;
    for (int i = clients.size - 2; i >= at; i--) {
    #ifdef __DEBUG
        assert(clients.clients[i+1].id == -1);
    #endif

        clients.clients[i+1] = clients.clients[i];

    #ifdef __DEBUG
        clients.clients[i].id = -1;
    #endif
    }
}

void
insert(Client* c, ClientIndex at)
{
    assert(0 <= at);
    assert(at <= clients.size);
    shiftright(at);
#ifdef __DEBUG
    assert(clients.clients[at].id == -1);
#endif

    clients.clients[at] = *c;

#ifdef __DEBUG
    clients.clients[at].id = id++;
#endif
    assertclients();
}

void
delete(ClientIndex at)
{
    assert(0 <= at);
    assert(at < clients.size);
#ifdef __DEBUG
    // mark elem for deletion
    clients.clients[at].id = -1;
#endif

    shiftleft(at);
    assertclients();
}

void
cleanup(void)
{
    free(clients.clients);
    for (unsigned long int i = 0; i < sizeof(colours)/sizeof(Colour); i++)
        XFreeColors(dpy, COLOURMAP, &colours[i].colour.pixel, 1, 0);
}

void
start(void)
{
    clients.capacity = 0;
    clients.size = 0;
    clients.clients = NULL;

    dpy = XOpenDisplay(NULL);

    XSetWindowAttributes wa;
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask;
    XSelectInput(dpy, ROOT, wa.event_mask);

    // TODO query tree
    //

    // TODO set cursor
    //

    // TODO colours
    //

    // alloc colours (TODO shift colours into arr w/ enum)
    for (unsigned long int i = 0; i < sizeof(colours)/sizeof(Colour); i++)
        XAllocColor(dpy, COLOURMAP, &colours[i].colour);
}

ClientIndex
fetchclient(Window w)
{
    for (int i = 0; i < clients.size; i++)
        if (clients.clients[i].window == w)
            return i;
    return -1;
}

static inline int
hashclient(Client* cl)
{
    return (cl->monitor << 9) | (cl->workspace << 1) | (cl->mask & 1);
}

bool
shouldbemanaged(Window w)
{
    XWindowAttributes attrib;
	if (w == (Window)ROOT)
		return false;
	if (!XGetWindowAttributes (dpy, w, &attrib) || attrib.override_redirect
     || (XGetTransientForHint (dpy, w, &w)))
		return false;
	return true;

}

ClientIndex
manage(Window w)
{
    if (fetchclient(w) != -1)
        return -1;

    int ret;
    Client ins = clients.clients[wmfocus];
    ins.window = w;
    ins.mask = shouldbemanaged(w) ? TRANSIENT_MASK : 0;

#ifdef __DEBUG
    ins.id = -1;
#endif

    int hash = hashclient(&ins);
    // store in non-ascending order
    for (int i = 0; i < clients.size; i++) {
        if (hash >= hashclient(&clients.clients[i])) {
            ret = i;
            insert(&ins, i);
            goto _exit;
        }
    }

    ret = clients.size;
    insert(&ins, clients.size);

_exit:
#ifdef __DEBUG
    assert(clients.clients[ret].id == id-1);
#endif

    XGrabKeyboard(dpy, w, False, GrabModeAsync, GrabModeAsync, CurrentTime);

    // TODO
    XSetWindowBorderWidth(dpy, w, 100);

    // set cursor
    Cursor cursor = XCreateFontCursor (dpy, XC_sailboat);
    XDefineCursor (dpy, w, cursor);
    XFreeCursor (dpy, cursor);

    if (ins.mask & TRANSIENT_MASK)
        XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);

    if (ret != 0)
        stack(ret);
    stack(ret);
    return ret;
}

bool
sendatom(Window w, char* name)
{
    int natoms;
    Atom* atoms;
    Atom atom = XInternAtom(dpy, name, False);
    XGetWMProtocols(dpy, w, &atoms, &natoms);

    for (int i = 0; i <= natoms; i++)
        if (atoms[i] == atom) {
            XEvent msg;
            msg.type = ClientMessage;
            msg.xclient.display = dpy;
            msg.xclient.window = w;
            msg.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", False);
            msg.xclient.format = 32;
            msg.xclient.data.l[0] = atom;
            msg.xclient.data.l[1] = CurrentTime;

            XSendEvent(dpy, w, False, NoEventMask, &msg);
            XFree(atoms);
            return true;
        }

    return false;
}

void
killclient(Window w)
{
    assert(w != (Window)ROOT);

    if (!sendatom(w, "WM_DELETE_WINDOW")) {
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, w);
    }

    unmanage(w);
}

void
unmanage(Window w)
{
    ClientIndex c = fetchclient(w);
    if (c == -1)
        return;

    delete(c);
    if (c == wmfocus)
        focusnext(false);

    XGrabServer(dpy);
    handlerOff();
    XSelectInput(dpy, w, NoEventMask);
    XUngrabKeyboard(dpy, CurrentTime);
    XUngrabServer(dpy);
    XSync(dpy, False);
    handlerOn();
}

/*
 * loop through each element and ensure that each client is ordered properly
 * TODO move to dbg
 */
#ifdef __DEBUG
static inline void
assertclients()
{
    for (int i = 1; i < clients.size; i++) {
        assert(clients.clients[i].id != -1);

        Client p = clients.clients[i-1];
        Client c = clients.clients[i];
        if (p.monitor < c.monitor)
            assert(false);
        if (p.monitor > c.monitor)
            continue;

        if (p.workspace < c.workspace)
            assert(false);
        if (p.workspace > c.workspace)
            continue;

        if ((p.mask & PIN_MASK) < (c.mask & PIN_MASK))
            assert(false);
    }
}
#endif

static inline void
stack(ClientIndex c)
{
    assert(c+1 < clients.size);
    /*
    if (c+1 == clients.size)
        return;

    if (hashclient(&clients.clients[c]) == hashclient(&clients.clients[c+1]))
    */

    XWindowChanges ch = { .sibling = clients.clients[c+1].window, .stack_mode = Below };
    XConfigureWindow(dpy, clients.clients[c].window, CWSibling | CWStackMode, &ch);
}

void
setfocus(ClientIndex c)
{
    assertclients();

    Client cl = clients.clients[c];
#ifdef __DEBUG
    clients.clients[c].id = -1;
#endif

    int h = hashclient(&cl);
    int i = c-1;

    while (i >= 0 && hashclient(&clients.clients[i]) - h <= 1) {
#ifdef __DEBUG
        assert(clients.clients[i+1].id == -1);
#endif

        clients.clients[i+1] = clients.clients[i];
#ifdef __DEBUG
        clients.clients[i].id = -1;
#endif
        i--;
    }

    assert(hashclient(&clients.clients[i]) != hashclient(&clients.clients[i+1]));
#ifdef __DEBUG
    assert(clients.clients[i+1].id == -1);
#endif

    stack(c);
    stack(c+1);
    stack(i);
    stack(i+1);
    wmfocus = i+1;
}

static inline void
colourwinborder(Window w, ColourEnum cl)
{
    XSetWindowBorder(dpy, w, colours[cl].colour.pixel);
}

void
focusnext(bool focus_minimized UNUSED)
{
    assertclients();
    ClientIndex pre = -1;
    ClientIndex target = -1;
    const Client w = clients.clients[wmfocus];
    for (int i = 0; i < clients.size; i++) {
        Client ind = clients.clients[i];

        if (ind.monitor != w.monitor)
            continue;
        if (i != wmfocus && ind.workspace == w.workspace) {
            if ((ind.mask & SEEN_MASK) == (w.mask & SEEN_MASK))
                pre = pre == -1 ? i : pre;
            else {
                target = i;
                break;
            }
        }
    }

    if (target != -1) {
        setfocus(target);
        assert(target == wmfocus);

    } else if (pre != -1) {
        setfocus(pre);
        assert(target == wmfocus);

    } else {
        assert(wmfocus+1 >= clients.size || hashclient(&clients.clients[wmfocus]) - hashclient(&clients.clients[wmfocus]) > 1);
        return;
    }

    clients.clients[wmfocus].mask ^= SEEN_MASK;
    colourwinborder(w.window, Inactive);
    colourwinborder(clients.clients[wmfocus].window, clients.clients[wmfocus].mask & PIN_MASK ? Pinned : Default);
}

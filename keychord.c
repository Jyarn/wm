#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <X11/XKBlib.h>
#include <stdbool.h>

/*
 * Effects of Xkb on Event State
 * Because XOpenDisplay initializes Xkb, some events contain an Xkb description
 * of the keyboard state instead of that normally used by the core protocol.
 * See section 17.1.1 for more information about the differences between Xkb
 * keyboard state and that reported by the core protocol.

 * Effects of Xkb on MappingNotify Events
 * When Xkb is missing or disabled, the X library tracks changes to the keyboard
 * mapping using MappingNotify events. Whenever the keyboard mapping is changed,
 * the server sends all clients a MappingNotify event to report the change. When
 * a client receives a MappingNotify event, it is supposed to call XRefreshKeyboardMapping
 * to update the keyboard description used internally by the X library.

 * The X Keyboard Extension uses XkbMapNotify and XkbNewKeyboardNotify events to track
 * changes to the keyboard mapping. When an Xkb-aware client receives either event,
 * it should call XkbRefreshKeyboardMapping to update the keyboard description used
 * internally by the X library. To avoid duplicate events, the X server does not send core
 * protocol MappingNotify events to a client that has selected for XkbMapNotify events.

 * The implicit support for Xkb selects for XkbMapNotify events. This means that
 * clients that do not explicitly use Xkb but that are using a version of the X library
 * that has implicit support for Xkb do not receive MappingNotify events over the wire.
 * Clients that were not written with Xkb in mind do not recognize or properly handle
 * the new Xkb events, so the implicit support converts them to MappingNotify events that
 * report approximately the same information, unless the client has explicitly selected
 * for the Xkb version of the event.

 * An Xkb-capable X server does not send events from keys that fall outside the legal range
 * of keycodes expected by that client. Once the server sends a client an XkbNewKeyboardNotify
 * event, it reports events from all keys because it assumes that any client that has
 * received an XkbNewKeyboardNotify event expects key events from the new range of keycodes.
 * The implicit support for Xkb asks for XkbNewKeyboardNotify events, so the range of keycodes
 * reported to the client might vary without the client’s knowledge. Most clients don’t
 * really care about the range of legal keycodes, but some clients maintain information
 * about each key and might have problems with events that come from unexpected keys.
 * Such clients can set the XkbLC_IgnoreNewKeyboards library control (see section 11.3.1) to
 * prevent the implicit support from requesting notification of changes to the legal range of
 * keycodes.
 */


/*
 * store binds in a string since instead of storing binds using keysyms/masks because while storing
 * "<C-XF86AudioMicMute>" takes more memory, we'll probably end up storing binds like "g" more often
 * which only takes 2 bytes, if we store it as a string, compared to 16 bytes
 *
 */

#define NCHORDS (sizeof(binds) / sizeof(struct Bind))
bool start;
Display* dpy;

struct Bind {
    void (*func)(void);
    char* bind;
    int depth;
};

void b1(void) { printf("b1\n"); }
void b2(void) { printf("b2\n"); }
void b3(void) { printf("b3\n"); }
void b4(void) { printf("b4\n"); }
void b5(void) { exit(1); }

struct Bind binds[] = {
    { .func = b1, .depth = 0 },
    { .func = b2, .depth = 0 },
    { .func = b3, .depth = 0 },
    { .func = b4, .depth = 0 },
    { .func = b5, .depth = 0 },
};


/*
 * String processing love (:
 */
void
match(XKeyPressedEvent* ev)
{
    for (int i = 0; i < NCHORDS; i++) {
        if (start)
            binds[i].depth = 0;
        else if (binds[i].depth == -1)
            continue;

        int j = binds[i].depth;
        unsigned int keyMask = 0;

        while (binds[i].bind[j]) {
            switch(binds[i].bind[j+1]) {
                case '-':

                case '>':
                    break;
            }
        }
    }
}

void
createbind(const char* bind, int index)
{
    binds[index].bind = malloc(strlen(bind));
    memcpy(binds[index].bind, bind, strlen(bind));
}

/*
 * TODO
 *  [ ] refresh mapping
 *  [ ] match
 */
int
main(void)
{
    int _ev, reason, _err;
    int maj = 1; int min = 7;
    dpy = XkbOpenDisplay(NULL, &_ev, &_err, &maj, &min, &reason);
    if (!dpy) {
        switch (reason) {
            case XkbOD_BadLibraryVersion: printf("error: Bad library version: %d.%d\n", maj, min); break;
            case XkbOD_ConnectionRefused: printf("error: Connection refused\n"); break;
            case XkbOD_BadServerVersion: printf("error: Bad server\n"); break;
            case XkbOD_NonXkbServer: printf("error: no xkb (who doesn't have xkb?)\n"); break;
        }

        return 1;
    }

    printf("info: Selecting version: %d.%d\n", maj, min);
    Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
                0, 0, 640, 480, 0, 0, 0);

    XSelectInput(dpy, w, KeyPressMask);
    XMapWindow(dpy, w);
    createbind("qq", 0);
    createbind("<C-q><C-c>", 1);
    createbind("<C-q><C-q>", 2);
    createbind("q<C-q>c", 3);
    createbind("<C-c>", 4);

    XEvent ev;

    for (;;) {
        XNextEvent(dpy, &ev);
        switch (ev.type) {
            case KeyPress:
                printf("keypress\n");
                match(&ev.xkey);
                break;
        }
    }
}

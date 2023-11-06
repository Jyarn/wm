#include <X11/Xlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "debug.h"
#include "wm.h"
#include "util.h"
#include "config.h"
#include "event.h"

void exit_wm (void* args UNUSED);
void spawn (void* args);
void focus (void* args UNUSED);
void killWindow (void* args UNUSED);
void moveWindow (void* args UNUSED);
void resizeWindow (void* args UNUSED);
void minimize (void* args UNUSED);
void tabwindows (void* args UNUSED);
void toggleFloating (void* args UNUSED);
void toggleTiling (void* args UNUSED);



void
exit_wm (void* args UNUSED) {
    evt_run = false;
    return;
}

void
spawn (void* _args) {
    int i = fork ();

    if (i < 0)
        perror ("[ ERROR ]  unable to fork");
    else if (i)
        return;
    else {
        char args[strlen (_args)];
        strcpy (args, _args);
        int nSpaces = 0;
        // count number of spaces
        for (int i = 0; args[i]; i++)
            nSpaces += args[i] == ' ';

        char* pass[nSpaces+2];
        int i = 1;
        for (int j = 0; args[j]; j++) {
            if (args[j] == ' ') {
                args[j] = '\0';
                pass[i] = &args[j+1];
                i++;
            }
        }

        pass[0] = args;
        pass[nSpaces+1] = NULL;
        execvp (args, pass);
        perror ("[ ERROR ]  execvp failed");
    }

    return;
}

void
focus (void* args UNUSED) {
    dbg_log ("[ INFO ] focus window %d\n", CURRENT_WINDOW);
    Client* cl = wm_fetchClient(CURRENT_WINDOW);
    if (!cl)
        return;

    XSetInputFocus (dpy, cl->window, RevertToPointerRoot, CurrentTime);
    wm_focus = cl;
}

void
killWindow (void* args UNUSED)  {
    if (wm_focus)
        wm_killClient (wm_focus->window);

    return;
}

void
moveWindow (void* args UNUSED) {
    Client* cl = wm_fetchClient (CURRENT_WINDOW);

    if (!cl)
        return;

    XMotionEvent m;
    handler h;

    int prevX = evt_currentEvent.xbutton.x_root;
    int prevY = evt_currentEvent.xbutton.y_root;

    WM_GRABPOINTER ();
    for (;;) {
        XNextEvent (dpy, &evt_currentEvent);
        switch (evt_currentEvent.type) {
            case ButtonPress:
            case KeyPress:
                break;
            case MotionNotify:
                m = evt_currentEvent.xmotion;
                wm_changeGeomRelative (cl,
                                       m.x_root - prevX,
                                       m.y_root - prevY,
                                       0, 0);
                prevX = m.x_root;
                prevY = m.y_root;
                break;
            case ButtonRelease:
                goto EXIT;
            default:
                if ((h = evt_handlers[evt_currentEvent.type]))
                    h ();
        }
    }

EXIT:
    WM_UNGRABPOINTER ();
}

void
resizeWindow (void* args UNUSED) {
    XButtonPressedEvent ev = evt_currentEvent.xbutton;
    Client* cl = wm_fetchClient (ev.window);
    if (!cl)
        return;

    const bool ulx = ev.x*2 < cl->w;
    const bool uly = ev.y*2 < cl->h;

    int prevX = ulx ? cl->x : cl->x + cl->w;
    int prevY = uly ? cl->y : cl->y + cl->h;

    XWarpPointer (dpy, defaultScreen.root, None, 0, 0, 0, 0, prevX - ev.x_root, prevY - ev.y_root);

    XMotionEvent mov;
    handler h;
    WM_GRABPOINTER ();
    for (;;) {
        XNextEvent (dpy, &evt_currentEvent);
        switch (evt_currentEvent.type) {
            case ButtonPress:
            case KeyPress:
                break;
            case MotionNotify:
                mov = evt_currentEvent.xmotion;
                if ((cl->x < mov.x || ulx) && (mov.x < cl->x+cl->w || !ulx)) {
                    if (ulx)
                        wm_changeGeomRelative (cl, mov.x_root-prevX, 0, prevX-mov.x_root, 0);
                    else
                        wm_changeGeomRelative (cl, 0, 0, mov.x_root-prevX, 0);
                }

                if ((cl->y < mov.y || uly) && (mov.y < cl->y+cl->h || !uly)) {
                    if (uly)
                        wm_changeGeomRelative (cl, 0, mov.y_root-prevY, 0, prevY-mov.y_root);
                    else
                        wm_changeGeomRelative (cl, 0, 0, 0, mov.y_root-prevY);
                }

                prevX = mov.x_root;
                prevY = mov.y_root;
                usleep (1000); // sleep for a bit to avoid calling XMoveResize so much
                break;
            case ButtonRelease:
                goto EXIT;
            default:
                h = evt_handlers[evt_currentEvent.type];
                if (h)
                    h ();
                break;
        }
    }

EXIT:
    WM_UNGRABPOINTER ();
    return;
}

void
minimize (void* args UNUSED) {
    Client* cl = wm_fetchClient (CURRENT_WINDOW);
    if (!cl)
        return;

    wm_minimize (cl);
    wm_focusNext (false);
}

void
tabwindows (void* args UNUSED) {
    wm_focusNext (true);
}


const keyChord keyBinds[N_KEY_BINDS] = {
    { .modifier = Mod4Mask | ShiftMask  , .key = "e"                    , .cmd = exit_wm    , .args = NULL},
    { .modifier = Mod4Mask              , .key = "Return"               , .cmd = spawn      , .args = "alacritty"},
    { .modifier = Mod4Mask              , .key = "d"                    , .cmd = spawn      , .args = "dmenu_run"},
    { .modifier = AnyModifier           , .key = "XF86AudioRaiseVolume" , .cmd = spawn      , .args = "pactl set-sink-volume @DEFAULT_SINK@ +5%"},
    { .modifier = AnyModifier           , .key = "XF86AudioLowerVolume" , .cmd = spawn      , .args = "pactl set-sink-volume @DEFAULT_SINK@ -5%"},
    { .modifier = AnyModifier           , .key = "XF86AudioMute"        , .cmd = spawn      , .args = "pactl set-source-mute @DEFAULT_SOURCE@ toggle"},
    { .modifier = Mod4Mask | ShiftMask  , .key = "q"                    , .cmd = killWindow , .args = NULL},
    { .modifier = Mod4Mask | ShiftMask  , .key = "Return"               , .cmd = spawn      , .args = "alacritty -e bash -c CAD.sh"},
    { .modifier = Mod4Mask              , .key = "q"                    , .cmd = minimize   , .args = NULL},
    { .modifier = Mod4Mask              , .key = "Tab"                  , .cmd = tabwindows , .args = NULL},
};

const mouseBind mouseBinds[N_MOUSE_BINDS] = {
    { .modifier = NOMODIFIER            , .buttons = Button1, .cmd = focus          , .args = NULL },
    { .modifier = Mod4Mask              , .buttons = Button1, .cmd = moveWindow     , .args = NULL },
    { .modifier = Mod4Mask | ShiftMask  , .buttons = Button1, .cmd = resizeWindow   , .args = NULL }
};
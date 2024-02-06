#include <X11/Xlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "debug.h"
#include "wm.h"
#include "util.h"
#include "config.h"
#include "event.h"
#include "tl.h"


void
exit_wm (Arg args UNUSED) {
    evt_run = false;
    return;
}

void
spawn (Arg _args) {
    int i = fork ();

    if (i < 0)
        perror ("[ ERROR ]  unable to fork");
    else if (i)
        return;
    else {
        char args[strlen (_args.str)];
        strcpy (args, _args.str);
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
focus (Arg args UNUSED) {
    dbg_log ("[ INFO ] focus window %d\n", CURRENT_WINDOW);
    Client* cl = wm_fetchClient(CURRENT_WINDOW);
    if (!cl)
        return;

    XSetInputFocus (dpy, cl->window, RevertToPointerRoot, CurrentTime);
    wm_focus = cl;
}

void
killWindow (Arg args UNUSED)  {
    if (wm_focus)
        wm_killClient (wm_focus->window);

    return;
}

void
moveWindow (Arg args UNUSED) {
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
resizeWindow (Arg args UNUSED) {
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
minimize (Arg args UNUSED) {
    Client* cl = wm_fetchClient (CURRENT_WINDOW);
    if (!cl)
        return;

    wm_minimize (cl);
    wm_focusNext (false);
}

void
tabwindows (Arg args UNUSED) {
    wm_focusNext (true);
}

void
switchworkspace (Arg args)
{
    if (args.ui == workspacenum)
        return;

    Client* cur = activeClients;
    dbg_log ("[ INFO ] switching to workspace %d\n", args.ui);

    while (cur) {
        if (cur->workspace == args.ui && !cur->minimized)
            XMoveWindow (dpy, cur->window, cur->x, cur->y);
        else if (!cur->minimized && cur->workspace == workspacenum && cur->workspace != WORKSPACE_ALWAYSON) {
            XMoveWindow (dpy, cur->window, -3840, -2160); // move to the shadow zone
        }
        cur = cur->next;
    }

    workspacenum = args.ui;
    if (wm_focus->workspace != WORKSPACE_ALWAYSON)
        wm_focusNext (false);
}

void
movetoworkspace (Arg args) {
    Client* cl = wm_fetchClient (CURRENT_WINDOW);
    if (args.ui == workspacenum || cl == NULL)
        return;
    assert (!cl->minimized);

    cl->workspace = args.ui;
    ignorenextunmap = true;
    if (args.ui != WORKSPACE_ALWAYSON) {
        XMoveWindow (dpy, cl->window, -3840, -2160);
        wm_focusNext (false);
    }
}

void
toggleFloating (Arg args UNUSED) {
    tl_remove (wm_focus);
}

void
toggleTiling (Arg args UNUSED) {
    tl_manage (wm_focus);
}

void
incStack (Arg args UNUSED) {
    tl_nMasterIncDec (1);
}

void
decStack (Arg args UNUSED) {
    tl_nMasterIncDec (-1);
}

const keyChord keyBinds[N_KEY_BINDS] = {
    { .modifier = Mod4Mask | ShiftMask  , .key = "e"                    , .cmd = exit_wm        , .args.vp = NULL },
    { .modifier = Mod4Mask              , .key = "Return"               , .cmd = spawn          , .args.str= "alacritty"},
    { .modifier = Mod4Mask              , .key = "d"                    , .cmd = spawn          , .args.str= "dmenu_run"},
    { .modifier = AnyModifier           , .key = "XF86AudioRaiseVolume" , .cmd = spawn          , .args.str= "pactl set-sink-volume @DEFAULT_SINK@ +5%"},
    { .modifier = AnyModifier           , .key = "XF86AudioLowerVolume" , .cmd = spawn          , .args.str= "pactl set-sink-volume @DEFAULT_SINK@ -5%"},
    { .modifier = AnyModifier           , .key = "XF86AudioMute"        , .cmd = spawn          , .args.str= "pactl set-source-mute @DEFAULT_SOURCE@ toggle"},
    { .modifier = Mod4Mask | ShiftMask  , .key = "q"                    , .cmd = killWindow     , .args.vp = NULL},
    { .modifier = Mod4Mask | ShiftMask  , .key = "Return"               , .cmd = spawn          , .args.str= "alacritty -e bash -c CAD.sh"},
    { .modifier = Mod4Mask              , .key = "q"                    , .cmd = minimize       , .args.vp = NULL},
    { .modifier = Mod4Mask              , .key = "Tab"                  , .cmd = tabwindows     , .args.vp = NULL},
    { .modifier = Mod4Mask              , .key = "0"                    , .cmd = switchworkspace, .args.ui = 0},
    { .modifier = Mod4Mask              , .key = "1"                    , .cmd = switchworkspace, .args.ui = 1 },
    { .modifier = Mod4Mask              , .key = "2"                    , .cmd = switchworkspace, .args.ui = 2 },
    { .modifier = Mod4Mask              , .key = "3"                    , .cmd = switchworkspace, .args.ui = 3 },
    { .modifier = Mod4Mask              , .key = "4"                    , .cmd = switchworkspace, .args.ui = 4 },
    { .modifier = Mod4Mask              , .key = "5"                    , .cmd = switchworkspace, .args.ui = 5 },
    { .modifier = Mod4Mask              , .key = "6"                    , .cmd = switchworkspace, .args.ui = 6 },
    { .modifier = Mod4Mask              , .key = "7"                    , .cmd = switchworkspace, .args.ui = 7 },
    { .modifier = Mod4Mask              , .key = "8"                    , .cmd = switchworkspace, .args.ui = 8 },
    { .modifier = Mod4Mask              , .key = "9"                    , .cmd = switchworkspace, .args.ui = 9 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "0"                    , .cmd = movetoworkspace, .args.ui = 0 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "1"                    , .cmd = movetoworkspace, .args.ui = 1 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "2"                    , .cmd = movetoworkspace, .args.ui = 2 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "3"                    , .cmd = movetoworkspace, .args.ui = 3 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "4"                    , .cmd = movetoworkspace, .args.ui = 4 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "5"                    , .cmd = movetoworkspace, .args.ui = 5 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "6"                    , .cmd = movetoworkspace, .args.ui = 6 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "7"                    , .cmd = movetoworkspace, .args.ui = 7 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "8"                    , .cmd = movetoworkspace, .args.ui = 8 },
    { .modifier = Mod4Mask | ShiftMask  , .key = "9"                    , .cmd = movetoworkspace, .args.ui = 9 },
    { .modifier = Mod4Mask              , .key = "w"                    , .cmd = toggleTiling   , .args.vp = NULL },
    { .modifier = Mod4Mask              , .key = "e"                    , .cmd = toggleFloating , .args.vp = NULL },
    { .modifier = Mod4Mask              , .key = "i"                    , .cmd = incStack       , .args.vp = NULL },
    { .modifier = Mod4Mask              , .key = "o"                    , .cmd = decStack       , .args.vp = NULL }
};

const mouseBind mouseBinds[N_MOUSE_BINDS] = {
    { .modifier = NOMODIFIER            , .buttons = Button1, .cmd = focus          , .args.vp = NULL },
    { .modifier = Mod4Mask              , .buttons = Button1, .cmd = moveWindow     , .args.vp = NULL },
    { .modifier = Mod4Mask | ShiftMask  , .buttons = Button1, .cmd = resizeWindow   , .args.vp = NULL }
};

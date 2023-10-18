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

void exit_wm (void* args UNUSED) {
    evt_run = false;
    return;
}

void spawn (void* _args) {
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

void focus (void* args UNUSED) {
    dbg_log ("[ INFO ] focus window %d\n", evt_currentEvent.xbutton.window);
    wm_setFocus (evt_currentEvent.xbutton.window);
    return;
}

void killWindow (void* args UNUSED)  {
    if (wm_focus)
        wm_killClient (wm_focus->window);

    return;
}

void moveWindow (void* args UNUSED) {
    Client* cl = wm_fetchClient (evt_currentEvent.xbutton.window);

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
                wm_moveWindow (cl,
                               cl->x + m.x_root - prevX,
                               cl->y + m.y_root - prevY);
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

void resizeWindow (void* args UNUSED) {
    return;
}

const keyChord keyBinds[] = {
    {.modifier = Mod4Mask | ShiftMask, .key = "e"                   ,   .cmd = exit_wm   , .args = NULL},
    {.modifier = Mod4Mask | ShiftMask, .key = "Return"              ,   .cmd = spawn     , .args = "alacritty"},
    {.modifier = Mod4Mask            , .key = "d"                   ,   .cmd = spawn     , .args = "dmenu_run"},
    {.modifier = AnyModifier         , .key = "XF86AudioRaiseVolume",   .cmd = spawn     , .args = "pactl set-sink-volume @DEFAULT_SINK@ +5%"},
    {.modifier = AnyModifier         , .key = "XF86AudioLowerVolume",   .cmd = spawn     , .args = "pactl set-sink-volume @DEFAULT_SINK@ -5%"},
    {.modifier = AnyModifier         , .key = "XF86AudioMute"       ,   .cmd = spawn     , .args = "pactl set-source-mute @DEFAULT_SOURCE@ toggle"},
    {.modifier = Mod4Mask | ShiftMask, .key = "q"                   ,   .cmd = killWindow, .args = NULL}
};

const mouseBind mouseBinds[] = {
    { .modifier = NOMODIFIER, .buttons = Button1, .cmd = focus          , .args = NULL },
    { .modifier = Mod4Mask  , .buttons = Button1, .cmd = moveWindow     , .args = NULL },
    { .modifier = Mod4Mask  , .buttons = Button1, .cmd = resizeWindow   , .args = NULL }
};
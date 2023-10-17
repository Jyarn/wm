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


bool exit_wm (void* args UNUSED, Window w UNUSED) {
    return false;
}

bool spawn (void* _args, Window w UNUSED) {
    int i = fork ();

    if (i < 0)
        perror ("[ ERROR ]  unable to fork");
    else if (i)
        return true;
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

    return false;
}

bool focus (void* args UNUSED, Window w) {
    dbg_log ("[ INFO ] focus window %d\n", w);
    wm_setFocus (w);
    return true;
}

bool killWindow (void* args UNUSED, Window w UNUSED)  {
    if (wm_focus)
        wm_killClient (wm_focus->window);

    return true;
}

bool moveWindow (void* args UNUSED, MotionEvent* evt) {
    if (evt->firstCall)
        return true;

    Client* cl = wm_fetchClient (evt->w);
    if (cl != NULL)
        wm_moveWindow (cl, cl->x + evt->x - evt->prevX, cl->y + evt->y - evt->prevY);
    return true;
}

bool resizeWindow (void* args UNUSED, MotionEvent* evt) {
    Client* cl = wm_fetchClient (evt->w);
    if (!cl)
        return true;

    static unsigned int wX;
    static unsigned int wY;
    if (evt->firstCall) {
        wX = (evt->x*2 > cl->x + cl->w) ? cl->w+cl->x : (unsigned int)cl->x;
        wY = (evt->y*2 > cl->y + cl->h) ? cl->h+cl->y : (unsigned int)cl->y;
        evt->prevX = wX;
        evt->prevY = wY;

        XWarpPointer (dpy, defaultScreen.root, None, 0, 0, 0, 0, wX-evt->x, wY-evt->y);
        return true;
    }
    else {
        wX += evt->x-evt->prevX;
        wY += evt->y-evt->prevY;
        cl->w = wX;
        cl->h = wY;
        XResizeWindow (dpy, evt->w, wX, wY);
    }

    return true;
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
    {.modifier = NOMODIFIER, .buttons = Button1, .cmd = focus  , .args = NULL }
};

const moveBind moveBinds[] = {
    {.modifier = Mod4Mask            , .buttons = Button1, .cmd = moveWindow  , .args = NULL},
    {.modifier = Mod4Mask | ShiftMask, .buttons = Button1, .cmd = resizeWindow, .args = NULL}
};
#include <X11/Xlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "util.h"
#include "config.h"


bool exit_wm (void* args UNUSED) {
    return false;
}

bool spawn (void* _args) {
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
    }

    return false;
}


const keyChord keyBinds[] = {
    {.modifier = Mod4Mask | ShiftMask, .key = "e"                   ,   .cmd = exit_wm   , .args = NULL},
    {.modifier = Mod4Mask | ShiftMask, .key = "Return"              ,   .cmd = spawn     , .args = "alacritty"},
    {.modifier = Mod4Mask            , .key = "d"                   ,   .cmd = spawn     , .args = "dmenu_run"},
    {.modifier = AnyModifier         , .key = "XF86AudioRaiseVolume",   .cmd = spawn     , .args = "pactl set-sink-volume @DEFAULT_SINK@ +5%"},
    {.modifier = AnyModifier         , .key = "XF86AudioLowerVolume",   .cmd = spawn     , .args = "pactl set-sink-volume @DEFAULT_SINK@ -5%"},
    {.modifier = AnyModifier         , .key = "XF86AudioMute"       ,   .cmd = spawn     , .args = "pactl set-source-mute @DEFAULT_SOURCE@ toggle"}
};

const mouseBind mouseBinds[] = {

};
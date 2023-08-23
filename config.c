#include <X11/Xlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include "config.h"

#define UNUSED __attribute__ ((unused))


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
        char* args = (char*)_args;
        int nSpaces;
        // count number of spaces
        for (nSpaces = 0; !args[nSpaces]; nSpaces++) {}

        char* pass[nSpaces+1];
        int i = 0;
        for (int j = 0; !args[j]; j++) {
            if (args[j] == ' ') {
                args[j] = '\0';
                pass[i] = &args[j+1];
                i++;
                assert (i < nSpaces);
            }
        }

        pass[nSpaces] = NULL;
        execvp (args, pass);
    }

    return false;
}


const keyChord keyBinds[] = {
    {.modifier = Mod4Mask | ShiftMask, .key = "e", .cmd = exit_wm   , .args = NULL},
    {.modifier = Mod4Mask | ShiftMask, .key = "f", .cmd = spawn     , .args = "alacritty"},
    {.modifier = Mod4Mask            , .key = "d", .cmd = spawn     , .args = "dmenu_run"}
};
#ifndef __CONFIG__H
#define __CONFIG__H

#include <X11/Xlib.h>
#include <stdbool.h>

#define N_KEY_BINDS 6
#define N_MOUSE_BINDS 1

/*
 * Specify length of chords (excluding modifiers).
 * Ex.
 *  Ctrl-f       = length 1
 *  f             = length 1
 *  Ctrl-Alt-f    = length 2
*/

typedef bool (*cmd)(void*, Window);

typedef struct {
    unsigned int modifier;
    char* key;
    cmd cmd;
    void* args;
} keyChord;

typedef struct {
    unsigned int modifier;
    unsigned int buttons;
    bool isDragged;             // Should the mouse be dragged
    cmd cmd;
    void* args;
} mouseBind;

extern const keyChord keyBinds[];
extern const mouseBind mouseBinds[];

#endif
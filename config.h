#ifndef __CONFIG__H
#define __CONFIG__H

#include <stdbool.h>

#define N_KEY_BINDS 6
#define N_MOUSE_BINDS 0

/*
 * Specify length of chords (excluding modifiers).
 * Ex.
 *  Ctrl-f       = length 1
 *  f             = length 1
 *  Ctrl-Alt-f    = length 2
*/

typedef bool (*cmd)(void*);

typedef struct {
    unsigned int modifier;
    char* key;
    cmd cmd;
    void* args;
} keyChord;

typedef enum {
    ClickedOnWindow,
    ClickOnRoot,
    ClickedOnBar
} clickLocation;

typedef struct {
    clickLocation where;        // Where the mouse should be to activate
    bool isDragged;             // Should the mouse be dragged
    unsigned int modifier;
    unsigned int buttons;
    cmd cmd;
    void* args;
} mouseBind;

extern const keyChord keyBinds[];
extern const mouseBind mouseBinds[];

#endif
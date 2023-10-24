#ifndef __CONFIG__H
#define __CONFIG__H

#include <X11/Xlib.h>
#include <stdbool.h>

#include "event.h"

#define N_KEY_BINDS 8
#define N_MOUSE_BINDS 3
#define NOMODIFIER 0

typedef void (*cmd)(void*);

typedef struct {
    unsigned int modifier;
    char* key;
    cmd cmd;
    void* args;
} keyChord;

typedef struct {
    unsigned int modifier;
    unsigned int buttons;
    cmd cmd;
    void* args;
} mouseBind;

extern const keyChord keyBinds[];
extern const mouseBind mouseBinds[];

#endif
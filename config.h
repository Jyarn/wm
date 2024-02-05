#ifndef __CONFIG__H
#define __CONFIG__H

#include <X11/Xlib.h>
#include <stdbool.h>

#include "event.h"

#define N_KEY_BINDS 30
#define N_MOUSE_BINDS 3
#define NOMODIFIER 0
#define BORDERWIDTH 3
#define WORKSPACE_ALWAYSON 0

typedef union {
    unsigned int ui;
    void* vp;
    char c;
    char* str;
} Arg;

typedef void (*cmd)(Arg);

typedef struct {
    unsigned int modifier;
    char* key;
    cmd cmd;
    Arg args;
} keyChord;

typedef struct {
    unsigned int modifier;
    unsigned int buttons;
    cmd cmd;
    Arg args;
} mouseBind;


extern const keyChord keyBinds[];
extern const mouseBind mouseBinds[];

#endif

#ifndef __CONFIG__H
#define __CONFIG__H

#include <X11/Xlib.h>
#include <stdbool.h>

#include "event.h"
#include "wm.h"

#define N_KEY_BINDS 36
#define N_MOUSE_BINDS 3
#define NOMODIFIER 0
#define BORDERWIDTH 3
#define WORKSPACE_ALWAYSON 0
#define NMON 2
#define SNAPDIST 30
#define CURRENTMON(cl) (monitors[cl->monnum])

typedef union {
    unsigned int ui;
    void* vp;
    char c;
    char* str;
    bool b;
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


extern const Monitor monitors[NMON];
extern const keyChord keyBinds[N_KEY_BINDS];
extern const mouseBind mouseBinds[N_MOUSE_BINDS];

#endif

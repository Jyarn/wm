#ifndef __CONFIG__H
#define __CONFIG__H

#include <X11/Xlib.h>
#include <stdbool.h>

#include "event.h"

#define N_KEY_BINDS 7
#define N_MOUSE_BINDS 1
#define N_MOVE_BINDS 2
#define NOMODIFIER 0

typedef bool (*cmd)(void*, Window);
typedef bool (*pointerFunc)(void*, MotionEvent*);

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

typedef struct {
    unsigned int modifier;
    unsigned int buttons;
    pointerFunc cmd;
    void* args;
} moveBind;

extern const keyChord keyBinds[];
extern const mouseBind mouseBinds[];
extern const moveBind moveBinds[];

#endif
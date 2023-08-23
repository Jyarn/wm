#ifndef __CONFIG__H
#define __CONFIG__H

#include <stdbool.h>

#define N_KEY_BINDS 3

/*
 * Specify length of chords (excluding modifiers).
 * Ex.
 *  Ctrl-f       = length 1
 * f             = length 1
 * Ctrl-Alt-f    = length 2
*/

typedef bool (*cmd)(void*);

typedef struct {
    unsigned int modifier;
    char* key;
    cmd cmd;
    void* args;
} keyChord;

extern const keyChord keyBinds[];

#endif
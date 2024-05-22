#include <X11/Xlib.h>

#ifndef __UTIL__H
#define __UTIL__H

#define __DEBUG

#define UNUSED __attribute__ ((unused))
#define STR_TO_KEYSYM(a) XKeysymToKeycode (dpy, XStringToKeysym (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(a, b, c) (MIN(a, MAX(b, c)))

int catstr(void* v, char** b);

#endif

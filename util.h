#include <X11/Xlib.h>

#define UNUSED __attribute__ ((unused))
#define STR_TO_KEYSYM(a) XKeysymToKeycode (dpy, XStringToKeysym (a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

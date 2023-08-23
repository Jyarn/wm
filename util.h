#include <X11/Xlib.h>
#define STR_TO_KEYSYM(a) XKeysymToKeycode (dpy, XStringToKeysym (a))

/*
 * Remove CAPS and num lock from mask
*/
#define ALLOWMASK(a) (a & (Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask | ShiftMask))
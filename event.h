#ifndef __EVENT__H
#define __EVENT__H

typedef bool (*handler)(XEvent*);
void evt_eventHandler (void);

#endif
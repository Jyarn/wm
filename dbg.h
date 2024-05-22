#ifndef __DEBUG__H
#define __DEBUG__H

#include "util.h"

#define BUFFERSZ 1024
#ifdef __DEBUG
void dbg_log(const char* str, ...);
#endif

#ifndef __DEBUG
#define dbg_log(str, ...)
#endif

void inithandler(void);
void handlerOn(void);
void handlerOff(void);

#endif

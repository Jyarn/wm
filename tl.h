#pragma once

#ifndef __TILING__H
#define __TILING__H

#include "wm.h"

void tl_manage (Client* cl);
void tl_nMasterIncDec (int a);
void tl_remove (Client* cl);

#endif
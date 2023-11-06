#include "wm.h"

// linked list of Clients being tiled
static Client* tilers = NULL;




void
tl_manage (Client* cl) {
    cl->tlNext = tilers;
    tilers = cl;
}

void
tl_remove (Client* cl) {
    Client* cur = tilers;
    Client* prev = NULL;

    while (cur) {
        if (cur == cl) {
            prev = cur->next;
            return;
        }

        prev = cur;
        cur = cur->next;
    }
}
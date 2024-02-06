#include "tl.h"
#include "wm.h"
#include "debug.h"

static int nMaster = 1;

void tl_restack (void);

void
tl_nMasterIncDec (int a) {
    nMaster += a;
    tl_restack ();
    dbg_log ("[ INFO ] nmaster=%d\n", nMaster);
}

void
tl_manage (Client* cl) {
    if (cl->type != floating)
        return;

    if (nMaster > 0) {
        nMaster--;
        cl->type = master;
        dbg_log ("[ INFO ] (%d) added to master (nmaster = %d)\n", cl->window, nMaster);
        return;
    }

    dbg_log ("[ INFO ] (%d) added to slave\n", cl->window);
    cl->type = slave;
}

void
tl_remove (Client* cl) {
    if (cl->type == floating)
        return;
    else if (cl->type == master) {
        nMaster++;
        tl_restack ();
    }
    cl->type = floating;
    dbg_log ("[ INFO ] (%d) removed (nmaster = %d)\n", cl->window, nMaster);
}


/*
 * optimize for inc/decs = 1
*/
void
tl_restack (void) {
    if (nMaster >= 0) {
        for (Client* c = activeClients; c && nMaster > 0; c = c->next)
            if (c->type == slave) {
                c->type = master;
                nMaster--;
                dbg_log ("[ INFO ] (%d) added to master (nmaster = %d)\n", c->window, nMaster);
            }
    }
    else {
        for (Client* c = activeClients; c && nMaster < 0; c = c->next)
            if (c->type == master) {
                c->type = slave;
                nMaster++;
                dbg_log ("[ INFO ] (%d) added to slave (nmaster = %d)\n", c->window, nMaster);
            }
    }

    if (nMaster < 0)
        nMaster = 0;
}
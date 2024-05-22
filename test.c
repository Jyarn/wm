#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "wm.h"
#include "dbg.h"
#include "util.h"

#ifndef __DEBUG
#error "__DEBUG must be enabled"
#endif

/*
 * tests for things that don't use xlib
 */

#define TEST(a, str) {                          \
    if (a)                                      \
        printf("\x1b[32m(P) %s\x1b[0m\n", str);                \
    else                                        \
        printf("\x1b[31m(F) %s@%d\x1b[0m\n", str, __LINE__);   \
}                                               \

bool
clchk(int ids[])
{
    for (int i = 0; i < clients.size; i++)
        if (clients.clients[i].id != ids[i])
            return false;
    return true;

}

int
main(void)
{
    clients.size = 0;
    clients.capacity = 0;
    clients.clients = NULL;
    Client c;
    c.mask = 0;
    c.window = 0;
    c.monitor = 0;
    c.workspace = 0;

    {
        insert(&c, 0);
        int ids[] = { 0 };
        TEST(clchk(ids), "[0]");
    }
    {
        insert(&c, 0);
        int ids[] = { 1, 0 };
        TEST(clchk(ids), "[1, 0]");
    }
    {
        insert(&c, 0);
        int ids[] = { 2, 1, 0 };
        TEST(clchk(ids), "[2, 1, 0]");
    }
    {
        insert(&c, 3);
        int ids[] = { 2, 1, 0, 3 };
        TEST(clchk(ids), "[2, 1, 0, 3]");
    }
    {
        insert(&c, 4);
        int ids[] = { 2, 1, 0, 3, 4 };
        TEST(clchk(ids), "[2, 1, 0, 3, 4]");
    }
    {
        insert(&c, 5);
        int ids[] = { 2, 1, 0, 3, 4, 5 };
        TEST(clchk(ids), "[2, 1, 0, 3, 4, 5]");
    }
    {
        insert(&c, 2);
        int ids[] = { 2, 1, 6, 0, 3, 4, 5 };
        TEST(clchk(ids), "[2, 1, 6, 0, 3, 4, 5]");
    }
    {
        insert(&c, 1);
        int ids[] = { 2, 7, 1, 6, 0, 3, 4, 5 };
        TEST(clchk(ids), "[2, 7, 1, 6, 0, 3, 4, 5]");
    }
    {
        insert(&c, 4);
        int ids[] = { 2, 7, 1, 6, 8, 0, 3, 4, 5 };
        TEST(clchk(ids), "[2, 7, 1, 8, 4, 0, 3, 4, 5]");
    }

    printf("\n");
    {
        delete(0);
        int ids[] = { 7, 1, 6, 8, 0, 3, 4, 5 };
        TEST(clchk(ids), "[7, 1, 6, 8, 0, 3, 4, 5]");
    }
    {
        delete(0);
        int ids[] = { 1, 6, 8, 0, 3, 4, 5 };
        TEST(clchk(ids), "[1, 6, 8, 0, 3, 4, 5]");
    }
    {
        delete(0);
        int ids[] = { 6, 8, 0, 3, 4, 5 };
        TEST(clchk(ids), "[6, 8, 0, 3, 4, 5]");
    }
    {
        delete(3);
        int ids[] = { 6, 8, 0, 4, 5 };
        TEST(clchk(ids), "[6, 8, 0, 4, 5]");
    }
    {
        delete(3);
        int ids[] = { 6, 8, 0, 5 };
        TEST(clchk(ids), "[6, 8, 0, 5]");
    }
    {
        delete(3);
        int ids[] = { 6, 8, 0 };
        TEST(clchk(ids), "[6, 8, 0]");
    }
    {
        delete(1);
        int ids[] = { 6, 0 };
        TEST(clchk(ids), "[6, 0]");
    }
    {
        delete(1);
        int ids[] = { 6 };
        TEST(clchk(ids), "[6]");
    }

    delete(0);
    TEST(clients.size == 0, "size = 0");
    TEST(clients.capacity == 0, "capacity = 0");
    printf("\n");

    insert(&c, clients.size);
    insert(&c, clients.size);
    insert(&c, clients.size);
    insert(&c, clients.size);
    insert(&c, clients.size);
    {
        shiftright(0);
        int ids[] = { -1, 9, 10, 11, 12, 13 };
        TEST(clchk(ids), "[-1, 9, 10, 11, 12, 13]");
    }
    {
        shiftright(clients.size);
        int ids[] = { -1, 9, 10, 11, 12, 13, -1 };
        TEST(clchk(ids), "[-1, 9, 10, 11, 12, 13, -1]");
    }
    {
        shiftright(3);
        int ids[] = { -1, 9, 10, -1, 11, 12, 13, -1 };
        TEST(clchk(ids), "[-1, 9, 10, -1, 11, 12, 13, -1]");
    }

    printf("\n");

    {
        shiftleft(0);
        shiftleft(2);
        shiftleft(5);
        int ids[] = { 9, 10, 11, 12, 13 };
        TEST(clchk(ids), "[9, 10, 11, 12, 13]");
    }
    {
        clients.clients[0].id = -1;
        shiftleft(0);
        int ids[] = { 10, 11, 12, 13 };
        TEST(clchk(ids), "[10, 11, 12, 13]");
    }
    {
        clients.clients[3].id = -1;
        shiftleft(3);
        int ids[] = { 10, 11, 12 };
        TEST(clchk(ids), "[10, 11, 12]");
    }
    {
        clients.clients[1].id = -1;
        shiftleft(1);
        int ids[] = { 10, 12 };
        TEST(clchk(ids), "[10, 12]");
    }
    {
        shiftright(0);
        shiftright(clients.size);
        shiftright(2);
        int ids[] = { -1, 10, -1, 12, -1 };
        TEST(clchk(ids), "[-1, 10, -1, 12, -1]");
    }

    printf("\n");
    dbg_log("%s\n", "test");

    free(clients.clients);
}

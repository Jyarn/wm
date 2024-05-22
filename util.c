#include <stdlib.h>
#include <assert.h>

#include "util.h"

int
catstr(void* v, char** b)
{
    char** strs = (char**)v;

    int count = 0;
    int i = 0; int j = 0;
    while (strs[j]) {
        if (strs[j][i] == '\0') {
            j++;
            i = 0;
            continue;
        }

        count++;
        i++;
    }

    assert(i == 0);

    *b = malloc(count+2);
    i = 0; j = 0; int k = 0;
    while (strs[j]) {
        if (strs[j][i] == '\0') {
            j++;
            i = 0;
            continue;
        }

        (*b)[k++] = strs[j][i++];
    }

    assert(count == k);
    (*b)[count++] = '\n';
    (*b)[count++] = '\0';
    return count;
}

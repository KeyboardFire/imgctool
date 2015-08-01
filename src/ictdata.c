#include "ictdata.h"

#include <string.h>

int addCh(char** s, char ch, int bufLen) {
    size_t len = strlen(*s);
    if (len + 1 >= bufLen) {
        // allocate more memory
        *s = realloc(*s, bufLen + BUF_ADD_SIZE);
        memset((*s) + bufLen, '\0', BUF_ADD_SIZE);
        bufLen += BUF_ADD_SIZE;
    }
    (*s)[len] = ch;
    return bufLen;
}

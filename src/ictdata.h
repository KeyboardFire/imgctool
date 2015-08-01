#ifndef __ICTDATA_H__
#define __ICTDATA_H__

#include <stdlib.h>

// used for C-string buffers
static const int BUF_ADD_SIZE = 10;

// utility to add character to C-string, return buffer size of resulting string
int addCh(char** s, char ch, int bufLen);

extern struct ictFile {
    char* filename;
    long long data;
}* files;
extern size_t nFiles;

extern struct ictCategory {
    char* name;
    char** chkboxes;
    size_t nChkboxes;
}* categories;
extern size_t nCategories;

#endif

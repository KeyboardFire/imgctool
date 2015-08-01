#ifndef __ICTDATA_H__
#define __ICTDATA_H__

#include <stdlib.h>

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

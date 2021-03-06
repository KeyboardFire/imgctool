#include "saverestore.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ictdata.h"

static const char SAVE_FILE[] = ".imgctool";

// utility methods
static int bitsToChars(int bits);

// restore() sub-methods
static int restoreHeader(FILE* f);
static int restoreCategories(FILE* f);
static int restoreFilenames(FILE* f);
static int restoreFileData(FILE* f);

// File format:
// header: 4 bytes, 0x89 "ICT"
// ([categoryname](0x31[chkboxname])+0x30)*
// 0x30
// filenames, separated by 0x31, with an 0x30 at the end
// file data, each in the least amount of bytes to fit (total number of
//   checkboxes) bits

// this is a little ugly
// (... it's pretty bad)
#define ERR_READ() do { fprintf(stderr, "error reading file %s\n", SAVE_FILE); fclose(f); return 1; } while (0)
#define ERR_WRITE() do { fprintf(stderr, "error writing to file %s\n", SAVE_FILE); fclose(f); return 1; } while (0)
#define ERR_HEADER() do { fprintf(stderr, "file %s corrupted? (invalid header)\n", SAVE_FILE); fclose(f); return 1; } while (0)
#define ERR_TERM() do { fprintf(stderr, "file %s terminated prematurely?\n", SAVE_FILE); fclose(f); return 1; } while (0)
#define ERR_TRAIL() do { fprintf(stderr, "trailing data in %s?\n", SAVE_FILE); fclose(f); return 1; } while (0)
#define ERR_ZERO() do { fprintf(stderr, "zero length name in %s?\n", SAVE_FILE); fclose(f); return 1; } while (0)

int save() {
    FILE* f = fopen(SAVE_FILE, "wb");
    if (f == NULL) ERR_WRITE();

    // write header
    fwrite("\x89ICT", sizeof(char), 4, f);

    // write categories and checkboxes
    int i, j;
    for (i = 0; i < nCategories; ++i) {
        fwrite(categories[i].name, sizeof(char),
            strlen(categories[i].name), f);
        for (j = 0; j < categories[i].nChkboxes; ++j) {
            fputc('\x31', f);
            fwrite(categories[i].chkboxes[j], sizeof(char),
                strlen(categories[i].chkboxes[j]), f);
        }
        fputc('\x30', f);
    }
    fputc('\x30', f);

    // write filenames
    if (nFiles >= 1) {
        fwrite(files[0].filename, sizeof(char), strlen(files[0].filename), f);
    }
    for (i = 1; i < nFiles; ++i) {
        fputc('\x31', f);
        fwrite(files[i].filename, sizeof(char), strlen(files[i].filename), f);
    }
    fputc('\x30', f);

    // write file data
    int chkboxCount = 0;
    for (i = 0; i < nCategories; ++i) chkboxCount += categories[i].nChkboxes;
    int chars = bitsToChars(chkboxCount);
    for (i = 0; i < nFiles; ++i) {
        for (j = chars - 1; j >= 0; --j) {
            fputc((char)((files[i].data >> (j*8)) & 0xFF), f);
        }
    }

    fclose(f);
    return 0;
}

int restore() {
    FILE* f = fopen(SAVE_FILE, "rb");
    if (f == NULL) return 0;

    int err;

    // check header
    if ((err = restoreHeader(f)) != 0) {
        fprintf(stderr, "from restoreHeader()\n");
        return err;
    }
    // process categories and checkboxes for those categories
    if ((err = restoreCategories(f)) != 0) {
        fprintf(stderr, "from restoreCategories()\n");
        return err;
    }
    // process filenames
    if ((err = restoreFilenames(f)) != 0) {
        fprintf(stderr, "from restoreFilenames()\n");
        return err;
    }
    // and finally, process file data
    if ((err = restoreFileData(f)) != 0) {
        fprintf(stderr, "from restoreFileData()\n");
        return err;
    }

    fclose(f);
    return 0;
}

int restoreHeader(FILE* f) {
    char header[5] = {0};
    if (fread(header, sizeof(char), 4, f) != 4) {
        if (feof(f)) ERR_HEADER();
        else ERR_READ();
    }
    if (strcmp(header, "\x89ICT") != 0) ERR_HEADER();

    return 0;
}

int restoreCategories(FILE* f) {
    int ch = '\x30', lastCh = '\0', processingName = 0, bufLen = 0;
    struct ictCategory* currentCategory;
    char* curStr;
    while (1) {
        switch (ch) {
            case EOF:
                ERR_TERM();
            case '\x30':  // ASCII record separator
                if (lastCh == '\x30') goto categoriesDone;
                // we're starting a new category
                categories = realloc(categories, (++nCategories) *
                    sizeof(struct ictCategory));
                currentCategory = &categories[nCategories - 1];
                currentCategory->name = calloc((bufLen = BUF_ADD_SIZE),
                    sizeof(char));
                currentCategory->chkboxes = NULL;
                currentCategory->nChkboxes = 0;
                processingName = 1;
                break;
            case '\x31':  // ASCII unit separator
                if ((lastCh == '\x30') || (lastCh == '\x31')) ERR_ZERO();
                // start a new checkbox
                curStr = calloc((bufLen = BUF_ADD_SIZE), sizeof(char));
                currentCategory->chkboxes = realloc(currentCategory->chkboxes,
                    (++currentCategory->nChkboxes) * sizeof(char*));
                currentCategory->chkboxes[currentCategory->nChkboxes - 1] =
                    curStr;
                processingName = 0;
                break;
            default: {
                bufLen = addCh(processingName ? &currentCategory->name :
                    &curStr, ch, bufLen);
                break;
            }
        }

        lastCh = ch;
        ch = fgetc(f);
        if (ferror(f)) ERR_READ();
    }
    categoriesDone:
    // there's an empty category now that we need to get rid of
    categories = realloc(categories, (--nCategories) *
        sizeof(struct ictCategory));

    return 0;
}

int restoreFilenames(FILE* f) {
    int ch = '\x31', lastCh = '\0', bufLen = 0;
    char* curStr;
    while (1) {
        switch (ch) {
            case EOF:
                ERR_TERM();
            case '\x30':
                if (lastCh == '\x31') {
                    if (nFiles > 1) ERR_ZERO();
                    else {
                        // there are no files, so just get rid of the empty one
                        free(curStr);
                        free(files);
                        files = NULL;
                        nFiles = 0;
                    }
                }
                return 0;
            case '\x31':
                if (lastCh == '\x31') ERR_ZERO();
                // start a new filename
                curStr = calloc((bufLen = BUF_ADD_SIZE), sizeof(char));
                files = realloc(files, (++nFiles) * sizeof(struct ictFile));
                files[nFiles - 1].filename = curStr;
                files[nFiles - 1].data = 0;
                break;
            default:
                bufLen = addCh(&curStr, ch, bufLen);
                break;
        }

        lastCh = ch;
        ch = fgetc(f);
        if (ferror(f)) ERR_READ();
    }

    // (we "return 0;" in "case '\x30':" above)
}

int restoreFileData(FILE* f) {
    // figure out how many checkboxes we have and how many chars fit them
    int i, chkboxCount = 0;
    for (i = 0; i < nCategories; ++i) chkboxCount += categories[i].nChkboxes;
    int chars = bitsToChars(chkboxCount);
    // TODO error if this is more than 8 ^, or handle in some other way

    char* buf = malloc(chars);
    for (i = 0; i < nFiles; ++i) {
        if (fread(buf, sizeof(char), chars, f) != chars) {
            if (feof(f)) ERR_HEADER();
            else ERR_READ();
        }
        int j;
        for (j = 0; j < chars; ++j) {
            files[i].data <<= 8;
            files[i].data |= buf[j];
        }
    }
    free(buf);

    if (fgetc(f) != -1) ERR_TRAIL();

    return 0;
}

int bitsToChars(int bits) {
    return ((bits + 7) & (~7)) >> 3;  // oooh fancy bitwise stuffs
}

#include <ncurses.h>  // also includes <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ictdata.h"  // ictFile and ictCategory, and some extern variables
struct ictFile* files = NULL;
size_t nFiles = 0;
struct ictCategory* categories = NULL;
size_t nCategories = 0;

#include "saverestore.h"  // save() and restore(), unsurprisingly

#include "interface.h"  // curses interface

int main(int argc, char* argv[]) {
    // check arguments
    if (argc < 2) {
        fprintf(stderr, "usage: %s [IMAGES...]\n", argv[0]);
        return 1;
    }

    // find image viewer program
    char* imgViewer = getenv("IMG_VIEWER");
    if (imgViewer == NULL) imgViewer = "eog";
    printf("Using image viewer `%s'. To change, call %s with the environment "
        "variable IMG_VIEWER set to a variable of your choice.\n",
        imgViewer, argv[0]);

    // make sure image viewer exists
    char* cmd = malloc((strlen(imgViewer) + 28) * sizeof(char));
    sprintf(cmd, "command -v %s >/dev/null 2>&1", imgViewer);
    if (system(cmd) != 0) {
        fprintf(stderr, "fatal: image viewer `%s' does not exist, aborting\n",
            imgViewer);
        return 1;
    }
    free(cmd);

    // check files
    int i, err = 0;
    for (i = 1; i < argc; ++i) {
        if (access(argv[i], R_OK) == -1) {
            fprintf(stderr, "%s: file does not exist\n", argv[i]);
            err = 1;
        }
    }
    if (err) return 1;

    // read existing data
    if (restore() != 0) {
        // an error happened somewhere
        return 1;
    }

    // finally ready to start!
    printf("Ready. Press enter to begin.");
    getchar();

    // set up ncurses
    initscr();  // initialize screen
    raw();      // disable line buffering, get all keys (including ex. ctrl+C)
    keypad(stdscr, TRUE);  // handling of F1, F2, arrow keys, etc.
    noecho();   // turn off echoing when a key is pressed
    refresh();

    interfaceGo();

    // cleanup ncurses
    endwin();

    return 0;
}

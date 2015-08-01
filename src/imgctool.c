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

static const char* CONTROLS[] = {
    "A/D/R: add/del/rename category", "a/d/r: add/del/rename chkbox",
    "j/k: category down/up", "h/l: checkbox left/right",
    "space: toggle checkbox", "n/p: next/previous image",
    "q/ctrl+c: save and quit", "w/ctrl+s: save"
};
static const int NCONTROLS = sizeof(CONTROLS) / sizeof(char*);
static const int CONTROL_LEN = 32;  // max len of str in CONTROLS + 2 (padding)

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

    // create windows / interface

    const int CTRL_PER_LINE = COLS / CONTROL_LEN;
    // http://stackoverflow.com/a/2745086/1223693
    const int HELP_HEIGHT = (NCONTROLS + CTRL_PER_LINE - 1) / CTRL_PER_LINE + 2;

    WINDOW* mainWin = newwin(LINES - HELP_HEIGHT, COLS, 0, 0);
    box(mainWin, 0, 0);
    mvwprintw(mainWin, 0, 2, "categories");
    for (i = 0; i < nCategories; ++i) {
        mvwprintw(mainWin, 1 + i, 1, categories[i].name);
    }
    wrefresh(mainWin);

    WINDOW* helpWin = newwin(HELP_HEIGHT, COLS, LINES - HELP_HEIGHT, 0);
    box(helpWin, 0, 0);
    mvwprintw(helpWin, 0, 2, "controls");
    for (i = 0; i < NCONTROLS; ++i) {
        mvwprintw(helpWin, 1 + i / CTRL_PER_LINE,
            1 + CONTROL_LEN * (i % CTRL_PER_LINE), CONTROLS[i]);
    }
    wrefresh(helpWin);

    char ch;
    while (1) {
        switch (ch = getch()) {
            case 'A':
                // add a category
                categories = realloc(categories, (++nCategories) *
                    sizeof(struct ictCategory));
                categories[nCategories - 1].name = calloc(3, sizeof(char));
                categories[nCategories - 1].name[0] = 'h';
                categories[nCategories - 1].name[1] = 'i';
                categories[nCategories - 1].chkboxes = NULL;
                categories[nCategories - 1].nChkboxes = 0;
                break;
            case 'a':
                // TODO checkbox add
                break;
            case 'D':
                // TODO category del
                break;
            case 'd':
                // TODO checkbox del
                break;
            case 'R':
                // TODO category rename
                break;
            case 'r':
                // TODO checkbox rename
                break;
            case 'j':
                // TODO category down
                break;
            case 'k':
                // TODO category up
                break;
            case 'h':
                // TODO checkbox left
                break;
            case 'l':
                // TODO checkbox right
                break;
            case ' ':
                // TODO checkbox toggle
                break;
            case 'n':
                // TODO image next
                break;
            case 'p':
                // TODO image prev
                break;
            case 'q':
            case '\x03': // ctrl+c
                goto quit;
            case 'w':
            case '\x13': // ctrl+s
                save();
                break;
        }
    }
    quit:

    endwin();  // cleanup

    return 0;
}

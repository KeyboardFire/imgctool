#include "interface.h"

#include <ncurses.h>

#include "ictdata.h"
#include "saverestore.h"

static const char* CONTROLS[] = {
    "A/D/R: add/del/rename category", "a/d/r: add/del/rename chkbox",
    "j/k: category down/up", "h/l: checkbox left/right",
    "space: toggle checkbox", "n/p: next/previous image",
    "q/ctrl+c: save and quit", "w/ctrl+s: save"
};
static const int NCONTROLS = sizeof(CONTROLS) / sizeof(char*);
static const int CONTROL_LEN = 32;  // max len of str in CONTROLS + 2 (padding)

static WINDOW *mainWin, *helpWin;

void interfaceGo() {
    int i;  // just a loop variable

    const int CTRL_PER_LINE = COLS / CONTROL_LEN;
    // http://stackoverflow.com/a/2745086/1223693
    const int HELP_HEIGHT = (NCONTROLS + CTRL_PER_LINE - 1) / CTRL_PER_LINE + 2;

    mainWin = newwin(LINES - HELP_HEIGHT, COLS, 0, 0);
    box(mainWin, 0, 0);
    mvwprintw(mainWin, 0, 2, "categories");
    for (i = 0; i < nCategories; ++i) {
        mvwprintw(mainWin, 1 + i, 1, categories[i].name);
    }
    wrefresh(mainWin);

    helpWin = newwin(HELP_HEIGHT, COLS, LINES - HELP_HEIGHT, 0);
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
                return;
            case 'w':
            case '\x13': // ctrl+s
                save();
                break;
        }
    }
}

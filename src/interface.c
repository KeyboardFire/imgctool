#include "interface.h"

#include <ncurses.h>
#include <string.h>

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

static WINDOW *mainWin, *helpWin, *inputPopup;

static void updateMainWin() {
    box(mainWin, 0, 0);
    mvwprintw(mainWin, 0, 2, "categories");
    int i;
    for (i = 0; i < nCategories; ++i) {
        mvwprintw(mainWin, 1 + i, 1, categories[i].name);
    }
    wrefresh(mainWin);
}

static void updateHelpWin() {
    box(helpWin, 0, 0);
    mvwprintw(helpWin, 0, 2, "controls");
    int i;
    for (i = 0; i < NCONTROLS; ++i) {
        mvwprintw(helpWin, 1 + i / (COLS / CONTROL_LEN),
            1 + CONTROL_LEN * (i % (COLS / CONTROL_LEN)), CONTROLS[i]);
    }
    wrefresh(helpWin);
}

static void (*inputCallback)(char* s);
static int gettingInput = 0;
static char* inputBuf;
static int inputBufLen;
static void getInput(void (*cb)(char* s), char* prompt) {
    inputCallback = cb;
    gettingInput = 1;
    inputBuf = calloc((inputBufLen = BUF_ADD_SIZE), sizeof(char));

    inputPopup = newwin(3, COLS, LINES / 2 - 1, 0);
    box(inputPopup, 0, 0);
    mvwprintw(inputPopup, 0, 2, prompt);
    wmove(inputPopup, 1, 1);
    wrefresh(inputPopup);
}

static void cbAddCategory(char* s) {
    categories = realloc(categories, (++nCategories) *
        sizeof(struct ictCategory));
    categories[nCategories - 1].name = calloc(strlen(s)+1, sizeof(char));
    strcpy(categories[nCategories - 1].name, s);
    categories[nCategories - 1].chkboxes = NULL;
    categories[nCategories - 1].nChkboxes = 0;
    updateMainWin();  // display new category
}

void interfaceGo() {
    const int CTRL_PER_LINE = COLS / CONTROL_LEN;
    // http://stackoverflow.com/a/2745086/1223693
    const int HELP_HEIGHT = (NCONTROLS + CTRL_PER_LINE - 1) / CTRL_PER_LINE + 2;

    mainWin = newwin(LINES - HELP_HEIGHT, COLS, 0, 0);
    updateMainWin();

    helpWin = newwin(HELP_HEIGHT, COLS, LINES - HELP_HEIGHT, 0);
    updateHelpWin();

    char ch;
    while (1) {
        ch = getch();
        if (gettingInput) {
            if (ch == '\n') {
                inputCallback(inputBuf);
                gettingInput = 0;
                free(inputBuf);

                delwin(inputPopup);
            } else if (ch == '\x07') {  // backspace
                if (strlen(inputBuf) != 0) {
                    inputBuf[strlen(inputBuf) - 1] = '\0';

                    waddstr(inputPopup, "\b \b");
                    wrefresh(inputPopup);
                }
            } else {
                inputBufLen = addCh(&inputBuf, ch, inputBufLen);

                waddch(inputPopup, ch);
                wrefresh(inputPopup);
            }
        } else switch (ch) {
            case 'A':
                // add a category
                getInput(cbAddCategory, "enter category name:");
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

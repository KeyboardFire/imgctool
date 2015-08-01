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
static int (*cursorPositions)[2];
static int nCpos = 0, cposIdx = 0;

static void updateMainWin() {
    wmove(mainWin, 1, 1);
    free(cursorPositions);
    cursorPositions = NULL;
    int i, j;
    for (i = 0; i < nCategories; ++i) {
        wattron(mainWin, A_BOLD);
        waddstr(mainWin, categories[i].name);
        wattroff(mainWin, A_BOLD);

        int idx = nCpos;
        nCpos += (categories[i].nChkboxes == 0 ? 1 : categories[i].nChkboxes);
        cursorPositions = realloc(cursorPositions, nCpos * sizeof(int[2]));

        int y, x; getyx(mainWin, y, x);
        for (j = 0; j < categories[i].nChkboxes; ++j) {
            if ((x + 6 + strlen(categories[i].chkboxes[j])) > (COLS - 1)) {
                // wrap
                waddstr(mainWin, "\n ");
                cursorPositions[idx][0] = y + 1;
                cursorPositions[idx][1] = 3;
            } else {
                cursorPositions[idx][0] = y;
                cursorPositions[idx][1] = x + 3;
            }
            ++idx;

            waddstr(mainWin, "  [ ] ");
            waddstr(mainWin, categories[i].chkboxes[j]);

            getyx(mainWin, y, x);
        }
        if (categories[i].nChkboxes == 0) {
            // allow the cursor to go to (y, 1) (start of name)
            cursorPositions[idx][0] = y;
            cursorPositions[idx][1] = 1;
        }

        // go to next line
        waddstr(mainWin, "\n ");
    }
    box(mainWin, 0, 0);
    mvwprintw(mainWin, 0, 2, "categories");
    wmove(mainWin, cursorPositions[cposIdx][0], cursorPositions[cposIdx][1]);
    wrefresh(mainWin);
}

static void updateHelpWin() {
    int i;
    for (i = 0; i < NCONTROLS; ++i) {
        mvwprintw(helpWin, 1 + i / (COLS / CONTROL_LEN),
            1 + CONTROL_LEN * (i % (COLS / CONTROL_LEN)), CONTROLS[i]);
    }
    box(helpWin, 0, 0);
    mvwprintw(helpWin, 0, 2, "controls");
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

    helpWin = newwin(HELP_HEIGHT, COLS, LINES - HELP_HEIGHT, 0);
    updateHelpWin();

    mainWin = newwin(LINES - HELP_HEIGHT, COLS, 0, 0);
    updateMainWin();

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
                // category down
                ++cposIdx;
                updateMainWin();
                break;
            case 'k':
                // category up
                --cposIdx;
                updateMainWin();
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

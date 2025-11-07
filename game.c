#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
  #include <windows.h>
  #include <mmsystem.h>
  #pragma comment(lib, "winmm.lib")
#else
  #include <unistd.h>
  #include <sys/ioctl.h>
  #include <termios.h>
  #include <time.h>
  #include <sys/types.h>
#endif

// ====== ANSI/Colors ======
#define CLEAR   "\033[2J"
#define HOME    "\033[H"
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

static char board[3][3];
static char currentPlayer = 'X';
static int  mode = 1; // 1 = PvsP, 2 = PvsCPU

// ====== Windows: enable ANSI support ======
static void enableANSI(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
    SetConsoleMode(hOut, dwMode);
#endif
}

// ====== Cross-platform sleep in ms ======
static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

// ====== Sound helpers ======
static void bell_once(void) {
#ifdef _WIN32
    Beep(800, 80);
#else
    fputc('\a', stdout);
    fflush(stdout);
    sleep_ms(60);
#endif
}

static void play_tone(int hz, int dur_ms) {
#ifdef _WIN32
    Beep(hz, dur_ms);
#else
    int repeats = dur_ms / 80;
    if (repeats < 1) repeats = 1;
    for (int i = 0; i < repeats; ++i) bell_once();
#endif
}

static void sfx_start(void) {           // short ascending
#ifdef _WIN32
    Beep(700, 80); Beep(900, 80); Beep(1100, 100);
#else
    bell_once(); sleep_ms(60); bell_once(); sleep_ms(60); bell_once();
#endif
}
static void sfx_move(void)      { play_tone(1000, 60); }     // tick
static void sfx_cpu_move(void)  { play_tone(700, 70); }      // lower tick
static void sfx_invalid(void) {
#ifdef _WIN32
    Beep(400, 90); Beep(300, 120);
#else
    bell_once(); sleep_ms(100); bell_once();
#endif
}
static void sfx_win(void) {
#ifdef _WIN32
    Beep(800,120); Beep(1000,120); Beep(1200,180);
#else
    bell_once(); sleep_ms(90); bell_once(); sleep_ms(90); bell_once();
#endif
}
static void sfx_draw(void) { play_tone(600, 160); }

// ====== Terminal width ======
static int getTerminalWidth(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int width = 80;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return width > 0 ? width : 80;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    return 80;
#endif
}

// ====== Visible length (ignores ANSI escape sequences) ======
static int visible_len_ansi(const char *s) {
    int len = 0;
    for (int i = 0; s[i]; ) {
        if (s[i] == '\x1b' && s[i+1] == '[') {
            i += 2;
            while (s[i] && !(s[i] >= '@' && s[i] <= '~')) i++; // consume CSI params
            if (s[i]) i++; // consume final byte
        } else {
            len++; i++;
        }
    }
    return len;
}

// ====== Center helpers ======
static void centerWrite(const char *text, int newline) {
    int width = getTerminalWidth();
    int len = visible_len_ansi(text);
    int spaces = (width - len) / 2;
    if (spaces < 0) spaces = 0;
    for (int i = 0; i < spaces; i++) putchar(' ');
    fputs(text, stdout);
    if (newline) putchar('\n');
}

static void centerText(const char *text)         { centerWrite(text, 1); }
static void centerPrompt(const char *text)       { centerWrite(text, 0); }

static void cprintf_center(int newline, const char *fmt, ...) {
    char buf[1024];
    va_list ap; va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    centerWrite(buf, newline);
}

// ====== Board & game ======
static void initializeBoard(void) {
    int n = 1;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = '0' + n++;
}

static void clearScreen(void) {
    printf(CLEAR HOME);
    fflush(stdout);
}

static void printBoard(void) {
    clearScreen();
    putchar('\n'); putchar('\n'); putchar('\n');

    centerText(CYAN "============================================" RESET);
    centerText(CYAN "             TIC TAC TOE GAME               " RESET);
    centerText(CYAN "============================================" RESET);
    putchar('\n'); putchar('\n');

    // Row will look like: " %c | %c | %c "  -> 13 visible chars
    const int visible_row_width = 13;
    int term_w = getTerminalWidth();
    int indent = (term_w - visible_row_width) / 2;
    if (indent < 0) indent = 0;

    for (int i = 0; i < 3; i++) {
        for (int s = 0; s < indent; ++s) putchar(' ');
        for (int j = 0; j < 3; j++) {
            char mark = board[i][j];
            if (mark == 'X')      printf(YELLOW " %c " RESET, mark);
            else if (mark == 'O') printf(RED    " %c " RESET, mark);
            else                  printf(" %c ", mark);

            if (j < 2) printf(CYAN "|" RESET), putchar(' ');
        }
        putchar('\n');

        if (i < 2) {
            for (int s = 0; s < indent; ++s) putchar(' ');
            printf(CYAN "----+-----+----" RESET "\n");
        }
    }
    putchar('\n'); putchar('\n');
}

static bool checkWin(void) {
    for (int i = 0; i < 3; i++)
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2]) return true;
    for (int i = 0; i < 3; i++)
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i]) return true;
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) return true;
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0]) return true;
    return false;
}

static bool isDraw(void) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] != 'X' && board[i][j] != 'O') return false;
    return true;
}

static bool makeMove(int block) {
    if (block < 1 || block > 9) return false;
    int row = (block - 1) / 3;
    int col = (block - 1) % 3;
    if (board[row][col] == 'X' || board[row][col] == 'O') return false;
    board[row][col] = currentPlayer;
    return true;
}

// ====== Input (centered prompts). 'Q' quits -> -999 ======
static int readIntInRange(const char *prompt, int minVal, int maxVal) {
    char line[128];
    long val;
    char *end;

    for (;;) {
        if (prompt && *prompt) { centerPrompt(prompt); fflush(stdout); }

        if (!fgets(line, sizeof(line), stdin)) { clearerr(stdin); continue; }

        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;

        if (toupper((unsigned char)*p) == 'Q') return -999;
        if (*p == '\0' || *p == '\n') continue;

        val = strtol(p, &end, 10);
        if (p == end) {
            cprintf_center(1, RED "Invalid input (not a number). Try again." RESET);
            sfx_invalid();
            continue;
        }

        while (*end && *end != '\n' && isspace((unsigned char)*end)) end++;
        if (*end != '\0' && *end != '\n') {
            cprintf_center(1, RED "Invalid input (extra characters). Try again." RESET);
            sfx_invalid();
            continue;
        }

        if (val < minVal || val > maxVal) {
            cprintf_center(1, RED "Please enter a number only %d to %d." RESET, minVal, maxVal);
            sfx_invalid();
            continue;
        }

        return (int)val;
    }
}

// ====== Computer move ======
static void computerMove(void) {
    for (int tries = 0; tries < 50; ++tries) {
        int block = rand() % 9 + 1;
        if (makeMove(block)) {
            cprintf_center(1, MAGENTA "Computer chose block %d" RESET, block);
            sfx_cpu_move();
            return;
        }
    }
    for (int b = 1; b <= 9; b++) {
        if (makeMove(b)) {
            cprintf_center(1, MAGENTA "Computer chose block %d" RESET, b);
            sfx_cpu_move();
            return;
        }
    }
}

// ====== Play again ======
static int askPlayAgain(void) {
    char buf[64];
    for (;;) {
        centerPrompt(RED "Play again? (1 = Yes / 0 = No): " RESET);
        fflush(stdout);
        if (!fgets(buf, sizeof(buf), stdin)) return 0;
        char c = 0;
        for (int i = 0; buf[i]; ++i) {
            if (!isspace((unsigned char)buf[i])) { c = (char)tolower((unsigned char)buf[i]); break; }
        }
        if (c == '1' || c == 'y') return 1;
        if (c == '0' || c == 'n' ) return 0;
        cprintf_center(1, RED "Invalid choice. Use 1 / 0." RESET);
        sfx_invalid();
    }
}

// ====== MAIN ======
int main(void) {
    enableANSI();
    srand((unsigned)time(NULL));

    clearScreen();
    putchar('\n'); putchar('\n'); putchar('\n');
    centerText(CYAN "======================================================================" RESET);
    centerText(CYAN "                       WELCOME TO TIC TAC TOE                         " RESET);
    centerText(CYAN "======================================================================" RESET);
    putchar('\n');
    centerText(YELLOW "~ ~ ~ Press ENTER to start the game ~ ~ ~" RESET);
    fflush(stdout);
    sfx_start();

    char dummy[16];
    fgets(dummy, sizeof(dummy), stdin);

    int playAgain = 1;

    while (playAgain) {
        // Mode select
        clearScreen();
        putchar('\n'); putchar('\n'); putchar('\n');
        centerText(MAGENTA "*************** Select Game Mode ***************" RESET);
        centerText(CYAN "1. Player vs Player" RESET);
        centerText(CYAN "2. Player vs Computer" RESET);
        putchar('\n');

        mode = readIntInRange(MAGENTA "Enter your choice (1 or 2): " RESET, 1, 2);
        if (mode == -999) {
            clearScreen();
            centerText(GREEN "-------------------- Quit by user --------------------" RESET);
            return 0;
        }

        initializeBoard();
        currentPlayer = 'X';
        bool gameOver = false;

        while (!gameOver) {
            printBoard();

            if (mode == 2 && currentPlayer == 'O') {
                computerMove();
                sleep_ms(500);
            } else {
                int block = readIntInRange("Enter block (1 - 9) or Q to Quit: ", 1, 9);
                if (block == -999) {
                    clearScreen();
                    centerText(GREEN "---------------------- Game Quit by User ----------------------" RESET);
                    return 0;
                }
                while (!makeMove(block)) {
                    cprintf_center(1, RED "That block is already taken. Try again." RESET);
                    sfx_invalid();
                    block = readIntInRange("Enter block (1 - 9) or Q to Quit: ", 1, 9);
                    if (block == -999) {
                        clearScreen();
                        centerText(GREEN "---------------------- Game Quit by User ----------------------" RESET);
                        return 0;
                    }
                }
                sfx_move();
            }

            // refresh and check state
            printBoard();

            if (checkWin()) {
                putchar('\n');
                if (mode == 2 && currentPlayer == 'O') {
                    centerText(GREEN "****************** COMPUTER Wins! ******************" RESET);
                } else {
                    cprintf_center(1, GREEN "****************** Player %c Wins! ******************" RESET, currentPlayer);
                }
                sfx_win();
                gameOver = true;
            } else if (isDraw()) {
                centerText(GREEN "******************** It's a Draw! *******************" RESET);
                sfx_draw();
                gameOver = true;
            } else {
                currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
            }
        }

        putchar('\n'); putchar('\n');
        playAgain = askPlayAgain();
    }

    clearScreen();
    putchar('\n'); putchar('\n'); putchar('\n');
    centerText(GREEN "---------------------- Thanks for playing Tic Tac Toe! ----------------------" RESET);
    putchar('\n'); putchar('\n');
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif

#define CLEAR "\033[2J"
#define HOME "\033[H"
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

char board[3][3];
char currentPlayer = 'X';
int mode = 1;

// ====== ENABLE ANSI COLORS ON WINDOWS ======
void enableANSI() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ====== SOUND FUNCTIONS ======
void playInputSound() {
#ifdef _WIN32
    Beep(750, 120);
#else
    printf("\a");
    fflush(stdout);
#endif
}

void playWinSound() {
#ifdef _WIN32
    PlaySound(TEXT("C:\\Windows\\Media\\tada.wav"), NULL, SND_FILENAME | SND_ASYNC);
#else
    system("afplay /System/Library/Sounds/Hero.aiff 2>/dev/null || play -q -n synth 0.3 tri 800 fade 0 0.1 0.2");
#endif
}

void playLoseSound() {
#ifdef _WIN32
    PlaySound(TEXT("C:\\Windows\\Media\\Windows Error.wav"), NULL, SND_FILENAME | SND_ASYNC);
#else
    system("afplay /System/Library/Sounds/Basso.aiff 2>/dev/null || play -q -n synth 0.3 sine 200 fade 0 0.1 0.3");
#endif
}

void playDrawSound() {
#ifdef _WIN32
    Beep(700, 200);
    Beep(700, 200);
#else
    printf("\a");
    fflush(stdout);
#endif
}

// ====== TERMINAL WIDTH ======
int getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int width = 80;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hOut, &csbi))
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    return width;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        return w.ws_col > 0 ? w.ws_col : 80;
    return 80;
#endif
}

// ====== CENTER TEXT ======
void centerText(const char *text) {
    int width = getTerminalWidth();
    int len = strlen(text);
    int spaces = (width - len) / 2;
    if (spaces < 0)
        spaces = 0;
    for (int i = 0; i < spaces; i++)
        printf(" ");
    printf("%s\n", text);
}

// ====== GAME LOGIC ======
void initializeBoard() {
    int n = 1;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = '0' + n++;
}

void clearScreen() {
    printf(CLEAR HOME);
}

void printBoard() {
    clearScreen();
    printf("\n\n\n\n");

    centerText(CYAN "============================================" RESET);
    centerText(CYAN "            TIC TAC TOE GAME                " RESET);
    centerText(CYAN "============================================" RESET);
    printf("\n\n");

    int width = getTerminalWidth();
    int indent = (width - 25) / 2;
    if (indent < 0) indent = 0;

    for (int i = 0; i < 3; i++) {
        for (int s = 0; s < indent; s++) printf(" ");
        for (int j = 0; j < 3; j++) {
            char mark = board[i][j];
            if (mark == 'X')
                printf(YELLOW " %c " RESET, mark);
            else if (mark == 'O')
                printf(RED " %c " RESET, mark);
            else
                printf(" %c " RESET, mark);

            if (j < 2)
                printf(CYAN "|" RESET);
        }
        printf("\n");
        if (i < 2) {
            for (int s = 0; s < indent; s++) printf(" ");
            printf(CYAN "---+---+---" RESET "\n");
        }
    }
    printf("\n");
}

bool checkWin() {
    for (int i = 0; i < 3; i++)
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return true;
    for (int i = 0; i < 3; i++)
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return true;
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return true;
    if (board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return true;
    return false;
}

bool isDraw() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] != 'X' && board[i][j] != 'O')
                return false;
    return true;
}

bool makeMove(int block) {
    if (block < 1 || block > 9)
        return false;
    int row = (block - 1) / 3;
    int col = (block - 1) % 3;
    if (board[row][col] == 'X' || board[row][col] == 'O')
        return false;
    board[row][col] = currentPlayer;
    return true;
}

void computerMove() {
    int block;
    do {
        block = rand() % 9 + 1;
    } while (!makeMove(block));
    printf(MAGENTA "\n\t\tComputer chose block %d\n" RESET, block);
#ifdef _WIN32
    Beep(650, 150);
#else
    printf("\a");
    fflush(stdout);
#endif
}

// ====== MAIN ======
int main() {
    enableANSI();
    srand(time(0));
    clearScreen();
    printf("\n\n\n\n");
    centerText(CYAN "=============================================================" RESET);
    centerText(CYAN "            WELCOME TO TIC TAC TOE GAME!                     " RESET);
    centerText(CYAN "=============================================================" RESET);
    printf("\n");
    centerText(YELLOW "~ Press ENTER to Start the Game ~" RESET);
    getchar();

    int playAgain = 1;
    while (playAgain) {
        clearScreen();
        printf("\n\n\n");
        centerText(MAGENTA "*************** Select Game Mode ***************" RESET);
        centerText(CYAN "1. Player vs Player");
        centerText(CYAN "2. Player vs Computer");
        printf(MAGENTA "\n\tEnter your choice (1 or 2): " RESET);
        scanf("%d", &mode);
        flushInput();
        if (mode != 1 && mode != 2)
            mode = 1;

        initializeBoard();
        currentPlayer = 'X';
        bool gameOver = false;

        while (!gameOver) {
            printBoard();

            if (mode == 2 && currentPlayer == 'O') {
                computerMove();
#ifdef _WIN32
                Sleep(1000);
#else
                sleep(1);
#endif
            } else {
                printf(CYAN "\t\tPlayer %c, enter block (1 - 9): " RESET, currentPlayer);
                int block;
                scanf("%d", &block);
                playInputSound();
                flushInput();
                if (!makeMove(block)) {
                    printf(CYAN "\tInvalid move! Press Enter...");
                    getchar();
                    continue;
                }
            }

            if (checkWin()) {
                playWinSound();
                printBoard();
                printf("\n");

                if (mode == 2 && currentPlayer == 'O') {
                    centerText(RED "**************** COMPUTER WINS! ****************" RESET);
                } else {
                    char msg[50];
                    sprintf(msg, GREEN "************ PLAYER %c WINS! ************" RESET, currentPlayer);
                    centerText(msg);
                }

                gameOver = true;
            } else if (isDraw()) {
                playDrawSound();
                printBoard();
                centerText(GREEN "**************** It's a Draw! ****************" RESET);
                gameOver = true;
            } else {
                currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
            }
        }

        printf("\n");
        printf(RED "\tPlay again? (1 = Yes / 0 = No): " RESET);
        scanf("%d", &playAgain);
        flushInput();
    }

    clearScreen();
    printf("\n\n\n");
    centerText(GREEN "------------- Thanks for playing Tic Tac Toe! -------------" RESET);
    printf("\n\n\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#endif

// ====== COLOR CODES ======
#define CLEAR "\033[2J"
#define HOME "\033[H"
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

// ====== GLOBAL VARIABLES ======
char board[3][3];
char currentPlayer = 'X';
int mode = 1;

// ====== ENABLE ANSI COLORS ON WINDOWS ======
void enableANSI()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
    {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
#endif
}

// ====== FLUSH INPUT ======
void flushInput()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// ====== SOUND FUNCTIONS ======
void playInputSound()
{
#ifdef _WIN32
    Beep(750, 120);
#else
    printf("\a");
    fflush(stdout);
#endif
}

void playWinSound()
{
#ifdef _WIN32
    Beep(1000, 150);
    Beep(1200, 150);
    Beep(1500, 200);
#elif _APPLE_
    system("afplay /System/Library/Sounds/Tink.aiff 2>/dev/null &");
#else
    system("play -q -n synth 0.3 tri 800 fade 0 0.1 0.2 2>/dev/null &");
#endif
}

void playDrawSound()
{
#ifdef _WIN32
    Beep(700, 200);
    Beep(700, 200);
#elif _APPLE_
    system("afplay /System/Library/Sounds/Pop.aiff 2>/dev/null &");
#else
    printf("\a");
    fflush(stdout);
#endif
}

// ====== TERMINAL WIDTH ======
int getTerminalWidth()
{
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
void centerText(const char *text)
{
    int width = getTerminalWidth();
    int len = (int)strlen(text);
    int spaces = (width - len) / 2;
    if (spaces < 0) spaces = 0;
    for (int i = 0; i < spaces; i++) printf(" ");
    printf("%s\n", text);
}

// ====== GAME LOGIC ======
void initializeBoard()
{
    int n = 1;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = '0' + n++;
}

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    printf(CLEAR HOME);
#endif
}

void printBoard()
{
    clearScreen();
    printf("\n\n\n\n");

    centerText(CYAN "============================================" RESET);
    centerText(CYAN "            TIC TAC TOE GAME                " RESET);
    centerText(CYAN "============================================" RESET);
    printf("\n\n");

    int width = getTerminalWidth();
    int indent = (width - 25) / 2;
    if (indent < 0) indent = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int s = 0; s < indent; s++) printf(" ");
        for (int j = 0; j < 3; j++)
        {
            char mark = board[i][j];
            if (mark == 'X')
                printf(YELLOW " %c " RESET, mark);
            else if (mark == 'O')
                printf(RED " %c " RESET, mark);
            else
                printf(" %c ", mark);
            if (j < 2) printf(CYAN "|" RESET);
        }
        printf("\n");
        if (i < 2)
        {
            for (int s = 0; s < indent; s++) printf(" ");
            printf(CYAN "---+---+---" RESET "\n");
        }
    }
    printf("\n");
}

bool checkWin()
{
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

bool isDraw()
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] != 'X' && board[i][j] != 'O')
                return false;
    return true;
}

bool makeMove(int block)
{
    if (block < 1 || block > 9)
        return false;
    int row = (block - 1) / 3;
    int col = (block - 1) % 3;
    if (board[row][col] == 'X' || board[row][col] == 'O')
        return false;
    board[row][col] = currentPlayer;
    return true;
}

void computerMove()
{
    int block;
    do
    {
        block = rand() % 9 + 1;
    } while (!makeMove(block));
    printf(MAGENTA "\n\n\t\t\t\t\t\t\t\t\t\tComputer chose Block ' %d '\n" RESET, block);
#ifdef _WIN32
    Beep(650, 150);
#else
    printf("\a");
    fflush(stdout);
#endif
#ifdef _WIN32
    Sleep(800);
#else
    sleep(1);
#endif
}

// ====== INPUT UTIL: read block or quit ======
bool readBlockOrQuit(int *outBlock, bool *quitRequested)
{
    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin))
        return false;

    // strip trailing newline
    size_t n = strlen(buf);
    if (n && buf[n-1] == '\n') buf[n-1] = '\0';

    // check quit
    if (buf[0] == 'q' || buf[0] == 'Q')
    {
        *quitRequested = true;
        return true; // successfully handled input
    }

    // parse integer
    char *endptr = NULL;
    long val = strtol(buf, &endptr, 10);
    if (endptr == buf) // no digits parsed
        return false;

    *outBlock = (int)val;
    return true;
}

// ====== MAIN ======
int main()
{
    enableANSI();
    srand((unsigned)time(NULL));

    clearScreen();
    printf("\n\n\n\n");
    centerText(CYAN "=============================================================" RESET);
    centerText(CYAN "                  WELCOME TO TIC TAC TOE GAME!               " RESET);
    centerText(CYAN "=============================================================" RESET);
    printf("\n");
    centerText(YELLOW "~ Press ENTER to Start the Game ~" RESET);
    getchar();

    int playAgain = 1;
    while (playAgain)
    {
        clearScreen();
        printf("\n\n\n");
        centerText(MAGENTA "       ***************** Select Game Mode ***************\n\n" RESET);
        centerText(CYAN "1. Player vs Player\n");
        centerText(CYAN "2. Player vs Computer");

        do
        {
            printf(YELLOW "\n\n\t\t\t\t\t\t\t\t\t\t  Enter your choice (1 or 2): " RESET);
            if (scanf("%d", &mode) != 1)
            {
                flushInput();
                mode = 0;
            }
            else
            {
                flushInput();
            }

            if (mode != 1 && mode != 2)
            {
                printf(RED "\n\t\t\t\t\t\t\t\t\t\tInvalid input! Please enter 1 or 2.\n" RESET);
#ifdef _WIN32
                Beep(500, 150);
#else
                printf("\a");
                fflush(stdout);
#endif
            }

        } while (mode != 1 && mode != 2);

        initializeBoard();
        currentPlayer = 'X';
        bool gameOver = false;

        while (!gameOver)
        {
            printBoard();
            if (mode == 2 && currentPlayer == 'O')
            {
                computerMove();
            }
            else
            {
                printf(CYAN "\n\n\t\t\t\t\t\t\t\t\t Player %c, enter block (1 - 9 " YELLOW "or Q to quit" CYAN "): " RESET, currentPlayer);

                int block = -1;
                bool quitRequested = false;

                if (!readBlockOrQuit(&block, &quitRequested))
                {
                    printf(CYAN "\n\n\t\t\t\t\t\t\t\t\t\tInvalid input! Press Enter..." RESET);
                    getchar();
                    continue;
                }

                playInputSound();

                if (quitRequested)
                {
                    printBoard();
                    char msg[120];
                    snprintf(msg, sizeof(msg),
                             YELLOW "************ PLAYER %c CHOSE TO QUIT. GAME OVER. ************" RESET,
                             currentPlayer);
                    centerText(msg);
                    gameOver = true;
                    break;
                }

                if (!makeMove(block))
                {
                    printf(CYAN "\n\n\t\t\t\t\t\t\t\t\t\tInvalid move! Press Enter..." RESET);
                    getchar();
                    continue;
                }
            }

            if (checkWin())
            {
                playWinSound();
                printBoard();
                printf("\n");
                if (mode == 2 && currentPlayer == 'O')
                    centerText(RED "\n\n\t\t\t\t\t\t\t\t\t\t**************** COMPUTER WINS! ****************" RESET);
                else
                {
                    char msg[60];
                    sprintf(msg, GREEN "************ PLAYER %c WINS! ************" RESET, currentPlayer);
                    centerText(msg);
                }
                gameOver = true;
            }
            else if (isDraw())
            {
                playDrawSound();
                printBoard();
                centerText(GREEN "**************** It's a Draw! ****************" RESET);
                gameOver = true;
            }
            else
            {
                currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
            }
        }

        if (!gameOver)
        {
            // Safety (shouldn't hit, but keep consistent)
            printBoard();
            centerText(YELLOW "************ GAME ENDED ************" RESET);
        }

        printf("\n");
        printf(RED "\n\n\t\t\t\t\t\t\t\t\t\t Play again? (1 = Yes / 0 = No): " RESET);
        if (scanf("%d", &playAgain) != 1) {
            playAgain = 0;
        }
        flushInput();
    }

    clearScreen();
    printf("\n\n\n\n\n\n\n\n\n");
    centerText(RED "----------------------------------- THE END -----------------------------------------\n\n");
    centerText(GREEN "          ------------------ Thanks for playing Tic Tac Toe! ------------------\n\n\n\n\n\n\n\n\n" RESET);
    printf("\n\n\n");
    return 0;
}

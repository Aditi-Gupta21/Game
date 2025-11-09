#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// ====== COLOR CODES ======
#define CLEAR "\033[2J"
#define HOME "\033[H"
#define RESET  "\033[0m"
#define BLACK  "\033[30m"
#define RED    "\033[31m"
#define GREEN  "\033[32m"
#define YELLOW "\033[33m"
#define BLUE   "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN   "\033[36m"
#define WHITE  "\033[37m"

// ===== BRIGHT (HIGH INTENSITY) COLORS =====
#define BRIGHT_BLACK   "\033[90m"
#define BRIGHT_RED     "\033[91m"
#define BRIGHT_GREEN   "\033[92m"
#define BRIGHT_YELLOW  "\033[93m"
#define BRIGHT_BLUE    "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN    "\033[96m"
#define BRIGHT_WHITE   "\033[97m"

char board[3][3];
char currentPlayer = 'X';

// ====== CENTER TEXT ======
void centerText(const char *text)
{
    int width = 80; // assume 80-column terminal
    int len = strlen(text);
    int spaces = (width - len) / 2;
    for (int i = 0; i < spaces; i++)
        printf(" ");
    printf("%s\n", text);
}

// ====== INITIALIZE BOARD ======
void initializeBoard()
{
    int n = 1;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = '0' + n++;
}

// ====== CLEAR SCREEN ======
void clearScreen()
{
    printf(CLEAR HOME);
}

// ====== PRINT BOARD ======
void printBoard()
{
    clearScreen();
    printf("\n\n\n");
    centerText(CYAN "\t==================================" RESET);
    centerText(CYAN "\t          TIC TAC TOE GAME         " RESET);
    centerText(CYAN "\t==================================" RESET);
    printf("\n\n");

    for (int i = 0; i < 3; i++)
    {
        printf("\t\t\t\t"); // horizontal centering
        for (int j = 0; j < 3; j++)
        {
            char mark = board[i][j];
            if (mark == 'X')
                printf(YELLOW " %c   " RESET, mark);
            else if (mark == 'O')
                printf(RED " %c   " RESET, mark);
            else
                printf(" %c   " RESET, mark);

            if (j < 2)
                printf(CYAN "|" RESET);
        }
        printf("\n");

        if (i < 2)
            printf("\t\t\t" CYAN "      -------+-----+-------" RESET "\n");
    }
    printf("\n\n");
}

// ====== CHECK WIN ======
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

// ====== CHECK DRAW ======
bool isDraw()
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] != 'X' && board[i][j] != 'O')
                return false;
    return true;
}

// ====== MAKE MOVE ======
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

// ====== MAIN FUNCTION ======
int main(){
    int playAgain = 1;

    while (playAgain)
    {
        initializeBoard();
        currentPlayer = 'X';
        bool gameOver = false;

        while (!gameOver)
        {
            printBoard();
            printf(CYAN "\t\t\t Player %c, enter block (1 - 9): " RESET, currentPlayer);

            int block;
            scanf("%d", &block);

            if (!makeMove(block))
            {
                printf(CYAN "\t\t\t Invalid move! Press Enter...");
                getchar();
                getchar();
                continue;
            }

            // === Refresh screen after each move ===
            clearScreen(); // or system("cls");
            printBoard();

            if (checkWin())
            {
                printf("\n");
                char msg[40];
                sprintf(msg, "****************** Player %c Wins! ******************", currentPlayer);
                centerText(GREEN);
                centerText(msg);
                centerText(RESET);
                gameOver = true;
            }
            else if (isDraw())
            {
                printBoard();
                centerText(GREEN "\t**************** It's a Draw! ****************" RESET);
                gameOver = true;
            }
            else
            {
                currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
            }
        }

        printf("\n\n");
        centerText(BRIGHT_BLUE "\tPlay again? (1 = Yes / 0 = No): " RESET);
        scanf("%d", &playAgain);
    }

    clearScreen();
    centerText(MAGENTA "\n\n\n\n\n\n~~~~~~~~~~~~~~~~~~~~~ Thanks for playing Tic Tac Toe! ~~~~~~~~~~~~~~~~~~~~~" RESET);
    printf("\n\n\n");
    return 0;
}

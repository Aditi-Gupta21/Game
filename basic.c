#include <stdio.h>

// Function prototypes
void drawBoard();
int checkWin();
void resetBoard();

// Global variables
char board[3][3];
char currentPlayer = 'X';

int main() {
    int move, row, col, moves = 0;
    int winner = 0;

    resetBoard();

    while (1) {
        drawBoard();
        printf("Player %c, enter your move (1-9): ", currentPlayer);
        scanf("%d", &move);

        // Validate move number
        if (move < 1 || move > 9) {
            printf("Invalid move. Choose a number between 1 and 9.\n");
            continue;
        }

        // Convert move (1–9) to row & column
        row = (move - 1) / 3;
        col = (move - 1) % 3;

        // Check if cell is empty
        if (board[row][col] != ' ') {
            printf("Cell already taken. Try again.\n");
            continue;
        }

        board[row][col] = currentPlayer;
        moves++;

        // Check for winner
        if (checkWin()) {
            drawBoard();
            printf("Player %c wins! 🎉\n", currentPlayer);
            break;
        } else if (moves == 9) {
            drawBoard();
            printf("It's a draw! 🤝\n");
            break;
        }

        // Switch player
        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    }

    return 0;
}

// Reset the board with empty spaces
void resetBoard() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = ' ';
}

// Display the board with numbering (1–9)
void drawBoard() {
    printf("\n");
    int num = 1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == ' ')
                printf(" %d ", num);
            else
                printf(" %c ", board[i][j]);
            if (j < 2) printf("|");
            num++;
        }
        printf("\n");
        if (i < 2) printf("---+---+---\n");
    }
    printf("\n");
}

// Check for a winner
int checkWin() {
    // Rows and columns
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == currentPlayer &&
            board[i][1] == currentPlayer &&
            board[i][2] == currentPlayer)
            return 1;

        if (board[0][i] == currentPlayer &&
            board[1][i] == currentPlayer &&
            board[2][i] == currentPlayer)
            return 1;
    }

    // Diagonals
    if (board[0][0] == currentPlayer &&
        board[1][1] == currentPlayer &&
        board[2][2] == currentPlayer)
        return 1;

    if (board[0][2] == currentPlayer &&
        board[1][1] == currentPlayer &&
        board[2][0] == currentPlayer)
        return 1;

    return 0;
}

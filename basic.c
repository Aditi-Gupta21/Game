#include <stdio.h>

// Function prototypes
void drawBoard();
int checkWin();
void resetBoard();

// Global variables
char board[3][3];
char currentPlayer = 'X';

int main() {
    int row, col, moves = 0;
    int winner = 0;

    resetBoard();

    while (1) {
        drawBoard();
        printf("Player %c, enter your move (row and column: 1-3): ", currentPlayer);
        scanf("%d %d", &row, &col);

        // Adjust for 0-based index
        row--;
        col--;

        // Validate move
        if (row < 0 || row >= 3 || col < 0 || col >= 3 || board[row][col] != ' ') {
            printf("Invalid move. Try again.\n");
            continue;
        }

        board[row][col] = currentPlayer;
        moves++;

        winner = checkWin();
        if (winner == 1) {
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

// Display the board
void drawBoard() {
    printf("\n");
    printf("  1   2   3\n");
    for (int i = 0; i < 3; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < 3; j++) {
            printf(" %c ", board[i][j]);
            if (j < 2) printf("|");
        }
        printf("\n");
        if (i < 2) printf("  ---+---+---\n");
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#define ROWS 6
#define COLS 7
#define N 4

typedef enum {
    EMPTY = 0,
    PLAYER_X = 1,
    PLAYER_Y = 2
} Cell;

int
isSetPlayerInformationSuccess (int* ally_player, int* opponent_player)
{
    int player_character;
    // scanf(" %d", &player_character);
    read(STDIN_FILENO, &player_character, sizeof(int));

    if (player_character == PLAYER_X) {
        *ally_player = PLAYER_X;
        *opponent_player = PLAYER_Y;
    }
    else if (player_character == PLAYER_Y) {
        *ally_player = PLAYER_Y;
        *opponent_player = PLAYER_X;
    }
    else {
        return false;
    }
    return true;
}

int
isSetBoardInformationSuccess (int board[ROWS][COLS])
{
    // for (int i = 0; i < ROWS; i++) {
    //     for (int j = 0; j < COLS; j++) {
    //         scanf("%d", &board[i][j]);
    //     }
    // }
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            read(STDIN_FILENO, &board[i][j], sizeof(int));
        }
    }
    return true;
}

int
isSetInputInformationSuccess (int* ally_player, int* opponent_player, int board[ROWS][COLS])
{
    if (isSetPlayerInformationSuccess(ally_player, opponent_player) &&
        isSetBoardInformationSuccess(board)) {
        return true;
    }
    return false;
}

void
printCurrentPlayer(int ally_player)
{
    printf("%d\n", ally_player);
}

void
printBoard (int row, int column, int board[ROWS][COLS])
{
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < column; j++) {
            printf("%2d ", board[i][j]);
        }
        printf("\n");
    }
}

int*
getFilledColumnCount (int board[ROWS][COLS])
{
    int* filled_column_count = calloc(COLS, sizeof(int));

    for (int col = 0; col < COLS; col++) {
        for (int row = ROWS - 1; row >= 0; row--) {
            if (board[row][col] != 0) {
                filled_column_count[col]++;
            }
        }
    }
    return filled_column_count;
}

int*
getFilledRowCount (int board[ROWS][COLS])
{
    int* filled_row_count = calloc(ROWS, sizeof(int));

    for (int row = ROWS - 1; row >= 0; row--) {
        for (int col = 0; col < COLS; col++) {
            if (board[row][col] != 0) {
                filled_row_count[row]++;
            }
        }
    }
    return filled_row_count;
}

int
isValidIndex (int row, int col)
{
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        return false;
    }
    return true;
}

int
isEmptyCell (int board[ROWS][COLS], int row, int col)
{
    if (board[row][col] == 0) {
        return true;
    }
    return false;
}

int
isUnderFilled (int board[ROWS][COLS], int row, int col)
{
    if (board[row + 1][col] != 0) {
        return true;
    }
    return false;
}

int
isLowestRow (int row)
{
    if (row == ROWS - 1) {
        return true;
    }
    return false;
}

int
isValidPlace (int board[ROWS][COLS], int row, int col)
{
    if (!isValidIndex(row, col)) {
        return false;
    }

    if (isLowestRow(row)) {
        if (isEmptyCell(board, row, col)) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        if (isUnderFilled(board, row, col) && 
            isEmptyCell(board, row, col)) {
            return true;    
        }
        else {
            return false;
        }
    }
}

int
isValuableVerticalCheck (int col, int* filled_column_count, int num)
{
    if (filled_column_count[col] < num ||
        filled_column_count[col] == ROWS) {
        return false;
    }
    return true;
}

int
checkVerticalCase1 (int player, int board[ROWS][COLS], 
                            int* filled_column_count, int col)
{
    int top_index = ROWS - filled_column_count[col];

    if (board[top_index][col] == player &&
        board[top_index + 1][col] == player &&
        board[top_index + 2][col] == player) {
        return col;
    }
    return -1;
}

int
getVerticalCase (int is_offense, int ally_player, int opponent_player, int board[ROWS][COLS], int* filled_column_count)
{
    int result;
    int player;
    
    if (is_offense == true) {
        player = ally_player;
    }
    else {
        player = opponent_player;
    }

    for (int col = 0; col < COLS; col++)
    {   
        if (!isValuableVerticalCheck(col, filled_column_count, 3)) {
            continue;
        }

        result = checkVerticalCase1(player, board, filled_column_count, col);
        if (result != -1) {
            return result;
        }
    }
    return -1;
}

int
isValuableDiagonalRightCheck (int row, int col, int* filled_column_count)
{
    if (filled_column_count[col] <  (5 - row) ||
        filled_column_count[col + 1] < (4 - row) ||
        filled_column_count[col + 2] < (3 - row) ||
        filled_column_count[col + 3] < (2 - row)) {
        return false;
    }
    return true;
}

int
checkDiagonalRightCase1 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row][col] == player &&
        board[row + 1][col + 1] == player &&
        board[row + 2][col + 2] == player) {
        if (isValidPlace(board, row + 3, col + 3)) {
            return col + 3;
        }
    }
    return -1;
}

int
checkDiagonalRightCase2 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row][col] == player &&
        board[row + 1][col + 1] == player &&
        board[row + 3][col + 3] == player) {
        if (isValidPlace(board, row + 2, col + 2)) {
            return col + 2;
        }
    }
    return -1;
}

int
checkDiagonalRightCase3 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row][col] == player &&
        board[row + 2][col + 2] == player &&
        board[row + 3][col + 3] == player) {
        if (isValidPlace(board, row + 1, col + 1)) {
            return col + 1;
        }
    }
    return -1;
}

int
checkDiagonalRightCase4 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row + 1][col + 1] == player &&
        board[row + 2][col + 2] == player &&
        board[row + 3][col + 3] == player) {
        if (isValidPlace(board, row, col)) {
            return col;
        }
    }
    return -1;
}

int
getDiagonalRightCase (int is_offense, int ally_player, int opponent_player, int board[ROWS][COLS], int* filled_column_count)
{
    int result;
    int player;
    
    if (is_offense == true) {
        player = ally_player;
    }
    else {
        player = opponent_player;
    }

    for (int row = ROWS - N; row >= 0; row--) { // 2 -> 1 -> 0
        for (int col = 0; col <= COLS - N; col++) {   // 0 -> 1 -> 2 -> 3 
            if (!isValuableDiagonalRightCheck(row, col, filled_column_count)) {
                continue;
            }

            result = checkDiagonalRightCase1(player, board, row, col);
            if (result != -1) {
                return result;
            }
            result = checkDiagonalRightCase2(player, board, row, col);
            if (result != -1) {
                return result;
            }
            result = checkDiagonalRightCase3(player, board, row, col);
            if (result != -1) {
                return result;
            }
            result = checkDiagonalRightCase4(player, board, row, col);
            if (result != -1) {
                return result;
            }
        }
    }
    return -1;
}

int
isValuableDiagonalLeftCheck (int row, int col, int* filled_column_count)
{
    if (filled_column_count[col] <  (5 - row) ||
        filled_column_count[col - 1] < (4 - row) ||
        filled_column_count[col - 2] < (3 - row) ||
        filled_column_count[col - 3] < (2 - row)) {
        return false;
    }
    return true;
}

int
checkDiagonalLeftCase1 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row][col] == player &&
        board[row + 1][col - 1] == player &&
        board[row + 2][col - 2] == player) {
        if (isValidPlace(board, row + 3, col - 3)) {
            return col - 3;
        }
    }
    return -1;
}

int
checkDiagonalLeftCase2 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row][col] == player &&
        board[row + 1][col - 1] == player &&
        board[row + 3][col - 3] == player) {
        if (isValidPlace(board, row + 2, col - 2)) {
            return col - 2;
        }
    }
    return -1;
}

int
checkDiagonalLeftCase3 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row][col] == player &&
        board[row + 2][col - 2] == player &&
        board[row + 3][col - 3] == player) {
        if (isValidPlace(board, row + 1, col - 1)) {
            return col - 1;
        }
    }
    return -1;
}

int
checkDiagonalLeftCase4 (int player, int board[ROWS][COLS], int row, int col)
{
    if (board[row + 1][col - 1] == player &&
        board[row + 2][col - 2] == player &&
        board[row + 3][col - 3] == player) {
        if (isValidPlace(board, row, col)) {
            return col;
        }
    }
    return -1;
}

int
getDiagonalLeftCase (int is_offense, int ally_player, int opponent_player, int board[ROWS][COLS], int* filled_column_count)
{
    int result;
    int player;
    
    if (is_offense == true) {
        player = ally_player;
    }
    else {
        player = opponent_player;
    }

    for (int row = ROWS - N; row >= 0; row--) { // 2 -> 1 -> 0
        for (int col = COLS - 1; col >= COLS - N; col--) {   // 6 -> 5 -> 4 -> 3 
            if (!isValuableDiagonalLeftCheck(row, col, filled_column_count)) {
                continue;
            }

            result = checkDiagonalLeftCase1(player, board, row, col);
            if (result != -1) {
                return result;
            }
            result = checkDiagonalLeftCase2(player, board, row, col);
            if (result != -1) {
                return result;
            }
            result = checkDiagonalLeftCase3(player, board, row, col);
            if (result != -1) {
                return result;
            }
            result = checkDiagonalLeftCase4(player, board, row, col);
            if (result != -1) {
                return result;
            }
        }
    }
    return -1;
}

int
checkHorizontalCase1 (int player, int board[ROWS][COLS], int row)
{
    for (int col = 0; col <= COLS - N + 1; col++) {
        if (board[row][col] == player &&
            board[row][col + 1] == player&&
            board[row][col + 2] == player) {
            if (isValidPlace(board, row, col - 1)) {
                return col - 1;
            }
            else if (isValidPlace(board, row, col + 3)) {
                return col + 3;
            }
        }
    }
    return -1;
}

int
checkHorizontalCase2 (int player, int board[ROWS][COLS], int row)
{
    for (int col = 0; col <= COLS - N; col++) {
        if (board[row][col] == player &&
            board[row][col + 1] == player &&
            board[row][col + 3] == player) {
            if (isValidPlace(board, row, col + 2)) {
                return col + 2;
            }
        }
    }
    return -1;
}

int
checkHorizontalCase3 (int player, int board[ROWS][COLS], int row)
{
    for (int col = 0; col <= COLS - N; col++) {
        if (board[row][col] == player &&
            board[row][col + 2] == player &&
            board[row][col + 3] == player) {
            if (isValidPlace(board, row, col + 1)) {
                return col + 1;
            }
        }
    }
    return -1;
}

int
isValuableHorizontalCheck (int row, int* filled_row_count, int num)
{
    if (filled_row_count[row] < num ||
        filled_row_count[row] == COLS) {
        return false;
    }
    return true;
}

int
getHorizontalCase (int is_offense, int ally_player, int opponent_player, int board[ROWS][COLS], int* filled_row_count)
{
    int result;
    int player;
    
    if (is_offense == true) {
        player = ally_player;
    }
    else {
        player = opponent_player;
    }

    for (int row = ROWS - 1; row >= 0; row--)
    {   
        if (!isValuableHorizontalCheck(row, filled_row_count, 3)) {
            continue;
        }

        result = checkHorizontalCase1(player, board, row);
        if (result != -1) {
            return result;
        }
        result = checkHorizontalCase2(player, board, row);
        if (result != -1) {
            return result;
        }
        result = checkHorizontalCase3(player, board, row);
        if (result != -1) {
            return result;
        }
    }
    return -1;
}

int 
tryAttackOrDefense (bool is_offense, int ally_player, int opponent_player,
                    int board[ROWS][COLS], int filled_column_count[COLS], int filled_row_count[ROWS]) 
{
    int column;

    column = getVerticalCase(is_offense, ally_player, opponent_player, board, filled_column_count);
    if (column != -1) {
        return column;
    }

    column = getHorizontalCase(is_offense, ally_player, opponent_player, board, filled_row_count);
    if (column != -1) { 
        return column;
    }

    column = getDiagonalRightCase(is_offense, ally_player, opponent_player, board, filled_column_count);
    if (column != -1) {
        return column;
    }

    column = getDiagonalLeftCase(is_offense, ally_player, opponent_player, board, filled_column_count);
    if (column != -1) {
        return column;
    }

    return -1;
}

int
tryVerticalCase1 (int player, int board[ROWS][COLS], 
                            int* filled_column_count, int col)
{
    int top_index = ROWS - filled_column_count[col];

    if (board[top_index][col] == player &&
        board[top_index + 1][col] == player) {
        return col;
    }
    return -1;
}

int
tryVerticalCase (int is_offense, int ally_player, int opponent_player, int board[ROWS][COLS], int* filled_column_count)
{
    int result;
    int player;
    
    if (is_offense == true) {
        player = ally_player;
    }
    else {
        player = opponent_player;
    }

    for (int col = 0; col < COLS; col++)
    {   
        if (!isValuableVerticalCheck(col, filled_column_count, 2)) {
            continue;
        }

        result = tryVerticalCase1(player, board, filled_column_count, col);
        if (result != -1) {
            return result;
        }
    }
    return -1;
}

int
tryHorizontalCase1 (int player, int board[ROWS][COLS], int row)
{
    for (int col = 0; col < COLS - 1; col++) {
        if (board[row][col] == player &&
            board[row][col + 1] == player) {
            if (isValidPlace(board, row, col - 2)) {
                return col - 2;
            }
            else if (isValidPlace(board, row, col - 1)) {
                return col - 1;
            }
            else if (isValidPlace(board, row, col + 2)) {
                return col + 2;
            }
            else if (isValidPlace(board, row, col + 3)) {
                return col + 3;
            }
        }
    }
    return -1;
}

int
tryHorizontalCase2 (int player, int board[ROWS][COLS], int row)
{
    for (int col = 0; col <= COLS - N + 1; col++) {
        if (board[row][col] == player &&
            board[row][col + 2] == player) {
            if (isValidPlace(board, row, col - 1)) {
                return col - 1;
            }
            else if (isValidPlace(board, row, col + 1)) {
                return col + 1;
            }
            else if (isValidPlace(board, row, col + 3)) {
                return col + 3;
            }
        }
    }
    return -1;
}

int
tryHorizontalCase (int is_offense, int ally_player, int opponent_player, int board[ROWS][COLS], int* filled_row_count)
{
    int result;
    int player;
    
    if (is_offense == true) {
        player = ally_player;
    }
    else {
        player = opponent_player;
    }

    for (int row = ROWS - 1; row >= 0; row--)
    {   
        if (!isValuableHorizontalCheck(row, filled_row_count, 2)) {
            continue;
        }

        result = tryHorizontalCase1(player, board, row);
        if (result != -1) {
            return result;
        }
        result = tryHorizontalCase2(player, board, row);
        if (result != -1) {
            return result;
        }
    }
    return -1;
}

int 
tryMakeThreeStones (bool is_offense, int ally_player, int opponent_player,
                    int board[ROWS][COLS], int filled_column_count[COLS], int filled_row_count[ROWS]) 
{
    int column;

    column = tryVerticalCase(is_offense, ally_player, opponent_player, board, filled_column_count);
    if (column != -1) {
        return column;
    }

    column = tryHorizontalCase(is_offense, ally_player, opponent_player, board, filled_row_count);
    if (column != -1) {
        return column;
    }

    return -1;
}

char
getOutput (int column)
{
    switch (column) {
        case 0:
            return 'a';
        case 1:
            return 'b';
        case 2:
            return 'c';
        case 3:
            return 'd';
        case 4:
            return 'e';
        case 5:
            return 'f';
        case 6:
            return 'g';
        default:
            return ' ';
    }
}

void
randomTimeoutOccur ()
{
    fprintf(stderr, "[DEBUG] Simulate Timeout\n");
    sleep(10);
}

void nullDereferenceForTest ()
{
    fprintf(stderr, "[DEBUG] Runtime Error\n");
    int* ptr = NULL;
    *ptr = 1;
}

void divisionZeroForTest()
{
    fprintf(stderr, "[DEBUG] Runtime Error\n");
    int a = 1;
    int b = 0;
    int c = a / b;
    (void)c;
}

void recurseForTest ()
{
    return recurseForTest();
}

void
randomRunTimeErrorOccur ()
{
    int error_type = rand() % 3;
    switch (error_type) {
        case 0: {
            nullDereferenceForTest();
            break;
        }
        case 1: {
            divisionZeroForTest();
            break;
        }
        case 2: {
            fprintf(stderr, "[DEBUG] Runtime Error\n");
            recurseForTest();
            break;
        }
    }
}

void
occurRandomEventForTest ()
{
    int error_chance = rand() % 100;
    if (error_chance < 5) {
        randomTimeoutOccur();
    }
    else if (error_chance < 10) {
        randomRunTimeErrorOccur();
    }
}

int 
main ()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec + tv.tv_sec);

#ifdef TIMEOUT
    occurRandomEventForTest();
#endif

    int ally_player, opponent_player;
    int board[ROWS][COLS];
    if (!isSetInputInformationSuccess(&ally_player, &opponent_player, board)) {
        goto err_1;
    }

    //printCurrentPlayer(ally_player);
    //printBoard(ROWS, COLS, board);

    int* filled_column_count = getFilledColumnCount(board);
    int* filled_row_count = getFilledRowCount(board);
    if (filled_column_count == NULL ||
        filled_row_count == NULL) {
        goto err_2;
    }

    int column;

    column = tryAttackOrDefense(true, ally_player, opponent_player, board, filled_column_count, filled_row_count);
    if (column != -1) {
        goto success;
    }

    column = tryAttackOrDefense(false, ally_player, opponent_player, board, filled_column_count, filled_row_count);
    if (column != -1) {
        goto success;
    }

    column = tryMakeThreeStones(true, ally_player, opponent_player, board, filled_column_count, filled_row_count);
    if (column != -1) {
        goto success;
    }

    goto random;

    
success:
    if (column < 0 || column > 6) {
        goto err_3;
    }

    printf("%c\n", getOutput(column));
    free(filled_column_count);
    free(filled_row_count);
    return EXIT_SUCCESS;
random:
    do {
        column = rand() % COLS;
    } while (filled_column_count[column] >= ROWS);
    goto success;
err_1:
    fprintf(stderr, "ERROR :: Wrong Input Information.\n");
    exit(EXIT_FAILURE);
err_2:
    fprintf(stderr, "ERROR :: Memory Allocation Failed.\n");
    exit(EXIT_FAILURE);
err_3:
    fprintf(stderr, "ERROR :: Memory Reference Error.\n");
    exit(EXIT_FAILURE);
}
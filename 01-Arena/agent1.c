#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 6
#define COLS 7

char * err_msg ;

int * map[COLS] ;
int player_id ;
int opponent_id ;

typedef enum {
    BLANK,
    X,
    Y
} State ;

void
print_map ()
{
    int header = 1 ;
    printf("==== Map ====\n") ;
    for (int r = 0; r < ROWS; r++) {
        if (header) {
            for (int c = 0; c < COLS; c++) {
                printf("%c ", 'A' + c) ;
            }
            printf("\n") ;
        }
        header = 0 ;
        for (int c = 0; c < COLS; c++) {
            printf("%d ", map[c][r]) ;
        }
        printf("\n") ;
    }
    printf("=============\n") ;
}

int 
config_map (int * map[])
{
    int player ;
    // scanf("%d", &player) ;
    fread(&player, sizeof(int), 1, stdin) ;

    if (player == X) {
        player_id = X ;
        opponent_id = Y ;
    } else if (player == Y) {
        player_id = Y ;
        opponent_id = X ;
    } else {
        fprintf(stderr, "Player is either 1 or 2.\n") ;
        err_msg = "Config" ;
        return EXIT_FAILURE ;
    }

    for (int c = 0; c < COLS; c++) {
        map[c] = malloc(sizeof(int) * ROWS) ;
        if (map[c] == NULL) {
            fprintf(stderr, "Memory allocation failed\n") ;
            exit(EXIT_FAILURE) ;
        }
    } 

    for (int c = 0; c < COLS; c++) {
        if (fread(map[c], sizeof(int), ROWS, stdin) != ROWS) {
            fprintf(stderr, "Failed to read column %d\n", c) ;
            return EXIT_FAILURE ;
        }
    }
    
    return EXIT_SUCCESS ;
}

int 
check_win (int col, int row, int player)
{
    int count = 1 ;
    for (int c = col - 1; c >= 0 && map[c][row] == player; c--) {
        count++ ;
    }
    for (int c = col + 1; c < COLS && map[c][row] == player; c++) {
        count++ ;
    }
    if (count >= 4)
        return EXIT_SUCCESS ;

    count = 1 ;
    for (int r = row + 1; r < ROWS && map[col][r] == player; r++) {
        count++ ;
    }
    if (count >= 4) 
        return EXIT_SUCCESS ;
    
    count = 1 ;
    for (int c = col - 1, r = row + 1; c >= 0 && r < ROWS && map[c][r] == player; c--, r++) {
        count++ ;
    }
    for (int c = col + 1, r = row - 1; c < COLS && r >= 0 && map[c][r] == player; c++, r--) {
        count++ ;
    }
    if (count >= 4) 
        return EXIT_SUCCESS ;
    
    count = 1 ;
    for (int c = col + 1, r = row + 1; c < COLS && r < ROWS && map[c][r] == player; c++, r++) {
        count++ ;
    }
    for (int c = col - 1, r = row - 1; c >= 0 && r >= 0 && map[c][r] == player; c--, r--) {
        count++ ;
    }
    if (count >= 4) 
        return EXIT_SUCCESS ;

    return EXIT_FAILURE ;
}

int
select_column (int * selected) 
{
    for (int c = 0; c < COLS; c++) {
        for (int r = ROWS - 1; r >= 0; r--) {
            if (map[c][r] == BLANK) {
                map[c][r] = player_id ;
                if (!(check_win(c, r, player_id))) {
                    *(selected) = c ;
#ifdef PRINT
                    printf("I am going to win!\n") ;
#endif
                    return EXIT_SUCCESS ;
                }
                map[c][r] = BLANK ;
                break ;
            }
        }
    }

    for (int c = 0; c < COLS; c++) {
        for (int r = ROWS - 1; r >= 0; r--) {
            if (map[c][r] == BLANK) {
                map[c][r] = opponent_id ;
                if (!(check_win(c, r, opponent_id))) {
                    map[c][r] = player_id ;
                    *(selected) = c ;
#ifdef PRINT
                    printf("I am going to block the opponent!\n") ;
#endif
                    return EXIT_SUCCESS ;
                }
                map[c][r] = BLANK ;
                break ;
            }
        }
    }

    const int column_priority[COLS] = {3, 2, 4, 1, 5, 0, 6};
    int best_score = -1, best_col = -1 ;
    for (int c = 0; c < COLS; c++) {
        if (map[c][0] != BLANK) {
            continue ;
        }

        int r ;
        for (r = ROWS - 1; r >= 0 && map[c][r] != BLANK; r--) ;
        if (r < 0) {
            continue ;
        }

        map[c][r] = player_id ;

        int score = 0, count = 1 ;
        for (int i = -1; i <= 1; i += 2) {
            int j = c + i ;
            if (j >= 0 && j < COLS && map[j][r] == player_id) {
                count++ ;
            }
        }
        if (count == 2) score += 2 ;

        if (r + 1 < ROWS && map[c][r + 1] == player_id) {
            score += 2 ;
        }

        score += (3 - abs(3 - c)) ;

        map[c][r] = BLANK ;

        if (score > best_score) {
            best_score = score ;
            best_col = c ;
        }
    }
    
    if (best_col != -1) {
        for (int r = ROWS - 1; r >= 0; r--) {
            if (map[best_col][r] == BLANK) {
                map[best_col][r] = player_id ;
                break ;
            }
        }
        *(selected) = best_col ;
#ifdef PRINT
        printf("Selecting the most potential column!\n") ;
#endif
        return EXIT_SUCCESS ;
    }

    int available[COLS], count = 0 ;
    for (int c = 0; c < COLS; c++) {
        if (map[c][0] == BLANK) {
            available[count++] = c ;
        }
    }
    if (count > 0) {
        int rand_idx = rand() % count ;
        *(selected) = available[rand_idx] ;
        for (int r = ROWS - 1; r >= 0; r--) {
            if (map[available[rand_idx]][r] == BLANK) {
                map[available[rand_idx]][r] = player_id ;
#ifdef PRINT
                printf("I am selecting randomly!\n") ;
#endif
                return EXIT_SUCCESS ;
            }
        }
    }
    
    err_msg = "Selection" ;
    return EXIT_FAILURE ;
}

int
main (int argc, char * argv[])
{
#ifdef TEST
    FILE *fp = fopen("map.txt", "r+") ;
    if (!fp) {
        fprintf(stderr, "Error opening map file.\n") ;
        exit(EXIT_FAILURE) ;
    }

    int player ;
    fscanf(fp, "%d", &player) ;

    // Set player_id and opponent_id
    if (player == X) {
        player_id = X ;
        opponent_id = Y ;
    } else if (player == Y) {
        player_id = Y ;
        opponent_id = X ;
    } else {
        fprintf(stderr, "Invalid player ID in map file.\n") ;
        exit(EXIT_FAILURE) ;
    }

    for (int c = 0; c < COLS; c++) {
        map[c] = malloc(sizeof(int) * ROWS) ;
        if (map[c] == NULL) {
            fprintf(stderr, "Memory allocation failed\n") ;
            exit(EXIT_FAILURE) ;
        }
    } 

    // Read map from file
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            fscanf(fp, "%d", &map[c][r]) ;
        }
    }

    print_map() ;
#else
    if (config_map(map)) {
        goto clean_up ;
    }
#endif

#ifdef PRINT
    print_map() ;
#endif

    srand(time(NULL)) ;
    int selected ;
    if (select_column(&selected)) {
        goto clean_up ;
    }

#ifdef PRINT
    printf("Selected column: ") ;
#endif
    fprintf(stdout, "%c\n", 'A' + selected) ;
    fflush(stdout) ;
    
#ifdef TEST
    fseek(fp, 0, SEEK_SET) ;
    fprintf(fp, "%d\n", opponent_id) ;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            fprintf(fp, "%d ", map[c][r]) ;
        }
        fprintf(fp, "\n") ;
    }
    
    fclose(fp) ;
#endif

#ifdef PRINT
    print_map() ;
#endif

    for (int i = 0; i < COLS; i++) {
        free(map[i]) ; 
    }

    return EXIT_SUCCESS ;
    
clean_up :
    for (int i = 0; i < COLS; i++) {
        free(map[i]) ; 
    }

err : 
    fprintf(stderr, "%s failed", err_msg) ;
    return EXIT_FAILURE ;
}
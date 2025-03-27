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

int 
config_map (int * map[])
{
    int player ;
    scanf("%d", &player) ;

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

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            scanf("%d", &map[c][r]) ;
        }
    }
    
    return EXIT_SUCCESS ;
}

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
                    printf("I am going to win!\n") ;
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
                    printf("I am going to block the opponent!\n") ;
                    return EXIT_SUCCESS ;
                }
                map[c][r] = BLANK ;
                break ;
            }
        }
    }

    const int column_priority[COLS] = {3, 2, 4, 1, 5, 0, 6};
    for (int i = 0; i < COLS; i++) {
        int c = column_priority[i] ;
        if (map[c][0] == BLANK) { 
            for (int r = ROWS - 1; r >= 0; r--) {
                if (map[c][r] == BLANK) {
                    map[c][r] = player_id ;
                    break ;
                }
            }
            *(selected) = c ;
            printf("Picking good column!\n") ;
            return EXIT_SUCCESS;
        }
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
                printf("I am selecting randomly!\n") ;
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
    if (config_map(map)) {
        goto clean_up ;
    }

    print_map() ;
    
    srand(time(NULL)) ;
    int selected ;
    if (select_column(&selected)) {
        goto clean_up ;
    }
    printf("Selected column: ") ;
    fprintf(stdout, "%c\n", 'A' + selected) ;
    
    print_map() ;

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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define n_stacks 7
#define stack_cap 6

static int this_player ;
static int board[stack_cap + 1][n_stacks + 1] ;
static int stones_count ;

char stack_name (int i)
{
        return 'A' + i - 1 ;
}

int stack_num (char c) 
{
        return c - 'A' + 1 ;
}

void null_dereference_error ()
{
    int* ptr = NULL ;
    *ptr = 1 ;
}

void division_zero_error()
{
    int a = 1 ;
    int b = 0 ;
    int c = a / b ;
    (void)c ;
}

void recurse_error ()
{
    return recurse_error() ;
}

void run_time_error_occur ()
{
    int error_type = rand() % 3 ;
    switch (error_type) {
        case 0: {
            null_dereference_error() ;
            break ;
        }
        case 1: {
            division_zero_error() ;
            break;
        }
        case 2: {
            recurse_error() ;
            break ;
        }
    }
}

int main ()
{
        scanf("%d", &this_player) ;
        if (this_player != 1 && this_player != 2)
                return EXIT_FAILURE ;

        for (int l = stack_cap ; l >= 1 ; l--) { 
                for (char s = 'A' ; s <= 'G' ; s++) { 
                        scanf("%d", &(board[l][stack_num(s)])) ;
                }
        }

        stones_count = 0 ;
        for (int l = stack_cap; l >= 1; l--) {
                for (char s = 'A'; s <= 'G'; s++) {
                    if (board[l][stack_num(s)] != 0) {
                        stones_count ++ ;
                    } 
                }
        }

        if (stones_count >= 6) {
            run_time_error_occur ();
        }

        srand(time(NULL)) ;
        char choice = rand() % n_stacks + 1 ;
        printf("%c", stack_name(choice)) ;

        return EXIT_SUCCESS ;
}

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

void timeout_occur ()
{
    sleep(5);
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
                timeout_occur() ;
        }

        srand(time(NULL)) ;
        char choice = rand() % n_stacks + 1 ;
        printf("%c", stack_name(choice)) ;

        return EXIT_SUCCESS ;
}

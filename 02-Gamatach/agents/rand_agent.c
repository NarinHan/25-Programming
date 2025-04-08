#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define n_stacks 7 
#define stack_cap 6

static int this_player ;
static int board[stack_cap + 1][n_stacks + 1] ; 

char stack_name (int i)
{
	return 'A' + i - 1 ;
}

int stack_num (char c)
{
	return c - 'A' + 1 ;
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
	
	// srand(time(NULL)) ;
	srand(1) ;
	char choice = rand() % n_stacks + 1 ;
	printf("%c", stack_name(choice)) ;

	return EXIT_SUCCESS ;
}

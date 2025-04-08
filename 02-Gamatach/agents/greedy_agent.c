#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define n_stacks 7 
#define stack_cap 6

static int this_player ;
static int board[stack_cap + 1][n_stacks + 1] ;
static int top[n_stacks + 1] = {0, 1, 1, 1, 1, 1, 1, 1} ;

int const upward = 1 ;
int const downward = -1 ;
int const leftward = -1 ;
int const rightward = 1 ;

int _count (int stack, int level, int hdir, int vdir, int player)
{
	int count = 0 ;
	while (1 <= stack && stack <= n_stacks && 
	       1 <= level && level <= stack_cap) {
		if (board[level][stack] == player) {
			count++ ;
			stack += hdir ;
			level += vdir ;
		}
		else {
			break ;
		}
	}
	return count ; 
}

int count_down (int stack, int level, int player) 
{
	return _count(stack, level + downward, 0, downward, player) ;
}

int count_left (int stack, int level, int player) 
{
	return _count(stack + leftward, level, leftward, 0, player) ;
}

int count_right (int stack, int level, int player) 
{
	return _count(stack + rightward, level, rightward, 0, player) ;
}

int count_down_left (int stack, int level, int player) 
{
	return _count(stack + leftward, level + downward, leftward, downward, player) ;
}

int count_down_right (int stack, int level, int player)
{
	return _count(stack + rightward, level + downward, rightward, downward, player) ;
}

int count_up_left (int stack, int level, int player) 
{
	return _count(stack + leftward, level + upward, leftward, upward, player) ;
}

int count_up_right (int stack, int level, int player)
{
	return _count(stack + rightward, level + upward, rightward, upward, player) ;
}

int greedy_offense (int player)
{
	int stack ;

	for (stack = 1 ; stack <= n_stacks ; stack++) {
		if (top[stack] > stack_cap /* fully-filled */) 
			continue ;

		if (count_down(stack, top[stack], player) == 3)
			return stack ;

		if (count_left(stack, top[stack], player) + count_right(stack, top[stack], player) == 3)
			return stack ;

		if (count_down_left(stack, top[stack], player) + count_up_right(stack, top[stack], player) == 3)
			return stack ;

		if (count_up_left(stack, top[stack], player) + count_down_right(stack, top[stack], player) == 3)
			return stack ;
	}
	return 0 ;
}

int greedy_defense (int player) 
{
	int other_player = 3 - player ;

	int stack ;
	for (stack = 1 ; stack <= n_stacks ; stack++) {
		if (top[stack] == stack_cap /* fully-filled */) 
			continue ;

		if (count_down(stack, top[stack], other_player) == 3)
			return stack ;

		if (count_left(stack, top[stack], other_player) + count_right(stack, top[stack], other_player) == 3)
			return stack ;

		if (count_down_left(stack, top[stack], other_player) + count_up_right(stack, top[stack], other_player) == 3)
			return stack ;

		if (count_up_left(stack, top[stack], other_player) + count_down_right(stack, top[stack], other_player) == 3)
			return stack ;
	}

	return 0 ;
}

int main ()
{
	int i, j ; 

	scanf("%d", &this_player) ;
	if (this_player != 1 && this_player != 2) 
		return EXIT_FAILURE ;

	for (i = stack_cap ; i > 0 ; i--) {
		for (j = 1 ; j <= n_stacks ; j++) {
			scanf("%d", &(board[i][j])) ;
			if (board[i][j] != 0 && top[j] == 1) {
				top[j] = i + 1 ;
			}
		}
	}

	char choice ;
	if (choice = greedy_offense(this_player)) {
	 	printf("%c", 'A' + choice - 1) ;
		return EXIT_SUCCESS ;
	}

	if (choice = greedy_defense(this_player)) {
		printf("%c", 'A' + choice - 1) ;
		return EXIT_SUCCESS ;
	}
	
	srand(time(NULL)) ;
	choice = rand() % n_stacks ;
	printf("%c", 'A' + choice) ;

	return EXIT_SUCCESS ;
}

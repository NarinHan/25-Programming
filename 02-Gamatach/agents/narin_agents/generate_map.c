#include <stdio.h>
#include <unistd.h>

#define ROWS 6
#define COLS 7

int main ()
{
	FILE *fp = fopen("map.txt", "w") ;
	if (!fp) return -1 ;
	
	fprintf(fp, "1\n") ;

	for (int r = 0; r < ROWS; r++) {
		for (int c = 0; c < COLS; c++) {
			fprintf(fp, "0 ") ;
		}
		fprintf(fp, "\n") ;
	}  

	fclose(fp) ;
	return 0 ;
}

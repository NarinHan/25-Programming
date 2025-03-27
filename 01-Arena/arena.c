#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define ROWS 6
#define COLS 7
#define BUFSIZE 512 

typedef enum {
    BLANK,
    X,
    Y
} State ;

typedef struct {
    int * map[COLS] ;
    int player_id ;
    int opponent_id ;
    int turn ;
} config_t ;

config_t config ;

int winner ;

char * err_msg ;

int ptoc_fd[2] ;
int ctop_fd[2] ;

int
error_exit (char * format) 
{
    err_msg = format ;
    return EXIT_FAILURE ;
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
            printf("%d ", config.map[c][r]) ;
        }
        printf("\n") ;
    }
    printf("=============\n") ;
}

int
allocate_map (config_t * config)
{
    for (int c = 0; c < COLS; c++) {
        config->map[c] = malloc(sizeof(int) * ROWS) ;
        if (config->map[c] == NULL) {
            fprintf(stderr, "Memory allocation failed\n") ;
            exit(EXIT_FAILURE) ;
        }
    } 

    return EXIT_SUCCESS ;
}

int 
select_random_player ()
{
    srand(time(NULL)) ;
    int idx = rand() % 2 + 1 ;

    return idx ;
}

int 
write_bytes (int dest, const config_t config)
{
    char buf[BUFSIZE] ;
    int offset = 0 ;

    memcpy(buf + offset, &(config.player_id), sizeof(int)) ;
    offset += sizeof(int) ;

    for (int c = 0; c < COLS; c++) {
        memcpy(buf + offset, config.map[c], sizeof(int) * ROWS) ;
        offset += sizeof(int) * ROWS ;
    }

    int total = 0, written ;
    int towrite = offset ;

    while (total < towrite) {
        written = write(dest, buf + total, towrite - total) ;
        if (written <= 0) {
            err_msg = "Writing to the agent" ;
            return - 1 ;
        }
        total += written ;
    }

    return total ;
}

int
init_config (config_t * config) 
{
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r < ROWS; r++) {
            config->map[c][r] = 0 ;
        }
    }

    switch (config->player_id) {
        case X :
            config->opponent_id = Y ;
            break ;
        case Y :
            config->opponent_id = X ;
            break ;
        default :
            return error_exit("Setting player ID in the first round") ;
    }

    return EXIT_SUCCESS ;
}

int
update_map (char selected, config_t * config)
{
    printf("turn: %d\n", config->turn) ;
    printf("player now: %d\n", config->player_id) ;
    printf("selected: %c\n", selected) ;
    int selected_num = selected - 'A' ;

    for (int r = ROWS - 1; r >= 0; r--) {
        if (config->map[selected_num][r] == BLANK) {
            config->map[selected_num][r] = config->player_id ;
            break ;
        }
    } 
    
    config->turn++ ;

    return EXIT_SUCCESS ;
}

int 
update_player (config_t * config)
{
    switch (config->player_id) {
        case X : 
            config->player_id = Y ;
            config->opponent_id = X ;
            break ;
        case Y : 
            config->player_id = X ;
            config->opponent_id = Y ;
            break ;
        default :
            return error_exit("Setting player ID") ;
    }
    
    return EXIT_SUCCESS ;
}



int 
check_win (int col, int row, config_t config)
{
    int count = 1 ;
    int player = config.player_id ;
    
    for (int c = col - 1; c >= 0 && config.map[c][row] == player; c--) {
        count++ ;
    }
    for (int c = col + 1; c < COLS && config.map[c][row] == player; c++) {
        count++ ;
    }
    if (count >= 4)
        return EXIT_SUCCESS ;

    count = 1 ;
    for (int r = row + 1; r < ROWS && config.map[col][r] == player; r++) {
        count++ ;
    }
    if (count >= 4) 
        return EXIT_SUCCESS ;
    
    count = 1 ;
    for (int c = col - 1, r = row + 1; c >= 0 && r < ROWS && config.map[c][r] == player; c--, r++) {
        count++ ;
    }
    for (int c = col + 1, r = row - 1; c < COLS && r >= 0 && config.map[c][r] == player; c++, r--) {
        count++ ;
    }
    if (count >= 4) 
        return EXIT_SUCCESS ;
    
    count = 1 ;
    for (int c = col + 1, r = row + 1; c < COLS && r < ROWS && config.map[c][r] == player; c++, r++) {
        count++ ;
    }
    for (int c = col - 1, r = row - 1; c >= 0 && r >= 0 && config.map[c][r] == player; c--, r--) {
        count++ ;
    }
    if (count >= 4) 
        return EXIT_SUCCESS ;

    return EXIT_FAILURE ;
}

int 
judge (config_t config)
{
    for (int c = 0; c < COLS; c++) {
        for (int r = ROWS - 1; r >= 0; r--) {
            if (config.map[c][r] == config.player_id) {
                if (!(check_win(c, r, config))) {
                    winner = config.player_id ;
                    printf("WINNER: %d\n", winner) ;
                    return EXIT_SUCCESS ;
                }
            }
        }
    }

    return EXIT_FAILURE ;
}

int
run_child ()
{
    if (close(ptoc_fd[1]) == -1) 
        return error_exit("Close for ptoc write end") ;

    if (close(ctop_fd[0]) == -1) 
        return error_exit("Close for ctop read end") ;
    
    if (dup2(ptoc_fd[0], STDIN_FILENO) < 0) 
        return error_exit("dup2 stdin") ;
    close(ptoc_fd[0]) ;
    
    if (dup2(ctop_fd[1], STDOUT_FILENO) < 0) 
        return error_exit("dup2 stdout") ;
    close(ctop_fd[1]) ;

    if (config.player_id == X)
        execlp("./agent1", "agent1", (char *) NULL) ;
    else
        execlp("./agent2", "agent2", (char *) NULL) ;
    
    return error_exit("Executing agent") ;
}

int
run_parent () 
{
    if (close(ptoc_fd[0]) == -1) 
        return error_exit("Close for ptoc read end") ;
    if (close(ctop_fd[1]) == -1) 
        return error_exit("Close for ctop write end") ;
    
    if (config.turn == 0) {
        config.player_id = select_random_player() ;
        if (init_config(&config)) 
            return error_exit("Initializing the player ID and the map") ;
    }
        
    int write_chk = write_bytes(ptoc_fd[1], config) ;
    if (write_chk < 0) 
        return error_exit("Write to child") ;
    close(ptoc_fd[1]) ;

    char buf[BUFSIZE] ;
    int read_chk = read(ctop_fd[0], buf, BUFSIZE) ;
    if (read_chk <= 0) 
        return error_exit("Read") ;
    printf("buf: %s\n", buf) ;
    
    char selected ;
    sscanf(buf, "%c", &selected) ;

    printf("read chk: %d\n", read_chk) ;
    printf("selected column: %c\n", selected) ;
    
    int status ;
    if (wait(&status) == -1) 
        return error_exit("Wait") ;
    
    if (update_map(selected, &config)) 
        return error_exit("Updating the map with received input") ;
    
    print_map() ;
    
    if (!(judge(config))) 
        return EXIT_SUCCESS ;

    if (update_player(&config))
        return error_exit("Updating the player ID") ;

    return EXIT_SUCCESS ;
}

int 
main (int argc, char * argv[])
{
    if (allocate_map(&config)) {
        goto clean_up ;
    }

    pid_t agent ;
    while (config.turn < (ROWS * COLS) && winner == 0) {
        if (pipe(ptoc_fd) == -1) 
            return error_exit("Pipe ptoc") ;
        if (pipe(ctop_fd) == -1) 
            return error_exit("Pipe ctop") ;

        switch (agent = fork()) {
            case -1 :
                return error_exit("Fork") ;

            case 0 : 
                if (run_child()) 
                    goto clean_up ;

            default : 
                if (run_parent())
                    goto clean_up ;
        }        
    }

    return EXIT_SUCCESS ;

clean_up :
    for (int i = 0; i < COLS; i++) {
        if (config.map[i])
            free(config.map[i]) ;
    }

err : 
    fprintf(stderr, "%s failed\n", err_msg) ;
    return EXIT_FAILURE ;
}
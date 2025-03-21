#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#define NAMELEN 1024
#define BUFSIZE 1024
#define PRINT_ON 1
#define PRINT_OFF 0

typedef struct config_t {
    int timelim ;
    char input_dir[NAMELEN] ;
    char output_dir[NAMELEN] ;
    char target_src[NAMELEN] ;
    struct timeval tv_start ;
    struct timeval tv_end ;
    struct itimerval timer ;
    struct sigaction timer_sa ;
} config_t ;

config_t config ;

pid_t child_pid = -1 ;

typedef struct result_t {
    int timeout_cnt ;
    int compile_err_cnt ;
    int runtime_err_cnt ; 
    int wrong_cnt ;
    int correct_cnt ; 
    long time_acc ;
} result_t ;

result_t result ;

int 
error_exit(const char *format) 
{
    fprintf(stderr, "Error: %s failed.\n", format) ;
    return EXIT_FAILURE ;
}

void
timeout_handler(int signum) 
{
    if (child_pid > 0) {
        printf("Time limit exceeded! Terminating child process %d...\n", child_pid) ;
        result.timeout_cnt++ ;
        kill(child_pid, SIGKILL) ;
    }
}

int 
parse_arg(int argc, char * argv[], config_t * config)
{
    int opt ;
    while ((opt = getopt(argc, argv, "i:a:t:")) != -1) {
        switch (opt) {
            case 'i':
                if (strlen(optarg) < NAMELEN) {
                    memcpy(config->input_dir, optarg, NAMELEN) ;
                    config->input_dir[NAMELEN - 1] = '\0' ;
                } else {
                    fprintf(stderr, "Error: The length of the name of the directory should be less than 1024.\n") ;
                    return EXIT_FAILURE ;
                }
                if (strncmp(optarg, "-a", 2) == 0 || strncmp(optarg, "-t", 2) == 0) {
                    fprintf(stderr, "Error: Check the name of the input directory.\n") ;
                    return EXIT_FAILURE ;
                }
                break ;
            case 'a':
                if (strlen(optarg) < NAMELEN) {
                    memcpy(config->output_dir, optarg, NAMELEN) ;
                    config->output_dir[NAMELEN - 1] = '\0' ;
                } else {
                    fprintf(stderr, "Error: The length of the name of the directory should be less than 1024.\n") ;
                    return EXIT_FAILURE ;
                }
                if (strncmp(optarg, "-i", 2) == 0 || strncmp(optarg, "-t", 2) == 0) {
                    fprintf(stderr, "Error: Check the name of the output directory.\n") ;
                    return EXIT_FAILURE ;
                }
                break ;
            case 't':
                if (strncmp(optarg, "-i", 2) == 0 || strncmp(optarg, "-a", 2) == 0) {
                    fprintf(stderr, "Error: Check the time.\n") ;
                    return EXIT_FAILURE ;
                }
                config->timelim = atoi(optarg) ;
                break ;
            case '?':
                fprintf(stderr, "Usage: %s -i <inputdir> -a <outputdir> -t <timelimit> <target src>\n", argv[0]) ;
                return EXIT_FAILURE ;
        }
    }

    if (strlen(config->input_dir) == 0 || strlen(config->output_dir) == 0 || config->timelim <= -1) {
        fprintf(stderr, "Error: Missing required arguments. Please check.\n") ;
        return EXIT_FAILURE ;
    }

    if (optind >= argc) {
        fprintf(stderr, "Usage: %s -i <inputdir> -a <outputdir> -t <timelimit> <target src>\n", argv[0]) ;
        return EXIT_FAILURE ; 
    }

    memcpy(config->target_src, argv[optind], NAMELEN) ;
    config->target_src[NAMELEN - 1] = '\0' ;

    if (strlen(config->target_src) == 0) {
        fprintf(stderr, "Error: Missing target source file name. Please check.\n") ;
        return EXIT_FAILURE ; 
    } else if (strlen(config->target_src) > NAMELEN) {
        fprintf(stderr, "Error: The length of the name of the directory should be less than 50.\n") ;
        return EXIT_FAILURE ; 
    }

    printf("[CHECK] Argument parsing results:\n") ;
    printf("- Input directory: %s\n", config->input_dir) ;
    printf("- Answer directory: %s\n", config->output_dir) ;
    printf("- Time limit: %d\n", config->timelim) ;
    printf("- Target source file name: %s\n", config->target_src) ;

    return EXIT_SUCCESS ;
}

int
count_files(const char * path) 
{
    struct dirent *entry ;
    DIR *dir = opendir(path) ;
    int cnt = 0 ;

    if (dir == NULL)
        return -1 ;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) 
            cnt++ ; 
    }
    closedir(dir) ;

    return cnt ;
}

int
check_directory(const char * path)
{
    int file_cnt = count_files(path) ;
    if (file_cnt <= 0) {
        fprintf(stderr, "Error: Files do not exist in the given input directory.\n") ;
        return EXIT_FAILURE ;
    }
    
    return EXIT_SUCCESS ;
}

int
set_timer(config_t * config) 
{
    config->timer.it_value.tv_sec = config->timelim ;
    config->timer.it_value.tv_usec = 0 ;
    config->timer.it_interval.tv_sec = 0 ;
    config->timer.it_interval.tv_usec = 0 ;
    if ((setitimer(ITIMER_REAL, &(config->timer), NULL)) < 0)
        return EXIT_FAILURE ;

    return EXIT_SUCCESS ;
}

int
set_sigaction(config_t * config)
{
    if ((sigemptyset(&(config->timer_sa.sa_mask))) < 0) 
        return EXIT_FAILURE ;
    config->timer_sa.sa_flags = 0 ;
    config->timer_sa.sa_handler = timeout_handler ;

    return EXIT_SUCCESS ;
}

int 
compile_target(config_t * config) 
{
    printf("... processing ... compilation ...\n") ;
   
    execlp("gcc", "gcc", "-fsanitize=address", "-o", "target", config->target_src, (char *) NULL) ;
    error_exit("execlp") ; /* if we get here, something went wrong */

    return EXIT_FAILURE ;
}

int
run_compilation_child()
{
    int status ; 
    switch (child_pid = fork()) {
        case -1:
            return error_exit("Fork for compilation process") ;
        
        case 0:
            compile_target(&config) ;
            break;
        
        default:
            if (wait(&status) == -1)
                return error_exit("Wait for compilation process") ;

            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                result.compile_err_cnt++ ;
                // TODO: compile error message should be printed
                return error_exit("Compilation") ;
            }
            break ;
    }

    return EXIT_SUCCESS ;
}

int 
write_bytes(char *filepath, int dest_fd) 
{
    char buf[BUFSIZE] = {0} ;
    size_t read_chk, write_chk = 0 ;

    int fd = open(filepath, O_RDONLY) ;
    if (fd == -1) {
        fprintf(stderr, "Error opening file.\n") ;
        return -1 ;
    }
    while ((read_chk = read(fd, buf, BUFSIZE)) > 0) {
        write_chk += write(dest_fd, buf, read_chk) ;
        memset(buf, 0, BUFSIZE) ;
    }
    if (read_chk == -1) {
        fprintf(stderr, "Error reading file.\n") ;
        close(fd) ;
        return -1 ;
    }
    close(fd) ;

    return write_chk ;
}

int 
read_bytes(int src_fd) 
{
    char buf[BUFSIZE] = {0} ;
    size_t read_chk, read_all = 0 ; 
    
    while ((read_chk = read(src_fd, buf, BUFSIZE)) > 0) {
        read_all += read_chk ;
        memset(buf, 0, BUFSIZE) ;
    }
    if (read_chk == -1) {
        fprintf(stderr, "Error reading.\n") ;
        return -1 ;
    }

    return read_all ;
}

long
calculate_exec_time(result_t * result)
{
    long seconds = config.tv_end.tv_sec - config.tv_start.tv_sec ;
    long microseconds = config.tv_end.tv_usec - config.tv_start.tv_usec ;
    long elapsed_ms = (seconds * 1000) + (microseconds / 1000) ;
    result->time_acc += elapsed_ms ;

    return elapsed_ms ;
}

int 
print_results()
{
    printf("=============== TOTAL RESULTS ===============\n") ;
    printf("Compile error                       : %d\n", result.compile_err_cnt) ;
    printf("Timeout                             : %d\n", result.timeout_cnt) ;
    printf("Runtime error                       : %d\n", result.runtime_err_cnt) ;
    printf("Wrong answer                        : %d\n", result.wrong_cnt) ;
    printf("Correct answer                      : %d\n", result.correct_cnt) ;
    printf("Running time (correct answers only) : %ld ms\n", result.time_acc) ;
    printf("=============================================\n") ;

    return EXIT_SUCCESS ;
}

int
read_and_compare_results(int src_fd, char * filename) 
{
    char read_buf[BUFSIZE] = {0}, file_buf[BUFSIZE] = {0} ;
    size_t read_chk, read_all = 0 ; 

    char answer_filepath[BUFSIZE] ;
    snprintf(answer_filepath, sizeof(answer_filepath), "%s/%s", config.output_dir, filename) ;

    int fd = open(answer_filepath, O_RDONLY) ;
    if (fd == -1) {
        fprintf(stderr, "Error opening answer file.\n") ;
        return EXIT_FAILURE ;
    }
    size_t fread_chk ;
    
    while ((read_chk = read(src_fd, read_buf, BUFSIZE)) > 0) {
        fread_chk = read(fd, file_buf, BUFSIZE) ;
        if (fread_chk == -1) {
            fprintf(stderr, "Error reading.\n") ;
            return EXIT_FAILURE ;
        } 
    
        if (memcmp(read_buf, file_buf, fread_chk) != 0) { 
            result.wrong_cnt++ ;
            return EXIT_FAILURE ;
        }

        read_all += read_chk ;  
        memset(read_buf, 0, BUFSIZE) ;
        memset(file_buf, 0, BUFSIZE) ;
    }
    if (read_chk == -1) {
        fprintf(stderr, "Error reading.\n") ;
        return EXIT_FAILURE ;
    }
    
    close(fd) ;
    
    result.correct_cnt++ ;

    return EXIT_SUCCESS ;
}

int
run_execution_child()
{
    struct dirent *entry ;
    DIR *dir = opendir(config.input_dir) ;
    if (!(dir))
        return error_exit("Opening directory") ;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            int ptoc_fd[2] ; // parent writes, child reads
            if (pipe(ptoc_fd) == -1) /* create the pipe */
                return error_exit("Pipe ptoc") ;
            
            int ctop_fd[2] ; // child writes, parent reads 
            if (pipe(ctop_fd) == -1)
                return error_exit("Pipe ctop") ;

            int time_fd[2] ; // pipe for time checking; child writes, parent reads
            if (pipe(time_fd) == -1) 
                return error_exit("Pipe time") ;

            // create child process for execution
            switch (child_pid = fork()) {
                case -1:
                    return error_exit("Fork for executing process") ;
                
                case 0: // child
                    if (close(ptoc_fd[1]) == -1) // close unused write end for child
                        return error_exit("Close for ptoc write end") ;
                    
                    if (close(ctop_fd[0]) == -1) // close unused read end for child
                        return error_exit("Close for ctop read end") ;

                    if (close(time_fd[0]) == -1)
                        return error_exit("Close for time pipe read end") ;
    
                    if (dup2(ptoc_fd[0], STDIN_FILENO) < 0) // redirect stdin to read from ptoc pipe
                        return error_exit("dup2 stdin") ;
                    close(ptoc_fd[0]) ;

                    if (dup2(ctop_fd[1], STDOUT_FILENO) < 0)  // redirect stdout to write to ctop pipe
                        return error_exit("dup2 stdout") ;
                    close(ctop_fd[1]) ;

                    if (gettimeofday(&(config.tv_start), NULL) == -1)
                        return error_exit("gettimeofday") ;
                    if (write(time_fd[1], &(config.tv_start), sizeof(struct timeval)) < 0)
                        return error_exit("Write starting time of the execution") ;
                    close(time_fd[1]) ;

                    execlp("./target", "target", (char *) NULL) ;
                    return error_exit("execlp") ; /* if we get here, something went wrong */
                
                default: // parent
                    if (sigaction(SIGALRM, &(config.timer_sa), NULL) == -1) // set up timer
                        return error_exit("sigaction") ;

                    if (close(ptoc_fd[0]) == -1) // close unused read end for parent
                        return error_exit("Close for ptoc read end") ;
                
                    if (close(ctop_fd[1]) == -1) // close unused write end for parent
                        return error_exit("Close for ctop write end") ;
                    
                    if (close(time_fd[1]) == -1)
                        return error_exit("Close for time pipe write end") ;
                    
                    // receive the starting time from the child
                    if (read(time_fd[0], &(config.tv_start), sizeof(struct timeval)) < 0) 
                        return error_exit("Read starting time of the execution") ;
                    close(time_fd[0]) ;
    
                    char input_filepath[BUFSIZE] ;
                    snprintf(input_filepath, sizeof(input_filepath), "%s/%s", config.input_dir, entry->d_name) ;

                    // write_bytes() will read the file and write the data to the pipe
                    int write_chk = write_bytes(input_filepath, ptoc_fd[1]) ;
                    if (write_chk < 0)
                        return error_exit("Write to child") ;
                    close(ptoc_fd[1]) ;
    
                    int status, ret ;
                    /* reference from [21.5] Interruption and Restarting of System Calls */
                    /* when the signal handler returns, blocking system call fails with the error EINTR */
                    /* the following method is to continue the execution of an interrupted system call */
                    /* by manually restarting a systetm call in the event that is interrupted by a signal handler */
                    do {
                        ret = waitpid(child_pid, &status, 0) ;
                    } while (ret == -1 && errno == EINTR) ;
                    
                    if (ret == -1)
                        return error_exit("Wait for execution process") ;

                    if (WIFEXITED(status)) {
                        int exit_code = WEXITSTATUS(status) ;
                        if (exit_code != 0) {
                            int read_chk = read_bytes(ctop_fd[0]) ;
                            if (read_chk < 0)
                                return error_exit("Read from child") ;
                            fprintf(stderr, "Runtime error detected!\n") ;
                            result.runtime_err_cnt++ ;
                        } else {
                            if (read_and_compare_results(ctop_fd[0], entry->d_name)) {
                                if (gettimeofday(&(config.tv_end), NULL) == -1)
                                    return error_exit("gettimeofday") ;
                                if (calculate_exec_time(&result) < 0)
                                    return error_exit("Calculating execution time") ;
                            } 
                        }
                    } else if (WIFSIGNALED(status)) { // child process was killed by a signal
                        fprintf(stderr, "Runtime error detected! Child was killed by signal %d (%s)\n", WTERMSIG(status), strsignal(WTERMSIG(status))) ;
                        result.runtime_err_cnt++ ;
                        close(ctop_fd[0]) ;
                        continue ; // skip reading
                    }
                    close(ctop_fd[0]) ;
                    
                    break ;    
            }
        }
    }

    closedir(dir) ;

    return EXIT_SUCCESS ;
}

int 
main(int argc, char * argv[])
{   
    if (parse_arg(argc, argv, &config))
        goto err ;

    if (check_directory(config.input_dir))
        goto err ;

    if (check_directory(config.output_dir))
        goto err ;

    if (set_timer(&config)) 
        goto err ;
    
    if (set_sigaction(&config))
        goto err ;
    
    if (run_compilation_child() == EXIT_FAILURE) 
        goto err ;

    sleep(1) ;
    if (run_execution_child()) 
        goto err ;
    
    print_results() ;

err :
    print_results() ;
    exit(EXIT_FAILURE) ;

    return EXIT_SUCCESS ;
}
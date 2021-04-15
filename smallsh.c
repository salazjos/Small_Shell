/*
 Name: Joseph Salazar
 email: salazjos@oregonstate.edu
 class: CS344
 Assignment: Program 3 - smallsh
 file: smallsh.c
 Description: contains main function for running smallsh ("small shell")
 program.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "smallshfunctions.h"

int main(int argc, const char * argv[]) {

    char input_line[ MAX_CHAR ]; //MAX_CHAR is 2048
    char *args[ MAX_ARGS ]; //MAX_ARGS is 512
    int cmd_test_value = 0,  cmd_amount = 0;
    int is_background_cmd = FALSE;
    int child_exit_status = JUNK_VALUE; //-5
    int target_fd_in = 0, target_fd_out = 0;
    int dup_result = 0;
    int process_id = getpid();
    int process_amount = 0;
    foreground_only_mode = FALSE; //declared in smallfunctions.h

    /*struct for containing redirection info.
     declared in smallfunctions.h */
    struct Re_Direction_Info rdi;

    //pid_t for spawn process
    pid_t spawn_id = JUNK_VALUE; //JUNK_VALUE is -5
    //pid_t array to hold background processes
    pid_t non_complete_processes[MAX_PROCESS_AMOUNT]; //MAX_PROCESS_AMOUNT is 100
    //fill array indexes with JUNK_VALUE (-5) to start
    fillWithJunk(non_complete_processes, MAX_PROCESS_AMOUNT);

    //sigaction struct to handle SIGINT signal
    struct sigaction sigint_action = {0};
    sigint_action.sa_handler = SIG_IGN;//main shell should ignore
    sigaction(SIGINT, &sigint_action, NULL);

    //sigaction struct to handgle SIGTSPT signal
    struct sigaction sigtstp_action = {0};
    sigtstp_action.sa_handler = catchSIGTSTP; //main shell will execute catchSIGTSTP function
    sigtstp_action.sa_flags   = SA_RESTART;
    sigaction(SIGTSTP, &sigtstp_action, NULL);

    //fill all args indexes to null
    int i;
    for(i = 0; i < MAX_ARGS; i++){
        args[i] = NULL;
    }

    //continue getting commands until exit command is given
    while(cmd_test_value != 1)
    {
        //memset the arrays for every loop
        memset(input_line, '\0', sizeof(input_line));

        //re-direction flags set to FALSE
        rdi.re_direction[0] = rdi.re_direction[1] = rdi.is_redirection = is_background_cmd = FALSE;
        rdi.out_direction_index = rdi.in_direction_index = 0;

        //free memory from *args[]
        freeArgs(args);

        //check on background processes
        handleBackgroundProcesses(non_complete_processes, MAX_PROCESS_AMOUNT, &process_amount);

        //show the input prompt
        printf(": ");
        fflush(stdout);

        //get the line from the user
        getInputLineFromTerminal(input_line);

        //test for blank line or comment
        if(input_line[0] == '\n' || input_line[0] == '#')
            continue;

        //parse the input line into arguments
        parseArray(input_line, args, &cmd_amount);

        //replace any "$$" with the process id
        replaceDollarSigns(args, cmd_amount, process_id);

        //test for valid built-in commands
        cmd_test_value = testForBuiltInCommands(args);

        //test for input/output re-direction
        testForReDirection(&rdi, args, cmd_amount);

        /*test if last argument is &. if yes, is_background_cmd = TRUE,
         and remove the & from args. else, is_background_cmd = FALSE */
        is_background_cmd = isBackgroundCmd(args, cmd_amount);

        //if foreground mode is on, then no background commands allowed
        if(foreground_only_mode == TRUE)
            is_background_cmd = FALSE;

        //test if exit command
        if(cmd_test_value == 1) //exit command
        {
            //kill all running background proccesses
            exitCommand(non_complete_processes, MAX_PROCESS_AMOUNT);
            //free argument array memory
            freeArgs(args);
            //break from while loop
            break;
        }

        //run built-in commands of "cd", "status"
        if(cmd_test_value >= 2)
        {
            switch(cmd_test_value)
            {
                case 2: //cd command
                    (cmd_amount > 1) ? cdCommandPath(args[1]) : cdCommandHome();
                    break;
                case 3: //status command
                    statusCommand(child_exit_status);
                    break;
                default: break;
            }
        }
        else //no built-in command, blank line, or comment
        {
            spawn_id = fork(); //spawn child process
            switch(spawn_id){
                case -1: //error from fork
                    perror("Forking error!\n");
                    exit(1);
                    break;
                case 0: //fork successful, child will run
                    //test if background process and foreground-only is off
                    if(is_background_cmd == TRUE && foreground_only_mode == FALSE){
                        //test if re-direction out ">"
                        if(rdi.re_direction[1] == TRUE){
                            assignRedirectionOut(&rdi, &dup_result, &target_fd_out, args);
                            args[1] = NULL;
                        }
                        else{ // no ">" redirection provided
                            assignNoOutRedirection(&dup_result, &target_fd_out);
                        }
                        //test if re-direction in "<"
                        if(rdi.re_direction[0] == TRUE){
                            assignRedirectionIn(&rdi, &dup_result, &target_fd_in, args);
                            args[1] = NULL;
                        }
                        else{ //no "<" re-direction
                            assignNoInRedirection(&dup_result, &target_fd_in);
                        }
                    } //end if backgound command is true and foreground mode only is false
                    else{ //not a background command
                        if(rdi.is_redirection == TRUE){
                            //test if re-direction out ">"
                            if(rdi.re_direction[1] == TRUE){
                                assignRedirectionOut(&rdi, &dup_result, &target_fd_out, args);
                            }
                            //test if re-direction in "<"
                            if(rdi.re_direction[0] == TRUE){
                                assignRedirectionIn(&rdi, &dup_result, &target_fd_in, args);
                            }
                            args[1] = NULL;
                        }
                    }

                    //SIGINT
                    if(is_background_cmd == FALSE){
                        sigint_action.sa_handler = SIG_DFL;
                        sigaction(SIGINT, &sigint_action, NULL);
                    }

                    //SIGTSTP
                    sigtstp_action.sa_handler = SIG_IGN;
                    sigaction(SIGTSTP, &sigtstp_action, NULL);

                    //EXECUTE the command
                    execvp(args[0], args);
                    perror("Execution error");  //error occurred
                    child_exit_status = 1;
                    exit(1);
                    break;
                default:
                    //test if foreground command
                    if(is_background_cmd == FALSE){
                        //wait for the process to finish
                        spawn_id = waitpid(spawn_id, &child_exit_status, 0);
                        //print terminating signal info that killed the process
                        if( WIFSIGNALED(child_exit_status))
                            statusCommand(child_exit_status);
                    }
                    else{ //background process
                        //spawn_id is intended as a background process
                        printf("background pid is %d\n", spawn_id); //print the pid
                        fflush(stdout);
                        //assign the spawn_id into the non complete processess array
                        assignInFirstEmptyIndex(non_complete_processes, spawn_id, MAX_PROCESS_AMOUNT);
                        process_amount++; //increase process amount by 1
                    }

            } //end switch statement

        } //end else

    } //while shell loop
    return 0;
}

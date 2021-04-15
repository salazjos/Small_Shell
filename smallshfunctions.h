/*
 name: Joseph Salazar
 email: salazjos@oregonstate.edu
 class: CS344
 Assignment: Program 3 - smallsh
 file: smallshfunctions.h
 Description: header file for smallshfunctions.h
 */

#ifndef SMALLSHELL_SMALLSHFUNCTIONS_H
#define SMALLSHELL_SMALLSHFUNCTIONS_H

#define MAX_ARGS 512
#define MAX_CHAR 2048
#define TRUE 1
#define FALSE 0
#define JUNK_VALUE -5
#define MAX_PROCESS_AMOUNT 100

#include <signal.h>
#include <sys/types.h>

//struct to hold re-direction info
struct Re_Direction_Info{
    /*holds TRUE/FALSE for if any re-direction
     symbol found.
     index 0 for <, index 1 for > */
    int re_direction[2];
    //holds index of ">" in args array
    int out_direction_index;
    //holds index of "<" in args array
    int in_direction_index;
    /*hold TRUE/FALSE if any
     re-direction symbol(< or >) was found */
    int is_redirection;
};

/*global variable that holds TRUE/FALSE, whether
 foreground-only mode is on or off */
volatile sig_atomic_t foreground_only_mode;

//Get input from the terminal
void getInputLineFromTerminal(char *input);

//split the input string into array of arguments
void parseArray(char *input_array, char **arg_array, int *cmd_amt);

//uses the chdir command to change the directory path
void cdCommandPath(char *path);

//no path, uses chdir command to change to "HOME" path
void cdCommandHome();

//exit command
void exitCommand(pid_t *backgroundProcess, int size);

// print the status
void statusCommand(int exit_status);

//test if there are any re-direction symbols (< or >)
void testForReDirection(struct Re_Direction_Info *rdi, char **input_array, int arg_amt);

//handle background processes
void handleBackgroundProcesses(pid_t *backgroundProcess, int size, int *process_amt);

//replace "$$" with process id in string
void replaceDollarSigns(char **input_array, int arg_amount, int p_id);

//free args memory
void freeArgs(char **arg_array);

//fill backgroundProcess array with JUNK_VALUE's
void fillWithJunk(pid_t *backgroundProcess, int size);

//assign child_process into the first available index in backgroundProcess
void assignInFirstEmptyIndex(pid_t *backgroundProcess, pid_t child_process, int size);

//handle output redirection in command
void assignRedirectionOut(const struct Re_Direction_Info *rdi, int *dup_result, int *target_fd_out,
                          char **args);

//handle input redirection in command
void assignRedirectionIn(const struct Re_Direction_Info *rdi, int *dup_result, int *target_fd_in,
                         char **args);

//handle no output redirection in command
void assignNoOutRedirection(int *dup_result, int *target_fd_out);

//handle no input redirection in command
void assignNoInRedirection(int *dup_result, int *target_fd_in);

//function for handling SIGTSPT signal
void catchSIGTSTP(int signo);

//test if the input line is a built in command of cd or status
int testForBuiltInCommands(char **input);

//determine if process is meant to be a run in background
int isBackgroundCmd(char **arg_array, int arg_amount);

#endif //SMALLSHELL_SMALLSHFUNCTIONS_H

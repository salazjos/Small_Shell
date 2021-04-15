/*
 name: Joseph Salazar
 email: salazjos@oregonstate.edu
 class: CS 344
 Assignment: Program 3 - smallsh
 file: smallshfunctions.c
 Descriptions: contains functions implimentations
 used in smallsh.c
 */

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "smallshfunctions.h"

/*
 Function name: getInputLine
 Parameters: pointer to char array (input)
 Function type: void
 Description: gets input via stdin and
 writes to input_array. copies string from
 input_array into input
 */
void getInputLineFromTerminal(char *input)
{
    char input_array[MAX_CHAR]; //MAX_CHAR = 2048
    fgets(input_array, MAX_CHAR, stdin);
    strcpy(input, input_array);
}

/*
 Function name: testForCommands
 Parameters: pointer to an array of strngs(input_array)
 Function type: int
 Description: test strings in the first index of input_array
 for any of the three words in build_in_cmd array. return 1 is
 "exit" found, return 2 if "cd" found, return 3 if "status"
 found, return FALSE (0) if none of those strings were found.
 */
int testForBuiltInCommands(char **input_array)
{
    //built-in commands list
    char *built_in_cmds[3] = {"exit", "cd", "status"};

    if(strstr(input_array[0], built_in_cmds[0]) != NULL) //exit command
        return 1;

    if(strstr(input_array[0], built_in_cmds[1]) != NULL) //cd command
        return 2;

    if(strstr(input_array[0], built_in_cmds[2]) != NULL) //status command
        return 3;

    return FALSE; //not built-in command, return 0.
}

/*
 Function name: parseArray
 Parameters: pointer to string array (input_array),
 pointer to an array of strings (array), pointer to int
 that holds argument amount (arg_amount).
 Function type: void
 Description: breaks up the input_array string into separate strings
 based on " " and "\n". Each string is assigned into array.
 */
void parseArray(char *input_array, char **array, int *arg_amt)
{
    int i = 0;
    char *token = NULL;

    //get the first token
    token = strtok(input_array, " \n");

    //fill the rest of the tokens into the array
    while(token != NULL)
    {
        array[i] = (char *)strdup(token);
        token = strtok(NULL, " \n");
        i++;
    }
    //assign the number of arguments into arg_amt
    *arg_amt = i;
}

/*
 Function name: cdCommandPath
 Parameters: pointer to char array (path)
 Function type: void
 Description: uses chdir to change the file path
 to that of path. prints to stderr if the path is
 not valid
 */
void cdCommandPath(char *path)
{
    if(chdir(path) == -1)
        fprintf(stderr, "%s: no such file or directory\n", path);
}

/*
 Function name: cdCommandHome()
 Parameters: none
 Function type: void
 Description: changes file path to the HOME directory
 */
void cdCommandHome()
{
    //revert to home directory
    chdir(  getenv("HOME")   );
}

/*
 Function name: exitCommand
 Parameters: array of pid_t type (backgroundProcess),
 int of size of the backgroundProcess array.
 Function type: void
 Description: kill any running processes in the
 backgroundProcess array with SIGKILL.
 */
void exitCommand(pid_t *backgroundProcess, int size)
{
    /*kill all the running process, array of processes
     kill( process, SIGTERM ); */
    int kill_value, i;
    for(i = 0; i < size; i++){
        if(backgroundProcess[i] != JUNK_VALUE){
            kill_value = kill(backgroundProcess[i], SIGKILL);
        }
    }
}

/*
 Function name: statusCommand
 Parameters: int for the exit status of a process (exit_status)
 Function type: void
 Description: prints the exit status of a process or the signal
 that terminated a process.
 */
void statusCommand(int exit_status)
{
    if(WIFEXITED(exit_status) != 0){ //terminated normally
        int exit_value = WEXITSTATUS(exit_status);
        printf("exit value %d\n", exit_value);
    }
    else{ //terminated by signal
        int term_signal = WTERMSIG(exit_status);
        printf("terminated by signal %d\n", term_signal);
    }
    fflush(stdout);
}

/*
 Function name: isBackGroundCmd
 Parameters: pointer to array of string (input_array), int
 for the amount of arguments (arg_amount).
 Function type: int
 Description: tests if the string in the last index of
 input_array has "&". if yes, command is intended as a
 background command and return TRUE (1). if no, not a
 background command and return FALSE (0).
 */
int isBackgroundCmd(char **input_array, int arg_amount)
{
    if( strcmp( input_array[ arg_amount - 1], "&") == 0 ){
        input_array[ arg_amount - 1 ] = NULL;
        return TRUE;
    }
    return FALSE;
}

/*
 Function name: testForReDirection
 Parameters: pointer to Re_Direction_Info struct (rdi), pointer to array of
 strings (input_array), int for the argument size (arg_amt)
 Function type: void
 Description: searches each index in the input_array, looking for "<" or ">".
 Assigns variables in rdi as appropriate.
 */
void testForReDirection(struct Re_Direction_Info *rdi, char **input_array, int arg_amt)
{
    int i;
    for(i = 0; i < arg_amt; i++)
    {
        //Look for <
        if( strcmp(input_array[i],  "<") == 0){
            rdi->re_direction[0] = TRUE;
            rdi->in_direction_index = i;
            rdi->is_redirection  = TRUE;
        }
        //Look for >
        if( strcmp(input_array[i],  ">") == 0){
            rdi->re_direction[1]  = TRUE;
            rdi->out_direction_index = i;
            rdi->is_redirection  = TRUE;
        }
    }
}

/*
 Function name: handleBackgroundProcesses
 Parameters: array of pid_t (backgroundProcess, int for size of
 backgroundProcess array (size), pointer to int of amount of processes in
 the backgroundProcess array.
 Function type: void
 Description: Loops through backgroundProcess array checking each process if they
 have finished. If finished, assign that index in the array to JUNK_VALUE so that it
 can be reassigned.
 */
void handleBackgroundProcesses(pid_t *backgroundProcess, int size, int *process_amount)
{
    int child_exit_value = JUNK_VALUE; //-5
    pid_t tempID = JUNK_VALUE;
    int i;
    for(i = 0; i < size; i++){
        if(backgroundProcess[i] != JUNK_VALUE){
            //check if  background process has finished
            tempID = waitpid(backgroundProcess[i], &child_exit_value, WNOHANG);
            if( tempID  > 0 ){
                //background process has finished
                printf("background pid %d is done: ", tempID);
                statusCommand(child_exit_value);
                backgroundProcess[i] = JUNK_VALUE; //reassign index to junk value
                (*process_amount)--; //decrement process amount
            }
        }
    }
}

/*
 Function name: replaceDollarSigns
 Parameters: pointer to array of strings (input_array),
 int for the amount of arguments in input_array (arg_amount),
 int for the process id (p_id).
 Function type: void
 Description: Loop through each string in input_array looking for "$$".
 if found, replace "$$" with the value of p_id. Builds a new string
 and places that string in the index that $$ was found in.
 */
void replaceDollarSigns(char **input_array, int arg_amount, int p_id)
{
    char temp_string[MAX_CHAR];
    char pIDtoStr[20];
    int dollar_sign_amount = 0;
    int sub_string_length  = 0;
    int temp_string_index  = 0;

    memset(pIDtoStr, '\0', sizeof(pIDtoStr));
    sprintf(pIDtoStr, "%d", p_id); //convert pid to string
    int pidstr_length = (int)strlen(pIDtoStr);

    int i, j, k;
    for(i = 0; i < arg_amount; i++){

        memset(temp_string, '\0', sizeof(temp_string));
        //look for $$ in each substring
        //$$ found
        if(strstr( input_array[i], "$$") != NULL ){
            //get the length of the string
            sub_string_length = strlen( input_array[i] );
            for(j = 0; j < sub_string_length; j++){
                //copy all characters from sub string that are not '$'
                if(input_array[i][j] != '$'){
                    temp_string[temp_string_index] = input_array[i][j];
                    temp_string_index++;
                }
                else{ //the character is $
                    dollar_sign_amount++;
                    if(dollar_sign_amount % 2 == 0){
                        //copy the numbers from pIDtoStr array into tempstring
                        for(k = 0; k < pidstr_length; k++){
                            if(pIDtoStr[k] != '\0'){
                                temp_string[temp_string_index] = pIDtoStr[k];
                                temp_string_index++;
                            }
                        }
                        dollar_sign_amount = 0;
                    }
                }
            }
            input_array[i] = strdup(temp_string); //uses malloc
            temp_string_index = 0;
        }
    }
}

/*
 Function name: freeArgs
 Parameters: pointer to array of strings (args_array)
 Function type: void
 Description: Every string in args_array is malloced.
 Loops through the args_array and free's any index that is
 not null.
 */
void freeArgs(char **arg_array)
{
    int i;
    for(i = 0; i < MAX_ARGS; i++){
        if(arg_array[i] != NULL){
            free(arg_array[i]);
            arg_array[i] = NULL;
        }
    }
}

/*
 Function name: fillWithJunk
 Parameters: array of pid_t (backgroundProcess), int for
 the size of backgroundProcess.
 Function type: void
 Description: Loops through every index in backgroundProcess
 array and assign JUNK_VALUE (-5)
 */
void fillWithJunk(pid_t *backgroundProcess, int size)
{
    int i;
    for(i = 0; i < size; i++)
        backgroundProcess[i] = JUNK_VALUE;
}

/*
 Function name: assignInfirstEmptyIndex
 Parameters: array of pid_t (backgroundProcess), pid_t that holds the
 child process (child_process), int for the size of the backgroundProcess array (size).
 Function type: void
 Description: Loops through the backgroundProcess array and assign child_process into
 the first available index.
 */
void assignInFirstEmptyIndex(pid_t *backgroundProcess, pid_t child_process, int size)
{
    int i;
    for(i = 0; i < size; i++){
        if(backgroundProcess[i] == JUNK_VALUE){
            backgroundProcess[i] = child_process;
            break;
        }
    }
}

/*
 Function name: catchSIGTSTP
 Parameters: int for the signal that was caught (signo)
 Function type: void
 Description: Fuction is intended as a function pointer that will be
 assigned as the handler for the sigtstp_action struct in main. Function
 toggles foreground_only_mode on and off when Ctrl-Z is sent.
 */
void catchSIGTSTP(int signo)
{
    char *colon_prompt = ": ";
    if(foreground_only_mode == FALSE){ //turn foreground_only_mode on
        foreground_only_mode = TRUE;
        char *foreground_mode_enter_message = "Entering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, foreground_mode_enter_message, 49);
        fflush(stdout);
        write(STDOUT_FILENO, colon_prompt, 2);
    }
    else{
        foreground_only_mode = FALSE; //turn foreground_only_mode off
        char *forground_mode_exit_message = "Exiting foreground-only mode\n";
        write(STDOUT_FILENO, forground_mode_exit_message, 29);
        write(STDOUT_FILENO, colon_prompt, 2);
    }
    fflush(stdout);
}

/*
Function name: assignRedirectionOut
Parameters: pointer to Re_Direction_Info struct (rdi), int pointer to duplicate file
descriptor result (dup_result), int pointer to the out target file descriptor (target_fd_out),
pointer to array of char arrays containing the arguments from the input as the desired terminal
command (args).
Function type: void
Description: When the outward direction symbol ">" is in the terminal command, this function assigns
the outout of the command to the specified file in the command.
*/
void assignRedirectionOut(const struct Re_Direction_Info *rdi, int *dup_result, int *target_fd_out,
                          char **args)
{
    //attempt to open file to write
    *target_fd_out = open( args[ rdi->out_direction_index + 1 ],
                           O_WRONLY | O_CREAT | O_TRUNC, 0644);
    //test if file error
    if(*target_fd_out == -1){
        fprintf(stderr,"Cannot open %s for output\n", args[ rdi->out_direction_index + 1]);
        exit(1);
    }
    //assign dup2() and test for dup2() error
    int temp_file_descriptor = *target_fd_out;
    *dup_result = dup2(temp_file_descriptor, 1);
    if((*dup_result) == -1){
        fprintf(stderr, "dup2() error\n");
        exit(2);
    }

}

/*
Function name: assignRedirectionIn
Parameters: pointer to Re_Direction_Info struct (rdi), int pointer to duplicate file
descriptor result (dup_result), int pointer to the input target file descriptor (target_fd_in),
pointer to array of char arrays containing the arguments from the input as the desired terminal
command (args).
Function type: void
Description: When the inward direction symbol "<" is in the terminal command, this function assigns
the output of one file as the input for the file specified in the command.
*/
void assignRedirectionIn(const struct Re_Direction_Info *rdi, int *dup_result, int *target_fd_in,
                         char **args)
{
    //attempt to open file for reading
    *target_fd_in = open(args[rdi->in_direction_index + 1], O_RDONLY);
    //test if file error.
    if(*target_fd_in == -1){
        fprintf(stderr, "Cannot open %s for input\n", args[rdi->in_direction_index + 1]);
        exit(1);
    }
    //assign dup2() and test for dup2() error
    int temp_file_descriptor = *target_fd_in;
    *dup_result = dup2(temp_file_descriptor, 0);
    if(*dup_result == -1){
        fprintf(stderr, "dup2() error\n");
        exit(2);
    }
}

/*
Function name: assignNoOutRedirection
Parameters: int pointer to duplicate file descriptor result (dup_result),
int pointer to the output target file descriptor (target_fd_out),
Function type: void
Description: When no outward redirection symbol (">") is in the terminal command, this
function assigns the output of the command to be directed to "/dev/null".
*/
void assignNoOutRedirection(int *dup_result, int *target_fd_out)
{
    *target_fd_out = open("/dev/null", O_WRONLY);
    //test if file error
    if(*target_fd_out == -1){
        fprintf(stderr,"Cannot open /dev/null for output\n");
        exit(1);
    }
    //assign dup2() and test for dup2() error
    int temp_file_descriptor = *target_fd_out;
    *dup_result = dup2(temp_file_descriptor, 1);
    if(*dup_result == -1){
        fprintf(stderr, "dup2() error\n");
        exit(2);
    }
}

/*
Function name: assignNoOutRedirection
Parameters: int pointer to duplicate file descriptor result (dup_result),
int pointer to the input target file descripor (target_fd_in),
Function type: void
Description: When no input redirection symbol ("<") is in the terminal command, this
function assigns the input to be directed to "/dev/null".
*/
void assignNoInRedirection(int *dup_result, int *target_fd_in)
{
    *target_fd_in = open("/dev/null", O_RDONLY);
    if(*target_fd_in == -1){
        fprintf(stderr, "Cannot open /dev/null for input\n");
        exit(1);
    }
    //assign dup2() and test for dup2() error
    int temp_file_descriptor = *target_fd_in;
    *dup_result = dup2(temp_file_descriptor, 0);
    if(*dup_result == -1){
        fprintf(stderr, "dup2() error\n");
        exit(2);
    }
}


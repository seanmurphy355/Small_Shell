//Name:Sean Murphy
//Description: This assignment aims to create a shell in C. Similair to bash. no other lang are allowed but C and we are allowed to use C99.
//The shell should run command line instructions and return the results similiar to other shells we have used. but without many of their fancier features.
// This will work like the bash shell you are used to using, prompting for a command line and running commands, but it will not have many of the special features of the bash shell.
// The shell should allow for the rediriection of standard input and standard output it will support both foreground and background processes. It should support the following commands
//Commands:exit,cd and statues it will also support comments which are lines beginning with the # character.

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pwd.h>

//This will be max string size that the program can handle it should be able to hand 2048 max length and a max args of 513
#define _GNU_SOURCE
#define Max_String 2048
#define Max_args 512
pid_t childPid[Max_args];
char inputArray[2048];
int Mode = 0;        // This will indicate what mode we are on 0: No active mode, 1 will indicate active entry to foreground mode and 2 will exit our foreground mode.
int foreMode = 1;    // FALSE = 0, True = 1 indicates the mode we are currently in.
char Expansion[512]; //global for pid expansion.

//Function definitions
char *GrabCommandInput();
int RunProgram();
void getStatus(int status);
void modeSigation(int signal);
void runCommands(char input_file[], char out_file[], char *myArgs[], int *status, struct sigaction SIG_AC, int *bg);

//Main function will make any calls we need from here!
int main()
{
    //run the program!
    RunProgram();

    return 0;
}

//takes in pid and newVal or pid expansion.
void PidExpansion(char *newValue, int pid)
{
    //set up needed variables for conversion.
    char stringPid[512];
    char buffer[2048];
    char *CopyVal = newValue;

    //Set stringpid to the pid that we currently have.
    sprintf(stringPid, "%d", pid);

    //find then $$ command within our substring. ststr -> finds occurances of substrings.
    while ((CopyVal = strstr(CopyVal, "$$")))
    {
        //We will then copy the sub string that occurs before sub string $$ and then append the pid to the string value that was input in the commandline.
        strncpy(buffer, newValue, CopyVal - newValue);
        buffer[CopyVal - newValue] = '\0';
        strcat(buffer, stringPid);
        strcat(buffer, CopyVal + strlen("$$"));

        strcpy(newValue, buffer); // set our newval to copy of buffer. dest <- src
        CopyVal++;
    }
}

//Function that will carry out the running of the entire program.
int RunProgram()
{
    char *Out_File = NULL; //inti of out file
    char *in_File = NULL;  //inti of in file
    pid_t spawnPid = -5;
    int exitValue = -5; //intilalization of child pid
    int status = 0;
    int bg = 0;
    pid_t backgroundpid[512]; //Will store pid of background process.
    char *Storage = NULL;     //Storage array used to store the user input
    char *Container = NULL;   // Container (just incase needed)
    int runProgram = 1;       // how to loop program
    char *myArgs[512] = {0};  //max args array
    int val = 0;
    char getInput[2048];
    int pos;

    struct sigaction SIGINT_action = {0};  //Initialize SIGINT_action struct to be empty
    struct sigaction SIGTSTP_action = {0}; //Initialize SIGSTP_action struct to be empty
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    SIGINT_action.sa_flags = 0;                // No flags set
    sigaction(SIGINT, &SIGINT_action, NULL);   // Install our signal handler
    SIGTSTP_action.sa_handler = modeSigation;  // Register modeSigation the signal handler
    sigfillset(&SIGTSTP_action.sa_mask);       // Block all catchable signals
    SIGTSTP_action.sa_flags = 0;               // No flags set
    sigaction(SIGTSTP, &SIGTSTP_action, NULL); // Install our signal handler

    memset(backgroundpid, 0, 512);

    do
    {
        //declare an area to store /convert pid when entering $$
        int pid = getpid(); //get pid

        //had an odd issue with not being able to generate a completely null arg array this fixed it.
        int j;
        for (j = 0; j < 512; j++)
        {
            myArgs[j] = NULL;
        }

        int max = 0;
        //Grab the users input
        char *getInput = GrabCommandInput();

        //Ensure that we handle comments and blanks
        if (getInput[0] == '#' || getInput[0] == '\0')
        {
            continue;
        }

        //strstok will break our file input up based on the delimiter
        Storage = strtok(getInput, " \n");
        int Idx = -1;
        //printf("%s", Storage); error checking!
        while (Storage != NULL)
        {
            //If we have a sign that means we need our output to be redircted. I.E if an output file exits we want to store it.
            if (strcmp(Storage, ">") == 0)
            {
                //Set storage to be the next token after > and then store file name as Out_file.
                Storage = strtok(NULL, " \n");
                Out_File = strdup(Storage);
                Storage = strtok(NULL, " \n");
                continue;
            }

            //If we have a sign that means we need our output to be redircted. I.E if an input file exits we want to store it.
            else if (strcmp(Storage, "<") == 0)
            {
                //Set storage to be the next token after < and then store file name as in_file.
                Storage = strtok(NULL, " \n");
                in_File = strdup(Storage);
                Storage = strtok(NULL, " \n");
                continue;
            }
            else
            {
                //edit if storage is  == to $$ then expand pid.
                if (strstr(Storage, "$$") != NULL)
                {
                    char NewValue[2048];
                    strcpy(NewValue, Storage);
                    PidExpansion(NewValue, pid);
                    Storage = NewValue;
                }

                myArgs[max] = strdup(Storage);
                max++;
                Storage = strtok(NULL, " \n");
            }
            //now we need to check for if the input has $$ and if so we must expand into a process ID of the shell iteself.
        }

        if (myArgs[0] != NULL && myArgs[0][0] != '#')
        {

            val = max;
            // if the last character is & then set bg to = 1
            if (strcmp(myArgs[max - 1], "&") == 0)
            {
                myArgs[max - 1] = NULL;
                if (bg == 0) //make sure bg is already 0 before setting to 1.
                {
                    bg = 1;
                }
            }

            // allows to the exit command to create an exit 0 escape point.
            if (strcmp(myArgs[0], "exit") == 0)
            {
                exit(0);
            }

            if (strcmp(myArgs[0], "cd") == 0)
            {
                // If the destination is not specificed move to home directory!
                char *dPath;
                if (myArgs[1] == NULL)
                {
                    //go to home directory.
                    dPath = getenv("HOME");
                    chdir(dPath);
                }
                //set the dir path to the first arg after cd.
                else
                {
                    dPath = (myArgs[1]);
                    chdir(dPath);
                }
                //Tell the user we couldnt find the directory they have entered.
                if (chdir(dPath) != 0)
                {
                }
            }

            //This if statement essentially checks our status if teh spawnpid remains -5 just return exit 0 if not eval what the exit or signal was that caused exit.
            if (strcmp(myArgs[0], "status") == 0)
            {
                //call the getstatus function (close to the one provided in the lectures)
                getStatus(status);
            }

            //This will handle anything other than main builtin commands and intialization of files.
            else
            {

                runCommands(in_File, Out_File, myArgs, &status, SIGTSTP_action, &bg);
            }
        }
        //free memory from myArgs for next command that will be input.
        int i;
        for (i = 0; i < val; i++)
        {
            if (myArgs[i] != NULL)
            {
                free(myArgs[i]);
            }
        }
        bg = 0;
        //set first arg back to null (had a odd issue with this)
        //Free the input file,getinput and the out file as well as set them back to null.
        in_File = NULL;
        free(Out_File);
        Out_File = NULL;
        free(Out_File);
        myArgs[512] = NULL;

        //While runProgram = 1 keep asking for userinput and check processes.
    } while (runProgram);
    return 0;
}

//Grab input will be the function used to read in user input it will return the input back to main!
char *GrabCommandInput()
{
    //run will run the process continously mainly for testing purposes added a break to end when done.
    int run = 1;
    int value = -5;
    char *getnum;
    int forground;
    int check = 1;

    while (run)
    {
        //print the prompt to the user and flush output buffer then grab user input
        printf(": ");
        fflush(stdout); //flushes the output buffer

        //Stops from generating a seg fault while running script.
        if (fgets(inputArray, 2048, stdin) == NULL)
        {
            exit(1);
        }

        //Get the size of the string input by the user.
        ssize_t len = strlen(inputArray);
        int value = len;
        value -= 1;

        if (value == -1)
        {
            clearerr(stdin);
        }
        //chcek to see if the string at index 0 has a # or is null.
        if (strcmp(inputArray, "\n") == 0 || inputArray[0] == '#')
        {
            //Ask again
            run = 1;
        }

        else
        {
            //Valid input was entered break both the else and while loop.
            run = 2;
            break;
        }
    }
    //returns our array that holds our user input.
    return inputArray;
}

void runCommands(char input_file[], char out_file[], char *myArgs[], int *status, struct sigaction SIG_AC, int *bg)
{

    pid_t spawnPid = -5;
    int infile;
    int outfile;
    int Value;

    //Begin our Child process and create a switch statement like the one from the lecture.
    spawnPid = fork();
    switch (spawnPid)
    {
    // incase of breach have an exit case.
    //ref: https://oregonstate.instructure.com/courses/1806251/pages/exploration-processes-and-i-slash-o?module_item_id=19760593
    //ref: https://oregonstate.instructure.com/courses/1806251/pages/exploration-process-api-monitoring-child-processes?module_item_id=19760584
    case -1:
    {
        perror("fork() Failed.\n");
        exit(1);
        break;
    }
    case 0:
    {
        // take care of ctrl c
        if (foreMode == 1)
        {
            //ref https://oregonstate.instructure.com/courses/1806251/pages/exploration-signal-handling-api?module_item_id=19760592
            SIG_AC.sa_handler = SIG_DFL;
            sigaction(SIGINT, &SIG_AC, NULL);
        }
        else
        {
            sigaction(SIGINT, &SIG_AC, NULL);
        }
        // If our input file is not equal to null assign it.
        if (input_file != NULL)
        {
            //
            infile = open(input_file, O_RDONLY);
            if (infile == -1)
            {
                perror("Cannot open input file\n");
                fflush(stdout);
                *status = 1; //set status to 1
                _Exit(1);    //aborts child process.
            }
            // set file descriptor
            Value = dup2(infile, 0);
            if (Value == -1)
            {
                perror("Cannot assign input file\n");
                fflush(stdout);
                *status = 1; //set status to 1
                _Exit(1);    //aborts child process.
            }
            //trigger file closure.
            fcntl(infile, F_SETFD, FD_CLOEXEC);
        }
        // if the outfile is null.
        if (out_file != NULL)
        {
            //open file up and use conditionals.
            outfile = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
            if (outfile == -1)
            {
                printf("Cannot open open output file\n");
                fflush(stdout);
                *status = 1; //set status to 1
                _Exit(1);    //aborts child process.
            }
            // set file descriptor
            Value = dup2(outfile, 1);
            if (Value == -1)
            {
                perror("dup2\n");
                fflush(stdout);
                *status = 1; //set status to 1
                _Exit(1);    //aborts child process.
            }
            //Trigger file closures  https://oregonstate.instructure.com/courses/1806251/pages/exploration-processes-and-i-slash-o?module_item_id=19760593
            fcntl(outfile, F_SETFD, FD_CLOEXEC);
        }
        //run execution. using exec family.
        if (execvp(myArgs[0], myArgs))
        {
            printf("%s: no such file or directory\n", myArgs[0]);
            *status = 1; //set status to 1
            fflush(stdout);
            _Exit(1); //aborts child process.
        }
        break;
    }
    default:
    {
        //If the process is in bg and foremode are the same then execture process
        if (*bg == 1 && foreMode == 1)
        {
            waitpid(spawnPid, status, WNOHANG);
            printf("background pid: %d\n", spawnPid);
            fflush(stdout);
        }
        //normal waitpid execution. (wait for terminated children)
        else
        {
            waitpid(spawnPid, status, 0);
        }
        //now we will check for any termination of the background processes.
        while ((spawnPid = waitpid(-1, status, WNOHANG)) > 0)
        {
            printf("Background Pid %d was terminated ; ", spawnPid);
            fflush(stdout);
            getStatus(*status);
        }
    }
    }
}

// this function will take in an int status which will be used as the exit type.
//ref: https://oregonstate.instructure.com/courses/1806251/pages/exploration-process-api-monitoring-child-processes?module_item_id=19760584
void getStatus(int status)
{
    //determine if we ended normally if so print the exit statsus.
    if (WIFEXITED(status))
    {
        status = WEXITSTATUS(status);
        printf("exit value %d\n", status);
    }
    //else print the signal value.
    else
    {
        printf("%d", WTERMSIG(status));
        status = WTERMSIG(status);
        printf(" terminated by signal %d \n", status);
    }
}

//This function will handle changing of modes to and from foreground takes in a int signal.
//Ref:https://oregonstate.instructure.com/courses/1806251/pages/exploration-signal-handling-api?module_item_id=19760592
void modeSigation(int signo)
{
    // if we move into fg mode allow user to know after print flush stdout as well as change foremode and mode
    if (foreMode == 1)
    {
        foreMode = 0;
        write(1, "\nEntering foreground-only mode (& is now ignored)\n", 54);
        fflush(stdout);
        Mode = 1;
    }
    // if we move out of fg mode allow user to know after print flush stdout as well as change foremode and mode
    else
    {
        foreMode = 1;
        write(1, "\nExiting foreground-only mode\n", 60);
        fflush(stdout);
        Mode = 2;
    }
}

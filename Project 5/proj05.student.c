/*******************************************************************************
 Chris Nosowsky
 Computer Project #5
 ******************************************************************************/
#include <unistd.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#define MAX 128
 
extern char **environ;

////////////////////////////PROJECT NOTE////////////////////////////
// More tokens then expected for cd and set -> Warning given, processes the first 3(set) or 2(cd) and ignores rest
// Unsetenv if variable not found, warning is given.
// External command will warn of errors in command.
////////////////////////////////////////////////////////////////////

void *external_comm(void* arg) {
     /*--------------------------------------------------------------------------   
     * Name:     external_comm   
     * Purpose:  Calls system() with inputted command.
     * Receive:  argument for thread function to process, command.
     * Return:   pthread_exit(NULL)
     * -----------------------------------------------------------------------*/ 
    if (system((char *) arg) != 0) {                // if external command errors or not found, enter in and display error message.
        printf("Error: command returned with errors. Continuing on...\n");
        pthread_exit( NULL );
    }
    pthread_exit( NULL );
}

int main(int argc, char* argv[]) {
    /*--------------------------------------------------------------------------   
     * Name:     main   
     * Purpose:  Loops through each file left to right and processes each line in
     *           input files as directive for first token, if valid.
     * Receive:  Char array command line arguments and # of arguments given
     * Return:   0 or 1(error)
     * -----------------------------------------------------------------------*/      
    FILE *fp;                                       // file pointer
    const char *path, *varName;                     // file path to open
    char userpath[128];
    char line[MAX];                                 // each line in file
    char tempLine[MAX];
    int lineNum = 1;                                // keeps track of the lines in file
    int numTokens = 1;                              // for tracking # of tokens to display error
    char cwd[PATH_MAX];                             
    const char del[] = " \t\r\n\f";                 // all delimeters for strtok
    int enterIt = 0;                                // tester to enter CD mechanics
    int skip = 0;
    char *token, *envtoken, *user, *p;              // strtok variables
    int breakit = 0;                                // if exit called, this sets to 1
    int seq = 1;                                    // verbose or sequence setter. Set to print <#> by default.                   
    int enterSet = 0;
    int skipSet = 0;
    int setSet = 0;
    int setCD = 0;
    pthread_t comthread;
    
    for (int i=1; i<argc; i++) {                    // loops through every arg in file
        breakit = 0;                                // reset variables for each loop
        lineNum = 1;
        numTokens = 1;
        if (strcmp(argv[i], "-s") == 0) {
            seq = 0;
        }
        else if(strcmp(argv[i], "-v") == 0) {
            seq = 1;
        }
        else if(argv[i][0] == '-') {                // another flag other then s and v. error.
            printf("Error on: '%s' -- invalid option. Please use '-v' or '-s' for options.\n", argv[i]);
            return 1;
        }
        else {                                      // else it is considered a file!!
            path = argv[i];
            fp = fopen(path, "r");
            if (!fp) {                              // didn't find file
                if(errno == EACCES) {
                    printf("Warning: insufficient permissions to read file/file path: '%s'. Ignoring and moving on...\n", path);
                    continue;
                }
                else {
                    printf("Warning: File '%s' not found. Please give a existing file or move to correct directory. Ignoring and moving on...\n", argv[i]);
                    continue;                 
                }
            }
            while(fgets(line, MAX, fp) != NULL && breakit != 1) {   // get each line until we get EOF. DO NOT ENTER if exit was called (breakit set))
                numTokens = 1;
                skip = 0;
                if (line[strlen(line) - 1] != '\n') {   // Line exceeds buffer size according to specs
                    printf("ERROR: BUFFER EXCEEDS SIZE OF 128.\n");
                    return 1;
                }
                
                if (seq == 1) {                     // if -v IS set (default, it is)
                    if(line[strlen(line)-1] == '\n')
                        printf("<%d>%s", lineNum, line);
                    else
                        printf("<%d>%s\n", lineNum, line);
                    lineNum += 1;
                }
                strcpy(tempLine, line);
                token = strtok(line, del);          // strips each token by delimeter array (whitespace)
                while(token != NULL) {
                    if (enterSet > 0 && numTokens < 4) { // if set called and another token found, set var. If 2 tokens found, then next loop will update or add new env variable to 2nd token value.
                        if (enterSet == 1) {
                            varName = token;
                        }
                        else if (enterSet == 2) {
                            setenv(varName, token ,1);
                        }
                        enterSet += 1;
                    }
                    else if(enterSet > 0 &&  numTokens >= 4) {          // got more then 3 tokens, warn user. Was only supposed to get set VAR or set VAR value, nothing more.
                        printf("Warning: Expected 3 tokens or less for set variable, got more. Processed first 3 tokens, skipping rest. Moving onto next line...\n");
                        break;
                    }
                    if (enterIt == 1 && numTokens < 3) { //cd was called, check next tokens
                        if (token[0] == '~') {                          // cd ~USER or cd ~
                            if (strlen(token) == 1) {                   //special case for cd ~
                                if (chdir(getenv("HOME")) != 0) {       //sets to home if just ~
                                    perror("Error: ");
                                }
                                setenv("PWD", getenv("HOME"), 1);       //updates the pwd to be the HOME env variable value
                            }
                            else {
                                user = token;
                                p = user;                               //user equals the token, I set p equal to user (ex: ~USER)
                                p++;                                    //p++ strips the ~ from USER so it's just USER
                                strcpy(userpath, "/user/");             //I copy the /user/ to userpath
                                strcat(userpath, p);                    //concatenate the /user/ to USER so it's formulated like: /user/cse325
                                if (chdir(userpath) != 0) {             //chdir unless error.
                                    perror("Error: ");
                                }
                                setenv("PWD", userpath, 1);             // update the PWD to the userpath
                            }
                            enterIt = 0;
                            skip = 1;
                        }
                        else {
                            if (chdir(token) != 0) {                        // changes directory to provided token unless it cannot find that directory. then display error.
                                perror("Error: ");
                            }
                            if(getcwd(cwd, sizeof(cwd)) != NULL) {          // gets current directory that file resides in
                                setenv("PWD", cwd, 1);
                            } else {                                        // incase for some reason, there's a problem getting the current directory
                                printf("Error: Problem getting current directory!\n");
                                return 1;
                            }
                            enterIt = 0;
                            skip = 1;
                        }
                    }
                    else if (setCD == 1 && numTokens >= 3) {                // CD was called but more then 2 tokens given, display a warning to user.
                        printf("Error: cd directive expects 2 tokens or less, but got more. Ignoring rest of tokens and moving on...\n");
                        break;
                    }
                    if (strcmp(token, "exit") == 0 && numTokens < 2) { // numTokens restricts any tokens other then the first one on each line from entering
                        breakit = 1;
                        printf("Exit Directive. Terminating File...\n");
                        break;
                    }
                    else if(strcmp(token, "date") == 0 && numTokens < 2) {
                        time_t current;
                        time(&current);
                        printf("%s", ctime(&current));                  // getting current system time
                    }
                    else if(strcmp(token, "env") == 0 && numTokens < 2) {
                        int t = 0;
                        while (environ[t] != NULL) {
                            printf("%s\n", environ[t]);                 // prints every environment variable
                            t++;
                        }
                    }
                    else if(strcmp(token, "path") == 0 && numTokens < 2) {
                        envtoken = strtok(getenv("PATH"), ":");         // gets environment variable "PATH". Breaks up each path by colon into tokens
                        while(envtoken != NULL) {
                            printf("%s\n",envtoken);
                            envtoken = strtok(NULL, ":");
                        }
                    }
                    else if(strcmp(token, "cwd") == 0 && numTokens < 2) {
                        if(getcwd(cwd, sizeof(cwd)) != NULL) {          // gets current directory that file resides in
                            printf("%s\n", cwd);
                        } else {                                        // incase for some reason, there's a problem getting the current directory
                            printf("Error: Problem getting current directory!\n");
                            return 1;
                        }
                    }
                    else if(strcmp(token, "cd") == 0 && numTokens < 2) { //might not need numTokens
                        setCD = 1;
                        if (skip == 0)
                            enterIt = 1;
                        else
                            skip = 0;
                    }
                    else if(strcmp(token, "set") == 0 && numTokens < 2) { //might not need numTokens
                        setSet = 1;
                        if (skipSet ==0)
                            enterSet = 1;
                        else
                            skipSet = 0;
                    }
                    else if(numTokens == 1 && strcmp(token, "proj05") == 0) {
                        printf("FATAL ERROR: proj05 found as first token. Infinite loop would happen. Terminating...\n");
                        return 1;
                    }
                    else if(numTokens == 1) {                           // if first token and no built-in found, then process as an external command
                        pthread_create(&comthread, NULL, external_comm, (void *) tempLine); //processes whole line. References pthread and calls external_comm function.
                        pthread_join(comthread, NULL);                      // joins the thread and breaks once finished.
                        break;
                    }
                    else if (setSet == 0 && setCD == 0) {                   // if set or cd are not set, go in here and display warning that more then 1 token was provided.
                        printf("Warning: Expected 1 token on line for built-in directive, got more. Ignoring remaining tokens and moving on...\n");
                        break;
                    }
                    token = strtok(NULL, del);                              // advances onto next token
                    numTokens += 1;
                }
                if (enterIt == 1) {                                         // only cd no DIR, resets to home directory and updates PWD.
                    if (chdir(getenv("HOME")) != 0) {
                        perror("Error: ");
                    }
                    setenv("PWD", getenv("HOME"), 1);
                    enterIt = 0;
                }
                if (enterSet == 1) {                                        // set called but only 1 token was given so it doesn't set anything.
                    printf("Warning: set token was called, but no variable provided. Continuing on...\n");
                    enterSet = 0;
                }
                else if(enterSet == 2) {                                    // if only set VAR, unset the VAR if it exists, otherwise if no VAR found in environment vars, display warning.
                    if (getenv(varName) != NULL)
                        unsetenv(varName);
                    else
                        printf("Warning: could not find variable name to remove. Continuing on...\n");
                    enterSet = 0;
                }
                setSet = 0;
                setCD = 0;
            }
            fclose(fp);
        }
    }
    return 0;
}
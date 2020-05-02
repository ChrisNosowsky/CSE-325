/*******************************************************************************
 Chris Nosowsky
 Computer Project #4
 ******************************************************************************/
#include <unistd.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#define MAX 128

extern char **environ;

int main(int argc, char* argv[]) {
    /*--------------------------------------------------------------------------   
     * Name:     main   
     * Purpose:  Loops through each file left to right and processes each line in
     *           input files as directive for first token, if valid.
     * Receive:  Char array command line arguments and # of arguments given
     * Return:   0 or 1(error)
     * -----------------------------------------------------------------------*/   
    FILE *fp;                                       // file pointer
    const char *path;                               // file path to open
    char line[MAX];                                 // each line in file
    int lineNum = 1;                                // keeps track of the lines in file
    int numTokens = 1;                              // for tracking # of tokens to display error
    char cwd[PATH_MAX];                             
    const char del[] = " \t\r\n\f";                 // all delimeters for strtok
    char *token, *prevtoken, *envtoken;                         // strtok variables
    int breakit = 0;                                // if exit called, this sets to 1
    int seq = 1;                                    // verbose or sequence setter. Set to print <#> by default.
    int moreThanOneWarning = 0;                     
    int notValidDirectiveWarning = 0;
    
    for (int i=1; i<argc; i++) {                    // loops through every arg in file
        breakit = 0;                                // reset variables for each loop
        lineNum = 1;
        numTokens = 1;
        moreThanOneWarning = 0;
        notValidDirectiveWarning = 0;
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
                    printf("Warning: File '%s' not found. Please give a existing file. Ignoring and moving on...\n", argv[i]);
                    continue;                 
                }
            }
            while(fgets(line, MAX, fp) != NULL && breakit != 1) {   // get each line until we get EOF. DO NOT ENTER if exit was called (breakit set))
                moreThanOneWarning = 0;
                notValidDirectiveWarning = 0;
                numTokens = 1;
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
                token = strtok(line, del);          // strips each token by delimeter array (whitespace)
                while(token != NULL) {
                    if (numTokens > 1 && moreThanOneWarning == 0) {
                       printf("Note: For this project, only first token processed as directive. Ignoring rest of line above after first token: '%s'\n", prevtoken); 
                       moreThanOneWarning = 1;
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
                    else {
                        if(notValidDirectiveWarning == 0 && numTokens < 2) { // will only enter once per line. If first token is invalid directive, display warning
                            printf("Warning on token: '%s' -- Not a valid directive (in this project). Ignoring and moving on to next line...\n", token);
                            notValidDirectiveWarning = 1;
                        }   
                    }
                    prevtoken = token;
                    token = strtok(NULL, del);                              // advances onto next token
                    numTokens += 1;
                }
            }
            fclose(fp);
        }
    }
    return 0;
}
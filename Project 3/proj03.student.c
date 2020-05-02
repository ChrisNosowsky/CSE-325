/*******************************************************************************
 Chris Nosowsky
 Computer Project #3
 ******************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


// How am I handeling unusual errors? I am taking every unsual case, calling printf
// and returning 1 to close the function prompting the user to try again.
// I detect the error, tell the user what went wrong/what to fix, then prompt them to try again.


int main(int argc, char* argv[]) {
    /*--------------------------------------------------------------------------   
     * Name:  main   
     * Purpose:  Copy contents of one file over to another using shell    
     * Receive:  arguments from terminal   
     * Return:   0 or 1(error)
     * -----------------------------------------------------------------------*/
    int bufferSize, fd, fd2;            // all variables initialized
    const char *path, *destPath;        // path and destination path
    size_t nbytes;
    ssize_t bytes_read;
    int truncate = 0;
    int append = 0;
    int buffer = 0;
    int destSet = 0;
    bufferSize = 256;                   // default
    
    if (argc < 3) {                     // need atleast program call then 2 files
        printf("Need file source and destination\n");
        return 1;
    }
    for (int i = 1; i < argc; i++) {    // loops through each argument
        if (argv[i][0] == '-') {        // if leading argument has a flag, check for flag symbol
            if (argv[i][1] == 'b' && strlen(argv[i]) < 3) {
                buffer += 1;
                if (buffer > 1) {
                    printf("Error: Duplicate -b detected. Try again.\n");
                    return 1;
                }
                if (argc == i + 1) {
                    printf("Error: Not an integer or no buffer number specified. Try again\n");
                    return 1;
                }
                int x = strlen(argv[i+1]);
                for (int t=0; t < x; t++) { // Looping to make sure the argument after -b is a FULL integer. For example: 12df...not a integer.
                    if (argv[i+1][t] == '-') {
                        printf("Error: Buffer size less than 1 byte is not meaningful.\n");
                        return 1;
                    }
                    else if (!isdigit(argv[i+1][t])) { // Also detects if it  is negative
                        printf("Error: Not an integer or no buffer number specified. Try again\n");
                        return 1;
                    }
                }
                if(atoi(argv[i+1]) < 1) {
                    printf("Error: Buffer size less than 1 byte is not meaningful.\n");
                    return 1;
                }
                bufferSize = atoi(argv[i+1]);
            }
            else if (argv[i][1] == 'a' && strlen(argv[i]) < 3) {
                append += 1;
                if (append > 1) {
                    printf("Error: Duplicate -a detected. Try again.\n");
                    return 1;
                }
            }
            else if (argv[i][1] == 't' && strlen(argv[i]) < 3) {
                truncate += 1;
                if (truncate > 1) {
                    printf("Error: Duplicate -t detected. Try again.\n");
                    return 1;
                }
            }
            else {                      // wrong flag if not -a, -b, or -t
                printf("Error: Wrong flag used. Please use -a, -b, or -t and try again\n");
                return 1;
            }
        }
        else {                           // else, if no - found, enter in, could be a filename
            if(argv[i-1][1] != 'b') {    // so ignore the buffer integer
                if (destSet == 0) {      // first name found, source file gets set.
                    destSet += 1;
                    path = argv[i];
                }
                else if (destSet == 2) {      // If setting is greater then 1, it found 3 or more filenames..error
                    printf("Error: Too many files specified. Can't have more then two files (source + destination)\n");
                    return 1;
                }
                else {
                    destSet += 1;            // destination path is set. If another argument other then flags given, it will throw error
                    destPath = argv[i];
                }
            }
        }
    }
    if (destSet < 2) {
        printf("Need file source and destination\n");
        return 1;
    }
    if (strcmp(destPath, path) == 0) {
        printf("Error: source and destination file cannot be the same. Try again.\n");
        return 1;
    }
    
    if(access(path, R_OK) != 0) {
        printf("Error: insufficient permissions to read source file\n");
        return 1;
    }
    else if (access(destPath, R_OK) != 0) {
        printf("Error: insufficient permissions to read destination file\n");
        return 1;
    }
    
    fd = open(path, O_RDWR, S_IRUSR|S_IWUSR);
    
    
    if (fd < 0) {                       //cannot find file
        printf("Error: source file not found; Need a source to copy to destination.\n");
        return 1;
    }
    char *buf[bufferSize];              // sets buffer (either default or changed by -b)
    if (append == 1 && truncate == 1) {
        printf("Error: Cannot have both truncating and appending to file. Try again.\n");
        return 1;
    }
    else if (append == 1 && truncate == 0)
        fd2 = open(destPath, O_APPEND|O_RDWR, S_IRUSR|S_IWUSR);
    else if (append == 0 && truncate == 1)
        fd2 = open(destPath, O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
    else
        fd2 = open(destPath, O_RDWR, S_IRUSR|S_IWUSR);
    
    if(fd2 < 0) {
        fd2 = open(destPath, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    }
    nbytes = sizeof(buf);           // gets size of buffer and thats the count of bytes to read(can't have more then buffer in char array)
    bytes_read = read(fd, buf, nbytes);
    write(fd2, buf, bytes_read);    // writes file
    close(fd);
    close(fd2);
    return 0;
}
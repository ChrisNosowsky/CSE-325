/*******************************************************************************
 Chris Nosowsky
 Computer Project #7
 ******************************************************************************/
#include <unistd.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#define MAX 128


struct data_cache {
    int v = 0;
    int m = 0;
    int tag = 0; //13 bits wide. Use int
    unsigned short data[16]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //16 one-byte entries.
};


int main(int argc, char* argv[]) {
    /*--------------------------------------------------------------------------   
     * Name:     main   
     * Purpose:  Data cache simulation
     * Receive:  Char array command line arguments and # of arguments given
     * Return:   0 or 1(error)
     * -----------------------------------------------------------------------*/
    int refs, debug, tokenNum, lineNum, cacheLine, byteOffset, tag = 0; //initial data
    int address_int;
    char line[MAX];
    const char del[] = " \t\r\n\f"; //delimeter for separating tokens
    char *token;                    // tokens in line
    char *operation;
    unsigned int address;
    struct data_cache cache[8];     // my data_cache records array. 8 Lines so the array is size 8
    FILE *fp;                       // file pointer
    char *filename;     
    for (int i=1; i<argc; i++) {    // looping through each command line argument
        if (strcmp(argv[i], "-refs") == 0) {    // if -refs is set, set it equal to 1
            if (refs == 1) { //if refs flag called more then once
                printf("Error. Can't have two -refs flags\n");
                return 1;
            }
            refs = 1;
        }
        else if (strcmp(argv[i], "-debug") == 0) {
            if (debug == 1) { //if debug flag called more then once
                printf("Error. Can't have two -debug flags\n");
                return 1;
            }
            debug = 1;
        }
        else if(argv[i][0] == '-') {                // another flag other then debug and refs. error.
            printf("Error on: '%s' -- invalid option. Please use '-p' or '-b' for options.\n", argv[i]);
            return 1;
        }
        else {
            filename = argv[i];
            if (strcmp(argv[i-1], "-refs") !=0) { //if refs doesn't follow filename, error.
                printf("Error. Refs not followed by name of file.\n");
                return 1;
            }

        }
    }
    
    if (refs == 0) { // error if refs was never called
        printf("Error: -refs never got set.\n");
        return 1;
    }

    fp = fopen(filename, "r"); //attempt to open file
    if(fp < 0 || fp == NULL) {
        printf("Error: file not found\n");
        return 1;
    }


    if(debug == 1) { //if debug was set, then display the contents at start of sim
        printf("\nI V M TAG  DATA\n"); //header
        for (int i=0; i<8; i++) { //0-7, 8 lines total
            printf("%d %d %d %04d ", i, cache[i].v, cache[i].m, cache[i].tag);
            for (int t=0; t<16; t++) {
                printf("%02hu ", cache[i].data[t]);
            }
            printf("\n");
        }
        printf("\n");
    }

    while(fgets(line, MAX, fp) != NULL) {        // for every line, token the line and mask the physical address        
        token = strtok(line, del);
        tokenNum = 0;
        while(token != NULL) {
            if(tokenNum == 0) { //first token == physical address being referenced
                address_int = (int) strtol(token, NULL, 16);
                address = (unsigned int)address_int;
                tag = (address >> 7) & 0x00000FFF; // shift right then mask
                cacheLine = (address >> 4) & 0x0000007; //shift right then mask for cache line(3 bits) so I use 7 instead of F (0111 vs 1111)
                byteOffset = (address >> 0) & 0x000000F;
            }
            else if (tokenNum ==  1) { // operation being performed would be the 2nd token of every line
                operation = token;
            }
            tokenNum +=1;
            token = strtok(NULL, del);
        }
        printf("%05x %s %04x %01x %01x\n", address, operation, tag, cacheLine, byteOffset); //operation processed
        if (debug == 1) { //if debug was set, list contents after each memory reference processed according to 5b
            printf("\nI V M TAG  DATA\n");
            for (int i=0; i<8; i++) {
                printf("%d %d %d %04d ", i, cache[i].v, cache[i].m, cache[i].tag);
                for (int t=0; t<16; t++) { //each byte in data
                    printf("%02hu ", cache[i].data[t]);
                }
                printf("\n");
            }
            printf("\n");
        }
        lineNum += 1;
    }
    //when simulation completes, print off the data cache contents below according to 4
    printf("\nI V M TAG  DATA\n"); //header
    for (int i=0; i<8; i++) {
        printf("%d %d %d %04d ", i, cache[i].v, cache[i].m, cache[i].tag);
        for (int t=0; t<16; t++) {
            printf("%02hu ", cache[i].data[t]);
        }
        printf("\n");
    }
    printf("\n"); //for readability

            
    
}


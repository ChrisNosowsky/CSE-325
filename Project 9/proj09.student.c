/*******************************************************************************
 Chris Nosowsky
 Computer Project #9
 ******************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#define MAX 128

struct page_table {
    int v = 0;
    int p = 0;
    int r = 0;
    int m = 0;
    unsigned short int frame_num = 0;
};

int main(int argc, char* argv[]) {
    /*--------------------------------------------------------------------------   
     * Name:     main   
     * Purpose:  Memory Management simulation
     * Receive:  Char array command line arguments and # of arguments given
     * Return:   0 or 1(error)
     * -----------------------------------------------------------------------*/
    int debug = 0;
    int refs = 0;
    int tokenNum = 0;
    int pageOffset = 0; // page offset
    int pageNum = 0; //initial data
    int address_int;
    int total_write = 0; //initial total write operations
    int total_read = 0; // initial total read operations
    int total_mem_refs = 0; // initial total memory references
    
    char line[MAX];
    const char del[] = " \t\r\n\f"; //delimeter for separating tokens
    char *token;                    // tokens in line
    char *operation;                // read or write?
    unsigned int address;           // address variable
    struct page_table pt[16];     // my page table record array
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
            printf("Error on: '%s' -- invalid option. Please use '-refs' or '-debug' for options.\n", argv[i]);
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
        printf("\n     V P R M Frame\n");    //format follows instructors
        for (int i=0; i<16; i++) { //16 lines total
            printf("[%01x]: %01x %01x %01x %01x %02x ", i, pt[i].v, pt[i].p, pt[i].r, pt[i].m, pt[i].frame_num);
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
                pageNum = (address >> 12) & 0x0000000F; // shift right then mask
                pageOffset = (address >> 0) & 0x0000FFF;
            }
            else if (tokenNum ==  1) { // operation being performed would be the 2nd token of every line
                operation = token;
                if(strcmp(operation, "R") == 0) {  // keeping count of read operations
                    total_read += 1;
                }
                else if(strcmp(operation, "W") == 0) { // keeping count of write operations
                    total_write += 1;
                }
                total_mem_refs += 1; // just increasing the memory reference by 1 for every line in the file
            }
            tokenNum +=1;
            token = strtok(NULL, del);
        }
        
        printf("%04x %s %01x %03x\n", address, operation, pageNum, pageOffset); //operation processed
        
        if (debug == 1) { //if debug was set, list contents after each memory reference processed according to 5b
            printf("\n     V P R M Frame\n");
            for (int i=0; i<16; i++) { // 16 lines total
                printf("[%01x]: %01x %01x %01x %01x %02x ", i, pt[i].v, pt[i].p, pt[i].r, pt[i].m, pt[i].frame_num);
                printf("\n");
            }
            printf("\n");  
        }
        
    }
    if (debug == 0) { // debug off? just display page frame contents at the end
        printf("\n     V P R M Frame\n");
        for (int i=0; i<16; i++) {
            printf("[%01x]: %01x %01x %01x %01x %02x ", i, pt[i].v, pt[i].p, pt[i].r, pt[i].m, pt[i].frame_num);
            printf("\n");
        }
        printf("\n");  
    }
    printf("Total Number of Memory References:  %d\n",total_mem_refs); // displaying the total number of memory references, read and write with the correct labels and format
    printf("Total Number of Read Operations:    %d\n", total_read);
    printf("Total Number of Write Operations:   %d", total_write);
    printf("\n");
    
}
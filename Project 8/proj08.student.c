/*******************************************************************************
 Chris Nosowsky
 Computer Project #8
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
    unsigned int old_address; //I wanted the old address tied to a specific data cache line in case I had to store another cache line old address before the current local old_address gets written back
    unsigned short int data[16]; //16 one-byte entries.
};

int main(int argc, char* argv[]) {
    /*--------------------------------------------------------------------------   
     * Name:     main   
     * Purpose:  Data cache simulation
     * Receive:  Char array command line arguments and # of arguments given
     * Return:   0 or 1(error)
     * -----------------------------------------------------------------------*/
    int debug = 0;
    int refs = 0;
    int tokenNum = 0;
    int lineNum = 0;
    int cacheLine = 0;
    int byteOffset = 0;
    int tag = 0; //initial data
    int address_int;
    char line[MAX];
    const char del[] = " \t\r\n\f"; //delimeter for separating tokens
    char *token;                    // tokens in line
    char *operation;                // read or write?
    char *HM;                       // hit or miss?
    unsigned int address;           // address variable
    unsigned short cpu[4] = {0, 0, 0, 0};   //cpu(copy 4 bytes)
    unsigned short int RAM[1048576];        // my RAM of unsigned short int!
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
        printf("\n     V M Tag   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");    //format follows instructors
        for (int i=0; i<8; i++) { //0-7, 8 lines total
            printf("[%01x]: %01x %01x %04x ", i, cache[i].v, cache[i].m, cache[i].tag);
            for (int t=0; t<16; t++) {
                    if(t == 15) {
                        printf("%02x", cache[i].data[t]);
                    }
                    else {
                        printf("%02x ", cache[i].data[t]);
                    }
            }
            printf("\n");
        }
        printf("\n");
    }
    int counter = 0;
    while(fgets(line, MAX, fp) != NULL) {        // for every line, token the line and mask the physical address
        token = strtok(line, del);
        tokenNum = 0;
        counter = 0;
        while(token != NULL) {
            
            if(tokenNum == 0) { //first token == physical address being referenced
                
                address_int = (int) strtol(token, NULL, 16);
                address = (unsigned int)address_int;
                tag = (address >> 7) & 0x00000FFF; // shift right then mask
                cacheLine = (address >> 4) & 0x000000F;
                byteOffset = (address >> 0) & 0x000000F;
                
            }
            else if (tokenNum ==  1) { // operation being performed would be the 2nd token of every line
                operation = token;
            }
            else {                      // if the token count is greater then 2, then it is a write and it is giving us 4 bytes to write
                cpu[counter] = (int) strtol(token, NULL, 16);       // convert the char* token to a base 16 integer value
                counter += 1;                                       // advance through array(should only ever reach 4)
            }
            tokenNum +=1;
            token = strtok(NULL, del);
        }
        
        if(!(cache[cacheLine].v == 1 && cache[cacheLine].tag == tag)) {    //logic follows that from proj08.cache.pdf 
            HM = (char*)"M";                                                // setting my hit or miss variable to miss if valid is not set or tag not equal to what is currently there in cache
            if(cache[cacheLine].v == 1 && cache[cacheLine].m == 1) {        // W R I T E B A C K
                for(int i=0;i<16;i++) {                                     // loops through each byte in data block and stores in RAM if write back
                    RAM[cache[cacheLine].old_address + i] = cache[cacheLine].data[i];
                } 
            }
            
            for(int i=0;i<16;i++) {                                         // COPIES FROM RAM(CURRENT ADDRESS) TO DATA CACHE
                cache[cacheLine].data[i] = RAM[address - byteOffset +i];
            }
            
            cache[cacheLine].m = 0;
            cache[cacheLine].tag = tag;
            cache[cacheLine].v = 1;
        }
        else {
            HM = (char*)"H";                                                // if we didn't miss, then we obviously hit (tag equal and valid 1)
        }
        if(strcmp(operation, "R") == 0) {                                   // read operation
            for(int i = 0; i < 4; i++) {
                cpu[i] = cache[cacheLine].data[byteOffset + i];
            }
        }
        else if(strcmp(operation, "W") == 0) {                              // write operation
            for(int i=byteOffset; i < byteOffset + 4; i++) {                // grabs from cpu and stores into our byte offset at our specified index
                cache[cacheLine].data[i] = cpu[i - byteOffset];
            }
            cache[cacheLine].m = 1;
            cache[cacheLine].old_address = address & 0xFFFFFF0;             // if M is set, then we modified. So when writeback gets called later, it grabs old address(stripped from it's "offset")
        }
        
        printf("%05x %s %04x %01x %01x %s ", address, operation, tag, cacheLine, byteOffset, HM); //operation processed
        for(int i=0; i<4; i++) {
            if(i == 3) {
              
                printf("%02x", cache[cacheLine].data[byteOffset + i]);
            }
            else {
                printf("%02x ", cache[cacheLine].data[byteOffset + i]);
            }   
        }
        printf("\n");
        
        
        if (debug == 1) { //if debug was set, list contents after each memory reference processed according to 5b
            printf("\n     V M Tag   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
            for (int i=0; i<8; i++) {
                printf("[%01x]: %01x %01x %04x ", i, cache[i].v, cache[i].m, cache[i].tag);
                for (int t=0; t<16; t++) { //each byte in data
                    if(t == 15) {
                        printf("%02x", cache[i].data[t]);
                    }
                    else {
                        printf("%02x ", cache[i].data[t]);
                    }
                    
                }
                printf("\n");
            }
            printf("\n");  
        }
        
        lineNum += 1;
    }
    
    if (debug == 0) {
        printf("\n     V M Tag   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        for (int i=0; i<8; i++) {
            printf("[%01x]: %01x %01x %04x ", i, cache[i].v, cache[i].m, cache[i].tag);
            for (int t=0; t<16; t++) { //each byte in data
                if(t == 15) {
                    printf("%02x", cache[i].data[t]);
                }
                else {
                    printf("%02x ", cache[i].data[t]);
                }

            }
            printf("\n");
        }
        printf("\n");  
    }
    for(int i = 0; i < 8; i++) {                    //128 bytes/16 = 8 lines of subset RAM
        printf("%05x: ", 131072 + 16*i);            // decimal of 20000(base 16). It prints in hex with the formatted 'x'.
        for(int t=0; t<16;t++) {
            if(t==15)   // no space
                printf("%02x", RAM[131072 + 16*i + t]);
            else
                printf("%02x ", RAM[131072 + 16*i + t]);
        }
        printf("\n");
    }
    printf("\n");
    
}


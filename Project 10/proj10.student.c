/*******************************************************************************
 Chris Nosowsky
 Computer Project #10
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
     * Return:   0 or 1 (error)
     * -----------------------------------------------------------------------*/
    int debug = 0;                          // flags for debug
    int refs = 0;                           // flags for refs
    int tokenNum = 0;
    int pageOffset = 0;                     // page offset
    int pageNum = 0;                        // initial data
    char writeBack = '\0';                  // Write back to blank or B
    char pageFault = '\0';                  // Page fault to blank or F
    unsigned int physicalAddr;              // Physical Address var
    int address_int;
    int total_write = 0;                    // initial total write operations
    int total_read = 0;                     // initial total read operations
    int total_page_faults = 0;              // initial total memory references
    int total_write_backs = 0;
    unsigned short int freeFrames[100];     // Free frames list
    int lineNum = 0;                        // For the config file, skipping LRU line
    int tokenInt = 0;                      
    int frameAmount = 0;
    int free = 0;
    int LRU = 0;
    int timeTags[16] = {0};                       // Keeping track of time for each page that's present
    int timeTracker = 1;
    int temp;
    unsigned short int freeChoice;
    char line[MAX];
    const char del[] = " \t\r\n\f";         // delimeter for separating tokens
    char *token;                            // tokens in line
    char *operation;                        // read or write?
    unsigned int address;                   // address variable
    
    int fatalError = 0;
    struct page_table pt[16];               // my page table record array
    FILE *fp;                               // file pointer
    FILE *fpConfig;
    char *filename;     
    for (int i=1; i<argc; i++) {            // looping through each command line argument
        if (strcmp(argv[i], "-refs") == 0) {    // if -refs is set, set it equal to 1
            if (refs == 1) {                    // if refs flag called more then once
                printf("Error. Can't have two -refs flags\n");
                return 1;
            }
            refs = 1;
        }
        else if (strcmp(argv[i], "-debug") == 0) {
            if (debug == 1) {                   // if debug flag called more then once
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
            if (strcmp(argv[i-1], "-refs") !=0) { // if refs doesn't follow filename, error.
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
        printf("Error: command line file not found\n");
        return 1;
    }
    fpConfig = fopen("config", "r");
    if(fpConfig < 0 || fpConfig == NULL) {
        printf("Error: config file not found\n");
        return 1;
    }
    
    if(debug == 1) {                            // if debug was set, then display the contents at start of sim
        printf("\n     V P R M Frame\n");       // format follows instructors
        for (int i=0; i<16; i++) {              // 16 lines total
            printf("[%01x]: %01x %01x %01x %01x %02x ", i, pt[i].v, pt[i].p, pt[i].r, pt[i].m, pt[i].frame_num);
            printf("\n");
        }
        printf("\n");
    }
    //////////////////////////////////////////////////////////
    while(fgets(line, MAX, fpConfig) != NULL) {         // While loop for config file
        tokenNum = 0;
        lineNum += 1;
        token = strtok(line, del);
        while(token != NULL) {
            if (lineNum == 2) {                         // If second line, store frame amount and freeFrames in list
                if(tokenNum == 0) {
                    frameAmount = atoi(token);
                }
                if (tokenNum > 0) {
                    freeFrames[tokenNum - 1] = (int) strtol(token, NULL,16);
                }
            }
            else if(lineNum == 3) {                     // if third line, set each token besides first one(the page count) to be valid!
                if (tokenNum > 0) {
                    tokenInt = (int) strtol(token, NULL, 16);
                    pt[tokenInt].v = 1;
                }
            }
            tokenNum +=1;
            token = strtok(NULL, del);
        }
    }

    while(fgets(line, MAX, fp) != NULL) {        // for every line, token the line and mask the physical address
        token = strtok(line, del);
        tokenNum = 0;
        fatalError = 0;
        pageFault = '\0';
        writeBack = '\0';
        while(token != NULL) {

            if(tokenNum == 0) { //first token == physical address being referenced
                address_int = (int) strtol(token, NULL, 16);
                address = (unsigned int)address_int;
                pageNum = (address >> 12) & 0x0000000F; // shift right then mask
                pageOffset = (address >> 0) & 0x0000FFF;
            }
            else if (tokenNum ==  1) { // operation being performed would be the 2nd token of every line
                operation = token;
            }
            tokenNum +=1;
            token = strtok(NULL, del);
        }
        
        if(pt[pageNum].v == 0) {
            fatalError = 1;
        }
        else {
            if(pt[pageNum].v == 1 && pt[pageNum].p == 0) {  // page fault processing below
                //interrupt
                free = 0;
                total_page_faults += 1;
                pageFault = 'F';
                for(int i=0; i<frameAmount; i++) {          // loop through free frames and check if any free(list will be all 0xfff if not free)
                    
                    if(freeFrames[i] != 0xfff) {            // if a page is free enter if statement
                        free = 1;
                        freeChoice = freeFrames[i];         // frame that's free
                        freeFrames[i] = 0xfff;
                        break;
                    }
                }
                if(free==0) {                               // free frame list is empty
                                                            // select page which is curr res in ram as the victim
                    for(int i=0; i<16; i++) {               // if not free, Find the LRU HERE!!!(the LRU holds the smallest time tag of 1)
                        if(timeTags[i] == 1) {
                            LRU = i;
                            timeTags[i] = 0;                // now that LRU is found, set that to 0 since it won't be present, it's getting evicted!
                            break;
                        }
                    }
                    if(pt[LRU].m == 1) {                    // algorithm logic, write back called if the LRU entry holds a modified bit
                        writeBack = 'B';
                        total_write_backs +=1;              // increase the total write back count by 1
                    }
                    pt[LRU].p = 0;                          // set LRU PTE present bit to 0 (we are evicting that is why)
                    freeFrames[0] = pt[LRU].frame_num;      // moving my frame number in the LRU PTE to the free frames list
                    freeChoice = freeFrames[0];
                    freeFrames[0] = 0xfff;
                }
                                                            // VVVVVV otherwise if freeframe list is NOT empty BELOW VVVVVV
                pt[pageNum].frame_num = freeChoice;
                pt[pageNum].p = 1;
                pt[pageNum].m = 0;
                pt[pageNum].r = 0;
                if(timeTracker <= frameAmount) {            // if all frame list slots has NOT been allocated, log time here
                    timeTags[pageNum] = timeTracker;
                    timeTracker += 1;
                }
                else {                                      // if all frames in use, update the NEW PTE that is present to hold the most recent time (highest time tag number)
                    for(int i=0; i<16; i++) {               // Time tags for all other ones will decrement by 1
                        if(timeTags[i] > 0) {
                            timeTags[i] -= 1;
                        }
                    }
                    if(timeTags[pageNum] != frameAmount) {
                        timeTags[pageNum] = frameAmount;
                    }
                }
            } //end page fault processing
            else if(pt[pageNum].v == 1 && pt[pageNum].p == 1) {
                temp = timeTags[pageNum];
                if(timeTags[pageNum] != timeTracker - 1) {
                    for(int i=0; i<16; i++) {               // same logic as lines 206:218 above
                        if(temp != 1 && timeTags[i] == 1) {
                            continue;
                        }
                        else if(timeTags[i] > 0) {
                            timeTags[i] -= 1;
                        }
                    }
                    if(timeTags[pageNum] != timeTracker - 1) {
                        timeTags[pageNum] = timeTracker - 1;
                    }
                }
            }
            if(strcmp(operation, "R") == 0) {           // read processing, increase read count
                total_read += 1;
                pt[pageNum].r = 1;
            }
            else if(strcmp(operation, "W") == 0) {      // write processing, increase write count
                total_write += 1;
                pt[pageNum].r = 1;
                pt[pageNum].m = 1;
            }
            physicalAddr = (pt[pageNum].frame_num << 12) | (pageOffset);    // build the physical by combining frame (2 hex) with the pageOffset(3 hex)
        }
        
        if(fatalError == 1) {               // IF WE TRIED TO ACCESS A PTE THAT DOES NOT HOLD A VALID BIT!!!!!!!!!!!!!!!!!!!!!
            printf("Fatal error: Invalid memory reference! (%04x)\n", address);
        }
        else {
            if((pageFault =='F' && writeBack != 'B') || (pageFault != 'F' && writeBack == 'B')) { // why 3 lines? Odd spacing for missing F and B, that is why
                printf("%04x %s %01x %03x %c %c  %05x\n", address, operation, pageNum, pageOffset, pageFault, writeBack, physicalAddr);
            } else if(pageFault =='F' && writeBack == 'B') {
                printf("%04x %s %01x %03x %c %c %05x\n", address, operation, pageNum, pageOffset, pageFault, writeBack, physicalAddr);
            } else {
                printf("%04x %s %01x %03x %c %c   %05x\n", address, operation, pageNum, pageOffset, pageFault, writeBack, physicalAddr);
            }
        }
        
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
    printf("Total Number of Read Operations:    %d\n", total_read);     // all counts at end of simulation
    printf("Total Number of Write Operations:   %d\n", total_write);
    printf("Total Number of Page Faults:        %d\n", total_page_faults);
    printf("Total Number of Write Backs:        %d", total_write_backs);
    printf("\n");
}
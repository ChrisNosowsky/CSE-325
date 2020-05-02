/*******************************************************************************
 Chris Nosowsky
 Computer Project #12 - Server File
 ******************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#define MAX 128

int main(int argc, char* argv[]) {
        
    if (argc > 1) {                     // if any arguments provided, warn user that no arguments need to be provided
        for(int i =1; i<argc; i++) {
            fprintf(stderr, "Warning: option (%s) ignored.\n", argv[i]);
        }
    }
    
    int listener_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_sd < 0) {              // error check if socket returns -1
        fprintf(stderr, "Error: Socket creation failed.\n");
        exit(1);
    }
    
    struct sockaddr_in saddr;       // Data structure for socket
    char name[128];
    gethostname(name, 128);
    char buffer[MAX];               // our buffer variable
    
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family = AF_INET;     // TCP connection-based
    saddr.sin_port = htons(0);      // htons of 0 to let the OS select our port
    saddr.sin_addr.s_addr = htonl( INADDR_ANY );
    
    int bstat = bind(listener_sd, (struct sockaddr *) &saddr, sizeof(saddr));
    if (bstat < 0) {                // error checking on binding the socket
        fprintf(stderr, "Error: failed to bind address to socket.\n");
        exit(2);
    }
    
    socklen_t saddr_size = sizeof(saddr);
    getsockname(listener_sd, (struct sockaddr *) &saddr, &saddr_size);
    
    int lstat = listen(listener_sd, 5);
    if (lstat < 0) {                // error checking on listen
        fprintf(stderr, "Error: failed to setup listener to accept incoming connection requests.\n");
        exit(3);
    }
    
    struct hostent* host;
    host = gethostbyname(name);     // getting the hostname(eg. arctic.cse.msu.edu)

    printf("\n%s %d\n\n", host->h_name, ntohs(saddr.sin_port));     // prints host followed by port it is listening for connections on
    
    struct sockaddr_in caddr;
    unsigned int clen = sizeof(caddr);
    int comm_sd = accept(listener_sd, (struct sockaddr *) &caddr, &clen);   // accepting the client connection
    if (comm_sd < 0) {              // error on failing to accept the incoming connection
        fprintf(stderr, "Error: failed to accept connection on socket.\n");
        exit(4);
    }

    bzero(buffer, MAX);
    int nrcv = recv(comm_sd, buffer, MAX, 0);           // wait to receive the file to open
    if(nrcv < 0) {
        fprintf(stderr, "Error: Problem receiving message.\n");
        exit(5);
    }

    const char *win = "SUCCESS";
    const char *lose = "FAILURE";
    int nsend;

    int opfd = open(buffer, O_RDONLY, S_IRUSR);   // attempts to open the provided file
    if (opfd < 0) { // if error in opening file, then send to client "FAILURE" and terminate execution
        nsend = send(comm_sd, lose, strlen(lose), 0);
        if(nsend < 0) {
            fprintf(stderr, "Error: Problem sending message.\n");
            exit(6);
        }
        exit(7);
    }

    nsend = send(comm_sd, win, strlen(win), 0);
    if(nsend < 0) {     // error check on sending message "SUCCESS" to client
        fprintf(stderr, "Error: Problem sending message.\n");
        exit(8);
    }

    bzero(buffer, MAX);
    nrcv = recv(comm_sd, buffer, MAX, 0);
    if(nrcv < 0) {
        fprintf(stderr, "Error: Problem receiving message.\n");
        exit(9);
    }

    if(strcmp(buffer, "PROCEED") != 0) {        // if PROCEED wasn't provided, terminate, otherwise continue to read file contents
        fprintf(stderr, "Error: did not receive 'PROCEED' message.\n");
        exit(10);
    }
    
    while(1) {      // loops through contents of file and sends back file contents to client until EOF
        bzero(buffer, MAX);
        ssize_t nbytes = read(opfd, buffer, MAX);       // reading file and storing up to MAX into buffer
        if (nbytes < 0) {
            fprintf(stderr, "Error: Reading error.\n");
            exit(11);
        }
        if (nbytes == 0) {
            break;
        }
        nsend = send(comm_sd, buffer, nbytes, 0);
        if(nsend < 0) {
            fprintf(stderr, "Error: Problem sending file contents.\n");
            exit(12);
        }

    }


    close(comm_sd);         // closing both connections and terminating execution
    close(listener_sd);
    
}
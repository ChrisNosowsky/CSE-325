/*******************************************************************************
 Chris Nosowsky
 Computer Project #11
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
#define MAX 128

// All print statements outside of the file contents in this project are sent to stderr stream

int main(int argc, char* argv[]) {
    int sd_client;                  // socket creation
    int stat;                       // Status for the connection
    int count;                      // Send count
    int countProceed;               // Sending "PROCEED"
    struct sockaddr_in saddr;       // Data structure for socket
    
    if (argc != 4) {                // error check on correct number of arguments
        fprintf(stderr, "Error: Please provide host, port and file to send\n");
        exit(1);
    }
    char *host = argv[1];           // second argument is the host
    unsigned short int port = atoi(argv[2]);
         
    sd_client = socket(AF_INET, SOCK_STREAM, 0); // IPV4 Protocol, TCP Connection, default protocol
    if (sd_client < 0) {
        fprintf(stderr, "Error: Socket creation failed\n");
        exit(2);
    }
    
    struct hostent *server = gethostbyname(host);
    if (server == NULL) {           // error check on the server
        fprintf(stderr, "Error: no such host %s\n", host);
        exit(3);
    }
    
    bzero(&saddr, sizeof(saddr));   // server address configuration    
    saddr.sin_family = AF_INET;     // TCP connection-based
    saddr.sin_port = htons(port);   // argument port provided
    bcopy(server->h_addr, &saddr.sin_addr.s_addr, server->h_length );
    
    stat = connect(sd_client, (struct sockaddr *) &saddr, sizeof(saddr));
    if(stat < 0) {                  // error connecting to server address
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(4);
    }
      
    char buffer[MAX];               // buffer is 128 bytes MAX
    strcpy(buffer, argv[3]);
    count = send(sd_client, buffer, strlen(buffer), 0);
    if (count < 0) {
        fprintf(stderr, "Error: Error sending message to server\n");
        exit(5);
    }
    
    bzero(buffer, MAX);  
    int countReceive = recv(sd_client, buffer, MAX, 0);     // Server ACK
    if (countReceive < 0) { // NACK
        fprintf(stderr, "Error: Error receiving message from server\n");
        exit(6);
    }
    
    if (strcmp(buffer, "SUCCESS") != 0) {
        fprintf(stderr, "Error: Client unable to open specified file name as an input file\n");
        exit(7);
    }
    bzero(buffer, MAX); 
    strcpy(buffer, "PROCEED");      // Sent to server as an ACK that client received message
    countProceed = send(sd_client, buffer, strlen(buffer), 0);
    if(countProceed < 0) {
        fprintf(stderr, "Error: Error sending proceed message to server\n");
        exit(8);
    }
    
    bzero(buffer, MAX);
    int countReceiveContent;
    while((countReceiveContent = recv(sd_client, buffer, MAX, 0)) >  0) {
        buffer[128] = '\0';         // Ran into encoding problems, need a null terminating byte.
        write(1, buffer, strlen(buffer));       // MY WRITE
        bzero(buffer, MAX);                     // Zeros out buffer for next iteration
    }
    if (countReceiveContent < 0) {
        fprintf(stderr, "Error: Error receiving message from server for file content\n");
        exit(9);
    }
    
    close(sd_client);                           // close the socket
}
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

const int COMMAND_LENGTH = 1024;
const int RESULT_LENGTH = 10240;
const int BUFFER_LENGTH = 1024;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void execCommand(char *command, char *result) {
    FILE *fp;
    int LINE_LENGTH = 1024;
    char line_contents[LINE_LENGTH];
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    while (fgets(line_contents, sizeof(line_contents)-1, fp) != NULL) {
        strcpy(result, line_contents);
        printf("%s", line_contents);
    }
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char command[COMMAND_LENGTH];
    char result[RESULT_LENGTH];
    char buffer[BUFFER_LENGTH];
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Please enter the cluster command: ");
    bzero(command,COMMAND_LENGTH);
    fgets(command,COMMAND_LENGTH - 1,stdin);
    execCommand(command, result);
    n = write(sockfd, result, strlen(result));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,BUFFER_LENGTH);
    n = read(sockfd, buffer, BUFFER_LENGTH - 1);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}
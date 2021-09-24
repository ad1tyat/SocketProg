/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "message.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

#define MAXN 1005

int PRICE[MAXN];
char *desc[MAXN];

void init() {
    FILE *databasePtr;
    databasePtr = fopen("database.txt", "r");
    if (databasePtr == NULL) {
        printf("database.txt failed to open.");
        exit(-1);
    }
    int x;
    char prodDesc[50];
    for (int i = 0; i < 5; i++) {
        fscanf(databasePtr, "%d %s\n", &x, prodDesc);
        PRICE[i] = x;
        desc[i] = prodDesc;
    }
    fclose(databasePtr);
}

void decode(char *string){

}
void encode(Message message){
	int req_type = message.Request_type;
	int upc_code = message.UPC_CODE;
	int number = message.number;
}

int main(int argc, char *argv[]) {
    init();
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept");
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n", buffer);
    n = write(newsockfd, "I got your message", 18);
    if (n < 0) error("ERROR writing to socket");
    close(newsockfd);
    close(sockfd);
    return 0;
}
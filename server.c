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

void populate() {
    FILE *databasePtr;
    databasePtr = fopen("database.txt", "r");
    if (databasePtr == NULL) {
        printf("database.txt failed to open.");
        exit(-1);
    }
    int x, upc;
    char prodDesc[50];
    for(int i = 0;i<1000;i++){
        PRICE[i] = -1;
        desc[i] = "UPC CODE NOT DEFINED";
    }
    while(fscanf(databasePtr, "%d %d %s\n",&upc, &x, prodDesc) > 0){
        PRICE[upc] = x;
        desc[upc] = prodDesc;
        printf("%d %d %s", upc, x, prodDesc);
    }
    fclose(databasePtr);
}

int main(int argc, char *argv[]) {
    populate();
    // Server setup
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr;
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

    // Accept new Client request
    struct sockaddr_in cli_addr;
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) error("ERROR on accept");

    int items = 0;

    while(1){
        // Read Request
        bzero(buffer, 256);
        n = read(newsockfd, buffer, 255);
        if (n < 0) error("ERROR reading from socket");
        printf("Request: %s\n", buffer);

        RequestMessage req = decode_request(buffer);
        ResponseMessage resp;
        if(req.Request_type == 0){
            resp.Response_type = 0;
            resp.item = 1;
            items += req.number;
            strcpy(resp.name, "Hey!");
            resp.price = 678;
        }       
        else if(req.Request_type == 1){
            resp.Response_type = 0;
            resp.item = 0;
            resp.total_amount = items;
        }

        // Send Response
        bzero(buffer, 256); 
        encode_response(resp, buffer);
        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0) error("ERROR writing to socket");

        // If closing request, break out
        if(req.Request_type == 1) break;
    }
    
    close(newsockfd);
    close(sockfd);
    return 0;
}
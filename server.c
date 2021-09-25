/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#include "message.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

#define MAXN 1005

// Database Arrays
int PRICE[MAXN];
char desc[MAXN][55];

// Log file
FILE *logger;

// SIGINT indicator
int sigint_received = 0;

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
        strcpy(desc[i], "UPC CODE NOT DEFINED");
    }
    while(fscanf(databasePtr, "%d %d %s\n",&upc, &x, prodDesc) > 0){
        PRICE[upc] = x;
        strcpy(desc[upc], prodDesc);
        fprintf(logger, "Populated : %d %d %s\n", upc, x, prodDesc);
    }
    fclose(databasePtr);
}

void clientHandler(int newsockfd){
    char buffer[256];
    int total_amount = 0;
    int n;

    while(!sigint_received){
        // Read Request
        bzero(buffer, 256);
        n = read(newsockfd, buffer, 255);
        if (n < 0) {
            if(errno == EINTR) continue;
            error("ERROR reading from socket");
        }
        fprintf(logger, "Raw Request: %s\n", buffer);
        
        RequestMessage req = decode_request(buffer);
        ResponseMessage resp;
        if(req.Request_type == 0){
            int upc_code = req.UPC_CODE;
            fprintf(logger, "upc: %d\n", upc_code);
            if(PRICE[upc_code] == -1){
                // UPC code doesn't exist
                resp.Response_type = 1;
                strcpy(resp.error, err2); //err2 in message.h
            }
            else{
                total_amount += PRICE[upc_code] * req.number;
                resp.Response_type = 0;
                resp.item = 1;
                strcpy(resp.name, desc[upc_code]);
                resp.price = PRICE[upc_code];
                fprintf(logger, "response: %s %d\n", resp.name, resp.price);
            }
            
        }       
        else if(req.Request_type == 1){
            // Closing Request, return total amount
            resp.Response_type = 0;
            resp.item = 0;
            resp.total_amount = total_amount;
        }

        // Send Response
        bzero(buffer, 256); 
        encode_response(resp, buffer);
        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0) error("ERROR writing to socket");

        // If closing request, break out
        if(req.Request_type == 1) break;
    }
    
}

void sigint_handler(int signum){
    sigint_received = 1;
}

void sigchld_handler(int signum){
    pid_t child_pid;
    int stat;
    child_pid = wait(&stat);
    return;
}

int main(int argc, char *argv[]) {
    // Registering Signal Handlers
    struct sigaction new_action;
    new_action.sa_handler = sigint_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, &new_action, NULL);

    new_action.sa_handler = sigchld_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGCHLD, &new_action, NULL);

    // Logging
    logger = fopen("serverlog.log","a+");
    if(logger == NULL){
        printf("serverlog.log failed to open.");
        exit(-1);
    }
    populate();

    // Server setup
    int sockfd, newsockfd, portno;
    socklen_t clilen;
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

    // Accept new Client request
    while(!sigint_received){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0){
            if(errno == EINTR){
                // system call was interrupted
                continue;
            }
            // Accept Error
            printf("Error accepting connection\n");
        }
        else{
            int childpid = fork();
            if(childpid == 0){
                close(sockfd);
                clientHandler(newsockfd);
                close(newsockfd);
                exit(0);
            }
        }
    }
    // Cleanup
    fclose(logger);
    close(sockfd);
    return 0;
}
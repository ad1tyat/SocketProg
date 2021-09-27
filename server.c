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

#define MAXN 1005
FILE *logger;

void error_exit_sys(const char *msg){
    perror(msg);
    exit(1);
}

// Signal handling
int sigint_received = 0;

void sigint_handler(int signum){
    sigint_received = 1;
}

void sigchld_handler(int signum){
    pid_t child_pid;
    int stat;
    child_pid = wait(&stat);
    return;
}

void register_signal_handler(int signum, void (*handler)(int)){
    struct sigaction new_action;
    new_action.sa_handler = handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(signum, &new_action, NULL);
}

// Database
int PRICE[MAXN];
char desc[MAXN][55];

void populate_database() {
    FILE *databasePtr;
    databasePtr = fopen("database.txt", "r");
    if (databasePtr == NULL) {
        fprintf(stderr, "populate_database: Failed to read database file. Please check 'database.txt' exists");
        exit(1);
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

void bind_server(int sockfd, int portno){
    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error_exit_sys("ERROR in socket binding\n");
}

// Client Connection Handler
void client_handler(int newsockfd);

int main(int argc, char *argv[]) {
    // Logging
    logger = fopen("serverlog.log","a+");
    if(logger == NULL){
        fprintf(stderr, "ERROR, Failed to create serverlog.log.");
        exit(1);
    }
    // Signal Handling
    register_signal_handler(SIGINT, sigint_handler);
    register_signal_handler(SIGCHLD, sigchld_handler);
    // Server Setup
    int sockfd, portno;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    portno = atoi(argv[1]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error_exit_sys("ERROR in opening socket\n");
    }
    bind_server(sockfd, portno);
    populate_database();
    listen(sockfd, 5);

    // Request Handling Loop
    struct sockaddr_in cli_addr;
    int newsockfd;
    socklen_t clilen;
    while(!sigint_received){
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0){
            // An interrupt had occured
            if(errno == EINTR) continue;
            // Otherwise, an Error has occurred
            perror("ERROR in accepting connection\n");
        }
        else{
            // Fork a child to handle the connection
            int childpid = fork();
            if(childpid == 0){
                close(sockfd);
                client_handler(newsockfd);
                close(newsockfd);
                exit(0);
            }
        }
    }

    // End Server
    fclose(logger);
    close(sockfd);
    return 0;
}

void client_handler(int newsockfd){
    char buffer[MAX_MESSAGE_SIZE + 1];
    int total_amount = 0;
    int n;

    while(!sigint_received){
        // Read Request
        bzero(buffer, sizeof buffer);
        n = read(newsockfd, buffer, MAX_MESSAGE_SIZE);
        if (n < 0) {
            if(errno == EINTR) continue;
            error_exit_sys("ERROR in reading from socket\n");
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
        bzero(buffer, sizeof buffer); 
        encode_response(resp, buffer);
        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0) {
            error_exit_sys("write: Error writing to socket\n");
        }

        // If closing request, break out
        if(req.Request_type == 1) break;
    }   
}
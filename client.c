#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "message.h"

#define MIN_UPC_CODE 0
#define MAX_UPC_CODE 999
FILE *logger;

void error_exit_sys(const char *msg)
{
    perror(msg);
    exit(0);
}

void connect_to_server(struct hostent *server, int sockfd, int portno){
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error_exit_sys("ERROR in connecting to server\n");
}

int main(int argc, char *argv[]){
    logger = fopen("clientlog.log","a+");
    if(logger == NULL){
        fprintf(stderr, "Failed to open clientlog.log");
        exit(1);
    }

    int sockfd, portno, n;
    struct hostent *server;
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error_exit_sys("ERROR opening socket\n");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    connect_to_server(server, sockfd, portno);
    
    // The Request Loop
    char buffer[MAX_MESSAGE_SIZE + 1];
    printf("Welcome\n");
    printf("************************\n");
    while(1){
        RequestMessage req;
        printf("Enter Req_Type (0: Order, 1: Finish): ");
        scanf("%d", &(req.Request_type));
        if(req.Request_type == 0){
            int correct = 1;
            do{
                printf("Enter Item Code and Quantity:\n");
                scanf("%d %d", &req.UPC_CODE, &req.number);
                correct = 1;
                if(req.UPC_CODE < MIN_UPC_CODE || req.UPC_CODE > MAX_UPC_CODE){
                    printf("Invalid: Item code must lie between 0 and 999. Please try again.\n");
                    correct = 0;                
                }
                if(req.number <= 0){
                    printf("Invalid: Quantity must be greater than 0. Please try again.\n");
                    correct = 0;                
                }
            }while(!correct);
        }
        // Make and Send Request
        bzero(buffer, sizeof buffer);
        encode_request(req, buffer);
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) 
            error_exit_sys("ERROR writing to socket\n");
        
        // Read Response
        bzero(buffer, sizeof buffer);
        n = read(sockfd, buffer, MAX_MESSAGE_SIZE);
        if (n < 0) 
            error_exit_sys("ERROR reading from socket\n");
        else if(n == 0){
            fprintf(stderr, "ERROR: Server closed connection prematurely\n");
            exit(0);
        }
        fprintf(logger, "Raw Response: %s\n",buffer);
        
        ResponseMessage resp = decode_response(buffer, req.Request_type);
        if(resp.Response_type == 0){
            // Status OK
            if(req.Request_type == 0){
                printf("Item: %s, Price: %d\n", resp.name, resp.price);
            }
            else if(req.Request_type == 1){
                printf("Total Amount: %d\n", resp.total_amount);
            }
        }
        else{
            // ERROR
            printf("Error: %s\n", resp.error);
        }
        printf("************************\n");
        // Closing Request, Break out
        if(req.Request_type == 1) break;
    }
    printf("Thank You For Shopping with us !\n");
    close(sockfd);
    fclose(logger);
    return 0;
}
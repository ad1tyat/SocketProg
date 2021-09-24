#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "message.h"

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
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
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // The Request Loop
    while(1){
        RequestMessage req;
        printf("Enter Req_Type: ");
        scanf("%d", &(req.Request_type));
        if(req.Request_type == 0){
            printf("Enter Item Code and Quantity:\n");
            scanf("%d %d", &req.UPC_CODE, &req.number);
        }
        // Make and Send Request
        bzero(buffer,256);
        encode_request(req, buffer);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) 
            error("ERROR writing to socket");
        
        // Read Response
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        printf("Response: %s\n",buffer);
        if (n < 0) 
            error("ERROR reading from socket");

        ResponseMessage resp = decode_response(buffer, req.Request_type);
        if(resp.Response_type == 0){
            // Status OK
            if(req.Request_type == 0){
                printf("Item: %s, Price: %d\n", resp.name, resp.price);
            }
            else if(req.Request_type == 1){
                printf("Total Price: %d\n", resp.total_amount);
            }
        }
        else{
            // ERROR
            printf("Error: %s\n", resp.error);
        }
        // Closing Request, Break out
        if(req.Request_type == 1) break;
    }
    close(sockfd);
    return 0;
}
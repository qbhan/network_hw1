//
//  main.c
//  server
//
//  Created by 한규범 on 15/09/2019.
//  Copyright © 2019 Q. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
//#include <sys/types.h>

typedef struct protocol protocol;
struct protocol {
    uint16_t    op;
    uint16_t    checksum;
    uint32_t    keyword;
    uint64_t    length;
    char        *data;
};

int getsocketfd(char *port) {
    int socket_fd;
    struct sockaddr_in sa;
    
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("FAILED TO CREATE SOCKET\n");
        exit(1);
    }
    printf("SUCCESSFULLY CREATED SOCKET\n");
    
    memset((char *)&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(atoi(port));
    
    if (bind(socket_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        exit(1);
    }
    listen(socket_fd, 5);
    return socket_fd;
}


int main(int argc, char * argv[]) {
    // insert code here...
    char *port;
    char recv_buff[2048];
    char send_buff[2048];
    int sockfd, new_fd, pid;
    struct sockaddr_storage their_sa;
    socklen_t addr_size;
    
    port = argv[2];
    
    sockfd = getsocketfd(port);
    addr_size = sizeof(struct sockaddr_storage);
    
    while(1){
        new_fd = accept(sockfd, (struct sockaddr*)&their_sa, &addr_size);
        if (new_fd > 0){
            perror("ACCEPTION ERROR");
            exit(1);
        }
        pid = fork();
        if (pid < 0){
            perror("NEW PROCESS CREATION ERROR");
            exit(1);
        }
        if (pid == 0){
            close(sockfd);
            printf("CHILD PROCESS\n");
            if (recv(new_fd, recv_buff, 2048, 0) < 0){
                perror("ERROR WHEN RECEIVING\n");
                return 0;
            }
            printf("Server got :%s\n", recv_buff);
            strcpy(send_buff, "Server Successfully Received");
            send(new_fd, send_buff, 2048, 0);
            
            close(new_fd);
        } else {
            printf("PARENT PROCESS\n");
            close(new_fd);
        }
    }
    
//    printf("Hello, World!\n");
    return 0;
}

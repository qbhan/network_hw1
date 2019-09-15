//
//  main.c
//  client
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

typedef struct protocol protocol;
struct protocol {
    uint16_t    op;
    uint16_t    checksum;
    uint32_t    keyword;
    uint64_t    length;
    char        *data;
};

int getclientfd(char* host, char* port){
    int client_fd;
    struct sockaddr_in sa;
    
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("FAILED TO CREATE SOCKET\n");
        return -1;
    }
    printf("SUCCESSFULLY CREATED SOCKET\n");
    
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, host, &sa.sin_addr);
    sa.sin_port = htons(atoi(port));
    
    if (connect(client_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        printf("FAILED SOCKET CONNECTION\n");
        return -1;
    }
    printf("SUCCESSFULLY CONNECTED SOCKET\n");
    
    return client_fd;
}

protocol getprotocol(int op, int checksum, char *keyword, char *length, char *data){
    protocol prtcl = {(uint16_t)op, (uint16_t)checksum, (uint32_t)keyword, (uint64_t)length, data};
    return prtcl;
}

int main(int argc, char *argv[]) {
    // insert code here...
    char *host;
    char *port;
    char *keyword;
    char buff[2048];
    int op, client_fd;
    
//    if (argc == 8){
//        if (strcmp(argv[1], "-h") == 0){
//            host = argv[2];
//        } else {
//            printf("Error parameter\n");
//        }
//        if (strcmp(argv[3], "-p") == 0){
//            port = argv[4];
//        } else {
//            printf("Error parameter\n");
//        }
//        if (strcmp(argv[5], "-o") == 0){
//            op = atoi(argv[6]);
//        } else {
//            printf("Error parameter\n");
//        }
//        if (strcmp(argv[7], "-o") == 0){
//            keyword = argv[8];
//        } else {
//            printf("Error parameter\n");
//        }
//    }
    
    host = argv[2];
    port = argv[4];
    op = atoi(argv[6]);
    keyword = argv[8];
    
    client_fd = getclientfd(host, port);
    if (client_fd == -1){
        printf("FAILED CONNECTION\n");
        exit(1);
    }
    
    fgets(buff, 2048, stdin);
    printf("%s\n", buff);
    
    printf("Hello, World!\n");
    return 0;
}

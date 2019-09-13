//
//  server_example.c
//  Server
//
//  Created by 한규범 on 13/09/2019.
//  Copyright © 2019 Q. All rights reserved.
//

#include "server_example.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <fcntl.h>

int open_listenfd(char *port)
{
    int listenfd;
    struct sockaddr_in addr;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        exit(1);
    }
    bzero((char *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(port));
    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        exit(1);
    }
    listen(listenfd, 5);
    return listenfd;
}

void download(int SCK, char* filename){
    int totBufNum, sendBytes;
    long size;
    char buf[1024];
    char path[1024] = "./Server_files/";
    char dest[1024];
    int status = 0;
    FILE *fptr;
    sprintf(dest, "%s%s", path, filename);
    fptr = fopen(dest, "rb");
    if (fptr == NULL)
    {
        status = 1;
        send(SCK, &status, sizeof(int), 0);
    }else
    {
        status = 0;
        send(SCK, &status, sizeof(int), 0);
        fseek(fptr, 0, SEEK_END);
        size = ftell(fptr);
        totBufNum = size / sizeof(buf) + 1;
        fseek(fptr, 0, SEEK_SET);
        sprintf(buf, "%ld", size);
        sendBytes = send(SCK, buf, sizeof(buf), 0);
        while ((sendBytes = fread(buf, sizeof(char), sizeof(buf), fptr))>0) {
            send(SCK, buf, sendBytes, 0);
        }
        fclose(fptr);
        printf("File sent: %s\n", dest);
    }
}


void upload(int SCK, char* filename){
    FILE *fptr;
    char dest[1024];
    sprintf(dest, "./Server_files/%s", filename);
    fptr = fopen(dest, "wb");
    int readBytes, totBufNum, BufNum;
    long size;
    long totReadBytes;
    char buf[1024];
    readBytes = recv(SCK, buf, sizeof(buf), 0);
    size = atoi(buf);
    totBufNum = size / 1024 + 1;
    BufNum = 0;
    while (BufNum != totBufNum){
        readBytes = recv(SCK, buf, sizeof(buf), 0);
        BufNum++;
        totReadBytes += readBytes;
        fwrite(buf, sizeof(char), readBytes, fptr);
    }
    fclose(fptr);
    printf("File received: %s\n", dest);
}

int main(int argc, char* argv[]) {
    char *port;
    int listenfd, connfd, fileh, size;
    char buf[1024], command[10], filename[1024];
    char *filed;
    char *path = "./Server_files/";
    char target_dir[1024];
    struct stat target;
    
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    printf("SUPER SERVER INTIALIZING...\n");
    port = argv[2];
    printf("Target port : %s \n", port);
    printf("Listening connection...\n");
    listenfd = open_listenfd(port);
    printf("Listening socket status : %d\n", listenfd);
    clientlen = sizeof(struct sockaddr_storage);
    connfd = accept(listenfd, (struct sockaddr*) &clientaddr, &clientlen);
    printf("Connection socket : %d\n", listenfd);
    printf("Waiting for request...\n");
    while(1){
        recv(connfd, buf, 1024, 0);
        sscanf(buf, "%s %s", command, filename);
        printf("[Request] %s %s\n", command, filename);
        if (strcmp(command, "dl") == 0)
        {
            printf("Download : %s \n", filename);
            download(connfd, filename);
        }else if(strcmp(command, "up") == 0){
            printf("Upload : %s \n", filename);
            upload(connfd, filename);
        }
    }
    return 0;
}

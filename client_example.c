//
//  client_example.c
//  Server
//
//  Created by 한규범 on 13/09/2019.
//  Copyright © 2019 Q. All rights reserved.
//

#include "client_example.h"
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

int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct sockaddr_in addr;
    printf("%s %s\n", hostname, port);
    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, hostname, &addr.sin_addr);
    addr.sin_port = htons(atoi(port));
    if (connect(clientfd, addr, sizeof(addr)) < 0)
    {
        return -1;
    }
    return clientfd;
}

/*
 * DESCRIPTION: List files in "client_files" directory
 *
 * PARAMETERS: None
 * RETURNS: None
 */
void list() {
    DIR *direc = NULL;
    struct dirent *file = NULL;
    char *file_directory = "./Client_files";
    if((direc = opendir(file_directory)) == NULL)
    {
        perror("Directory isn't exist");
        printf("s");
    }
    while((file = readdir(direc)) != NULL)
    {
        if (strcmp(file->d_name, ".") == 0)
        {
            continue;
        }else if (strcmp(file->d_name, "..") == 0)
        {
            continue;
        }
        printf("%s ", file->d_name);
    }
    printf("\n");
    closedir(direc);
}

/*
 * DESCRIPTION: Upload the file with FILENAME to the server
 *
 * PARAMETERS:
 *  - int socket:
 *  - char* filename: name of the file to upload (in client_file directory)
 *
 * RETURNS: None
 *
 * NOTES: If no such file exists, then print "Error: no such file" in stdout
 */
void upload(int socket, char* filename) {
    int totBufNum, sendBytes;
    long size;
    char buf[1024];
    char path[1024] = "./Client_files/";
    char dest[1024];
    sprintf(dest, "%s%s", path, filename);
    FILE *fptr;
    fptr = fopen(dest, "rb");
    printf("[Upload] Checking File Existence...\n");
    if (fptr == NULL)
    {
        printf("Error: no such file\n");
    }else
    {
        char comm[1024];
        sprintf(comm, "up %s", filename);
        send(socket, comm, sizeof(comm), 0);
        printf("[Upload] Command Sent\n");
        fseek(fptr, 0, SEEK_END);
        size = ftell(fptr);
        printf("[Upload] File Size: %ld\n", size);
        totBufNum = size / sizeof(buf) + 1;
        fseek(fptr, 0, SEEK_SET);
        sprintf(buf, "%ld", size);
        sendBytes = send(socket, buf, sizeof(buf), 0);
        while ((sendBytes = fread(buf, sizeof(char), sizeof(buf), fptr))>0) {
            send(socket, buf, sendBytes, 0);
        }
        fclose(fptr);
        printf("[Upload] Successful.\n");
    }
}


/*
 * DESCRIPTION: Download the file with FILENAME from the server
 *
 * PARAMETERS:
 *  - int socket:
 *  - char* filename: name of the file to download (in server_file directory)
 *
 * RETURNS: None
 *
 * NOTES: If no such file exists, then print "Error: no such file" in stdout
 */
void download(int socket, char* filename) {
    FILE *fptr;
    char dest[1024];
    int exist_num = 0;
    int status = 0;
    char comm[1024];
    sprintf(comm, "dl %s", filename);
    send(socket, comm, sizeof(comm), 0);
    printf("[Download] Command Sent\n");
    recv(socket, &status, sizeof(int), 0);
    if (status){
        printf("Error: no such file\n");
    }else{
        sprintf(dest, "./Client_files/%s", filename);
        printf("[Download] Checking existence and replacing file name...\n");
        while((fptr = fopen(dest, "r")) != NULL)
        {
            char parser_name[1024], a[1024], b[1024];
            sscanf(filename, "%[^'.'].%s", a, b);
            exist_num++;
            sprintf(dest, "./Client_files/%s(%d).%s", a, exist_num, b);
            fclose(fptr);
        }
        printf("[Donwload] Directory : %s \n", dest);
        fptr = fopen(dest, "wb");
        int readBytes, totBufNum, BufNum;
        long size;
        char buf[1024];
        readBytes = recv(socket, buf, sizeof(buf), 0);
        size = atoi(buf);
        printf("[Download] File Size: %ld\n", size);
        totBufNum = size / 1024 + 1;
        BufNum = 0;
        while (BufNum != totBufNum){
            readBytes = recv(socket, buf, sizeof(buf), 0);
            BufNum++;
            fwrite(buf, sizeof(char), readBytes, fptr);
        }
        fclose(fptr);
        printf("[Download] Successful.\n");
    }
}

int main(int argc, char* argv[]) {
    int clientfd;
    char *host, *port, buf[1024];
    char command[1024];
    host = argv[2];
    port = argv[4];
    clientfd = open_clientfds(host, port);
    if (clientfd == -1){
        printf("Connection Failed.\n");
        exit(1);
    }
    while(1){
        printf("[20180676]> ");
        bzero(command, sizeof(command));
        fgets(command, 1024, stdin);
        if ( !strncmp(command, "download", 8) )
        {
            char *param = NULL;
            char param2[1024] = "";
            bzero(param2, sizeof(param2));
            param = strtok(command, " ");
            param = strtok(NULL, " ");
            strncpy(param2, param, strlen(param)-1);
            download(clientfd, param2);
        }else if( !strncmp(command, "upload", 6) )
        {
            char *param = NULL;
            char param2[1024] = "";
            bzero(param2, sizeof(param2));
            param = strtok(command, " ");
            param = strtok(NULL, " ");
            strncpy(param2, param, strlen(param)-1);
            upload(clientfd, param2);
        }else if(!strncmp(command, "ls", 2))
        {
            list();
        }else if(!strncmp(command, "exit", 4))
        {
            exit(1);
        }
    }
    return 0;
}

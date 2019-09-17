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
#include <netdb.h>
#include <unistd.h>
#include <ifaddrs.h>
#include "Header.h"

typedef struct protocol protocol;
typedef struct pseudo_header pseudo_header;
typedef struct protocol_header protocol_header;

struct protocol {
    unsigned short      op;
    unsigned short      checksum;
    unsigned char       keyword[4];
    unsigned long long   length;
    char        data[];
};

struct pseudo_header{
    unsigned int source_address;
    unsigned int dest_address;
    unsigned char placeholder;      // 0
    unsigned char protocol;         // 6
    unsigned short tcp_length;      //
};

/* 출처: https://kaspyx.tistory.com/61 [잘지내나요, 내청춘] */

struct protocol_header{
    uint16_t    op;
    uint16_t    checksum;
    uint32_t    keyword;
    uint64_t    length;
//    char        *data;
};

unsigned short in_checksum(unsigned short *ptr,int nbytes) {
    register long long sum;
    unsigned short oddbyte;
    register short answer=0;
    
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((unsigned char*)&oddbyte)=*(unsigned char*)ptr;
        sum+=oddbyte;
    }
    
    while(sum>>16 > 0) {
        sum = (sum>>16)+(sum & 0xffff);
//        sum = sum + (sum>>16);
        answer=(short)~sum;
    }
    
    return(answer);
}
/* 출처: https://kaspyx.tistory.com/61 [잘지내나요, 내청춘] */




int getclientfd(char* host, char* port){
    int client_fd;
    struct sockaddr_in sa;
//    struct hostent *server;
    
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("FAILED TO CREATE SOCKET\n");
        return -1;
    }
    printf("SOCKET SUCCESSFULLY CREATED\n");
//    server->h_name = gethostbyname((const char *)host);
//    bzero((char *)&sa, sizeof(sa));
//    sa.sin_family = AF_INET;
//    bcopy((char *)sa->h_addr, (char *)&sa.sin_addr.s_addr, server->h_name, );
    
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, host, &sa.sin_addr);
    sa.sin_port = htons(atoi(port));
    
    if (connect(client_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        printf("SOCKET CONNECTION FAILED\n");
        exit(1);
    }
    printf("SOCKET SUCCESSFULLY CONNECTED\n");
    
    return client_fd;
}


//unsigned short getchecksum(unsigned short *data, int bytes){
//    signed short checksum;
//
//
//    return checksum;
//}

int main(int argc, char *argv[]) {
    // insert code here...
    char *host;
    char *port;
    char *keyword;
    char buff[2048];                //Get data to encript or decript
    int op, client_fd, proc_len;
    
    
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
//    strncpy(keyword, argv[8], 4);
    printf("input host: %s\n", host);
    printf("input port: %s\n", port);
    printf("input op: %d\n", op);
    printf("input keyword: %s\n", keyword);
    client_fd = getclientfd(host, port);
    if (client_fd == -1){
        printf("FAILED CONNECTION\n");
        exit(1);
    }
//    memcpy(ptr + 16, string, strlen(string))
    fgets(buff, 2048, stdin);
//    protocol_len = 16 + (int)strlen(buff);
//    printf("length = %d\n", protocol_len);
//    printf("length = %d\n", (int)strlen(buff));
//    printf("%s\n", buff);
//    buff_len = (int)strlen(buff);
//    send(client_fd, (void *)buff, buff_len, 0)
    protocol *proc = (protocol *) malloc(sizeof(protocol) + strlen(buff));
    proc_len = htobe64(sizeof(proc->op)) + htobe64(sizeof(proc->checksum)) + htobe64(sizeof(proc->keyword)) + htobe64(sizeof(proc->length)) + 1 + strlen(buff);
    proc->op = htons(op);
    proc->checksum = 0x0000;
//    printf("here?\n");
    strncpy(proc->keyword, keyword, 4);
//    printf("there?\n");
//    strncpy(proc+8, keyword, 4);
    proc->length = proc_len;
//    strcpy(proc->data, buff);
    strcpy(proc->data, buff);
    
    struct pseudo_header *psh;
    struct protocol_header *myth;
    struct ifaddrs *id;                     //영철이 참조
    getifaddrs(&id);              //자기자신 주소 헤헤
    
    
    psh = (pseudo_header*) malloc(sizeof(pseudo_header));
    myth = (protocol_header*) malloc(sizeof(protocol_header));
    
    psh->source_address = inet_addr((const char *)id->ifa_addr);
    psh->dest_address = inet_addr(host);
    psh->placeholder = 0;
    psh->protocol = 6;
    psh->tcp_length = htobe64(sizeof(proc->op)) + htobe64(sizeof(proc->checksum)) + htobe64(sizeof(proc->keyword)) + htobe64(sizeof(proc->length));;
    
    myth->op = htons(op);
    myth->checksum = 0x0000;
    myth->keyword = (unsigned int) keyword;
    myth->length = htobe64(sizeof(proc->op)) + htobe64(sizeof(proc->checksum)) + htobe64(sizeof(proc->keyword)) + htobe64(sizeof(proc->length));
    
    
    unsigned char *seudo;
    unsigned int protocol_data_size;
    protocol_data_size = sizeof(pseudo_header)+ sizeof(protocol);
    seudo = (unsigned char *)malloc(protocol_data_size);
    memcpy(seudo, psh, sizeof(pseudo_header));
    memcpy(seudo+sizeof(pseudo_header), myth, sizeof(protocol));
    
    
//    출처: https://kaspyx.tistory.com/61 [잘지내나요, 내청춘]
    
    proc->checksum = in_checksum((unsigned short*)seudo, protocol_data_size);
    printf("checksum is :%d\n", proc->checksum);
    
//    printf()
    if (send(client_fd, proc, proc_len, 0) < 0){
        perror("ERROR WHEN SENDING\n");
        return 0;
    }
    // Send simple string
    if (send(client_fd, buff, proc_len, 0) < 0){
        perror("ERROR WHEN SENDING\n");
        return 0;
    }
    printf("op in protocol: %hu\n", proc->op);
    printf("checksum in protocol: %hu\n", proc->checksum);
    printf("keyword in protocol: %s\n", proc->keyword);
    printf("length in protocol: %llu\n", proc->length);
    printf("data in protocol: %s\n", proc->data);
//    printf("size of protocol: %d\n", sizeof(proc));
    
    
//    protocol *recv_proc = malloc(sizeof(protocol) + strlen(buff));
//    printf("HERE\n");
//    if (recv(client_fd, recv_proc, proc_len, 0) < 0){
//        perror("ERROR WHEN RECEIVING\n");
//        return 0;
//    }
//    printf("size is %d\n", size);
//    printf("%d\n", (int)recv_proc);
//    printf("%d\n", (int)recv_proc->checksum);
//    printf("%d\n", (int)recv_proc->op);
//    printf("%d\n", (int)recv_proc->);
//    printf("%d\n", (int)recv_proc->checksum);
//    printf("%s\n", recv_proc->data);
    
    //Receive simple String
    char recv_buff[2048];
    if (recv(client_fd, recv_buff, 2048, 0) < 0){
        perror("ERROR WHEN RECEIVING\n");
        return 0;
    }
    
    printf("Client received :%s\n", recv_buff);
    
    close(client_fd);
    printf("\n");
    return 0;
}


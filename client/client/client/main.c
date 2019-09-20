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

//struct pseudo_header{
//    unsigned int source_address;
//    unsigned int dest_address;
//    unsigned char placeholder;      // 0
//    unsigned char protocol;         // 6
//    unsigned short tcp_length;      //
//};

/* 출처: https://kaspyx.tistory.com/61 [잘지내나요, 내청춘] */

struct protocol_header{
    unsigned short          op;
    unsigned short          checksum;
    unsigned char           keyword;
    unsigned long long      length;
//    char        *data;
};

unsigned short in_checksum(unsigned short *ptr,long long nbytes) {
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
    }
    
    answer=(short)~sum;
    
    return answer;
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
//    printf("SOCKET SUCCESSFULLY CREATED\n");
//    server->h_name = gethostbyname((const char *)host);
//    bzero((char *)&sa, sizeof(sa));
//    sa.sin_family = AF_INET;
//    bcopy((char *)sa->h_addr, (char *)&sa.sin_addr.s_addr, server->h_name, );
    
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, host, &sa.sin_addr);
    sa.sin_port = htons(atoi(port));
    
    if (connect(client_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        printf("SOCKET CONNECTION FAILED\n");
        return -1;
    }
//    printf("SOCKET SUCCESSFULLY CONNECTED\n");
    
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
    char ch = 0;
    char *host;
    char *port;
    char *keyword;
    char buff[999984];                //Get data to encript or decript
    int op, client_fd, buff_cnt;
    long long proc_len;
    
    if (argc != 9){
        printf("Error parameter\n");
        return -1;
    }
    
    if (strcmp(argv[1], "-h") == 0){
        host = argv[2];
    } else {
        printf("Error parameter\n");
        return -1;
    }
    if (strcmp(argv[3], "-p") == 0){
        port = argv[4];
    } else {
        printf("Error parameter\n");
        return -1;
    }
    if (strcmp(argv[5], "-o") == 0){
        op = atoi(argv[6]);
    } else {
        printf("Error parameter\n");
        return -1;
    }
    if (strcmp(argv[7], "-k") == 0){
        keyword = argv[8];
    } else {
        printf("Error parameter\n");
        return -1;
    }
    
    host = argv[2];
    port = argv[4];
    op = atoi(argv[6]);
    keyword = argv[8];
//    strcpy(keyword, argv[8]);
//    printf("input host: %s\n", host);
//    printf("input port: %s\n", port);
//    printf("input op: %d\n", op);
//    printf("input keyword: %s\n", keyword);
//    printf("input keyword: %s\n", argv[8]);
    client_fd = getclientfd(host, port);
    if (client_fd == -1){
        printf("FAILED CONNECTION\n");
        exit(1);
    }
    
    buff_cnt = 0;
    while((buff_cnt < 999984)&&(ch != EOF)){
        ch = getchar();
        buff[buff_cnt] = ch;
        buff_cnt++;
    }
    buff[buff_cnt] = EOF;
//    printf("EOF is : %c\n", EOF);
    
    
    
//    fgets(buff, 999984, stdin);
//    printf("Buffer has :%s\n", buff);
    
//    protocol_len = 16 + (int)strlen(buff);
//    printf("length = %d\n", protocol_len);
//    printf("length = %d\n", (int)strlen(buff));
    ////    printf("%s\n", buff);
////    buff_len = (int)strlen(buff);
////    send(client_fd, (void *)buff, buff_len, 0)
    protocol *proc = (protocol *) malloc(sizeof(protocol) + strlen(buff) + 1);
//    proc_len = htobe64(sizeof(proc->op)) + htobe64(sizeof(proc->checksum)) + htobe64(sizeof(proc->keyword)) + htobe64(sizeof(proc->length)) + htobe64(1 + strlen(buff));
//    proc_len = sizeof(proc->op) + sizeof(proc->checksum) + sizeof(proc->keyword) + sizeof(proc->length) + 1 + strlen(buff);
    proc_len = sizeof(proc->op) + sizeof(proc->checksum) + sizeof(proc->keyword) + sizeof(proc->length) + strlen(buff) + 1;
    proc->op = htons(op);
    proc->checksum = 0;
////    printf("here?\n");
    strncpy((char *)proc->keyword, keyword, 4);
////    printf("there?\n");
////    strncpy(proc+8, keyword, 4);
    proc->length = htobe64(proc_len);
    strncpy(proc->data, buff, strlen(buff));
    
//    struct pseudo_header *psh;
//    struct protocol_header *myth;
//    struct ifaddrs *id;                     //영철이 참조
//    getifaddrs(&id);              //자기자신 주소 헤헤
////
////
//    psh = (pseudo_header*) malloc(sizeof(pseudo_header));
//    myth = (protocol_header*) malloc(sizeof(protocol_header));
////
//    psh->source_address = inet_addr((const char *)id->ifa_addr);
//    psh->dest_address = inet_addr(host);
//    psh->placeholder = 0;
//    psh->protocol = 6;
//    psh->tcp_length = htons(20);
//
//    myth->op = htons(op);
//    myth->checksum = 0x0000;
//    myth->keyword = (unsigned int) keyword;
//    myth->length = htobe64(sizeof(proc->op)) + htobe64(sizeof(proc->checksum)) + htobe64(sizeof(proc->keyword)) + htobe64(sizeof(proc->length));
//    myth->length = htobe64(proc_len);
//    myth->length = htobe64(16);
//
//
//    unsigned char *seudo;
//    unsigned int protocol_data_size;
////    protocol_data_size = sizeof(pseudo_header) + sizeof(protocol);
//    protocol_data_size = sizeof(pseudo_header) + proc_len;
//    seudo = (unsigned char *)malloc(protocol_data_size);
//    memcpy(seudo, psh, sizeof(pseudo_header));
////    memcpy(seudo+sizeof(pseudo_header), proc, sizeof(protocol));
//    memcpy(seudo+sizeof(pseudo_header), proc, proc_len);
//
//
////    출처: https://kaspyx.tistory.com/61 [잘지내나요, 내청춘]
//
    proc->checksum = in_checksum((unsigned short*)proc, proc_len);
//    proc->checksum = in_checksum((unsigned short*)myth, proc_len);
//    printf("checksum is :%d\n", proc->checksum);
//
//    printf("op in protocol: %hu\n", ntohs(proc->op));
//    printf("addr of op in protocol: %d\n", &proc->op);
//    printf("checksum in protocol: %hu\n", proc->checksum);
//    printf("addr of checksum in protocol: %d\n", &proc->checksum);
//    printf("keyword in protocol: %s\n", proc->keyword);
//    printf("addr of keyword in protocol: %d\n", &proc->keyword);
//    printf("length in protocol: %llu\n", be64toh(proc->length));
//    printf("addr of length in protocol: %d\n", &proc->length);
//    printf("data in protocol\n%s\n", proc->data);
//    printf("send length: %lu\n", strlen(proc->data));
//    printf("addr of data in protocol: %d\n", &proc->data);
//    printf("size of protocol: %ld\n", sizeof(proc));
    
    if (send(client_fd, proc, proc_len, 0) < 0){
        perror("ERROR WHEN SENDING\n");
        return 0;
    }
    
//     send simple string
//    if (send(client_fd, buff, 2048, 0) < 0){
//        perror("ERROR WHEN SENDING\n");
//        return 0;
//    }
    
//    printf("size of recv_proc: %lu\n", sizeof(protocol) + strlen(buff)+1);
    protocol *recv_proc = (protocol *)malloc(sizeof(protocol) + strlen(buff)+1);
//    printf("HERE\n");
//    int check_recv;
//    if ((check_recv = recv(client_fd, recv_proc, proc_len, 0)) < 0){
//        perror("ERROR WHEN RECEIVING\n");
//        return 0;
//    }
    int cnt_byte = 0;
    int cnt;
    while (cnt_byte < proc_len){
        cnt = recv(client_fd, recv_proc+cnt_byte, proc_len-cnt_byte, 0);
        if(cnt<=0){
            if (cnt_byte==0){
                break;
            }
        }
        cnt_byte += cnt;
    }
    
//    printf("check_recv =%d\n", check_recv);
//    printf("size is %d\n", size);
//    printf("%d\n", (int)recv_proc);
//    printf("%d\n", (int)recv_proc->checksum);
//    printf("%d\n", (int)recv_proc->op);
//    printf("%d\n", (int)recv_proc->);
//    printf("%d\n", (int)recv_proc->checksum);
//    printf("recv length: %lu\n", strlen(recv_proc->data));
//    printf("%s\n", recv_proc->data);
    printf("%s", (char *)recv_proc+16);
    printf("%s", (char *)recv_proc);
    
    //Receive simple String
//    char recv_buff[2048];
//    if (recv(client_fd, recv_buff, 2048, 0) < 0){
//        perror("ERROR WHEN RECEIVING\n");
//        return 0;
//    }
    
//    printf("Client received :%s\n", recv_buff);
    
    close(client_fd);
//    printf("\n");
    return 0;
}


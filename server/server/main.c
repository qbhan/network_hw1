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
    unsigned short      op;
    unsigned short      checksum;
    unsigned char       keyword[4];
    unsigned long long   length;
    char        data[];
};

///////////////////////////////////////////////////////////
//Change all characters in string to lower cases.
//Need to handle allocation error.
///////////////////////////////////////////////////////////
char* get_lower_string(const char* word){
    int length = (int)strlen(word);
    char* word_Lower;
    word_Lower = (char*)malloc(sizeof(char)*length);
    
    for (int i=0; i<length; i++){
        word_Lower[i] = tolower(word[i]);
    }
    
    return word_Lower;
}

//////////////////////////////////////////////////////////
//Return n_values from corresponding keyword.
//Need to handle allocation error.
//////////////////////////////////////////////////////////
int* get_n_values(const char* keyword){
    const char* keyword_Lower = get_lower_string(keyword);
    int keyword_Length = (int)strlen(keyword_Lower);
    int* n_values;
    n_values = (int*)malloc(sizeof(int)*keyword_Length);
    
    for (int i =0; i<keyword_Length; i++) {
        n_values[i] = (int)keyword_Lower[i]-97;
    }
    
    return n_values;
}

///////////////////////////////////////////////////////////
//Decrypt given text according to keyword.
//Need to handle allocation error.
///////////////////////////////////////////////////////////
char* decrypt(const char* keyword, const char* cyphertext){
    int text_Length = (int)strlen(cyphertext);
    //    printf("%d\n", text_Length);
    
    //    char plaintext[text_Length];
    char* plaintext;
    char* cyphertext_lower = get_lower_string(cyphertext);
    plaintext = strdup(cyphertext_lower);
    
    int* n_values = get_n_values(keyword);
    int keyword_Length = (int)strlen(keyword);
    
    int key_cnt = 0;
    for (int i=0; i<text_Length; i++){
        if (isalpha(plaintext[i])){
            int cnt = key_cnt++ % keyword_Length;
            int ch = (int)plaintext[i] - n_values[cnt];
            if (ch < 97){
                ch = ch + 26;
            }
            plaintext[i] = (char)ch;
        }
    }
    
    return plaintext;
}

char* encript(const char* keyword, const char* plaintext){
    int text_Length = (int)strlen(plaintext);
    //    printf("%d\n", text_Length);
    
    //    char plaintext[text_Length];
    char* cyphertext;
    char* plaintext_lower = get_lower_string(plaintext);
    cyphertext = strdup(plaintext_lower);
    
    int* n_values = get_n_values(keyword);
    int keyword_Length = (int)strlen(keyword);
    
    int key_cnt = 0;
    for (int i=0; i<text_Length; i++){
        if (isalpha(cyphertext[i])){
            
            int cnt = key_cnt++ % keyword_Length;
            int ch = (int)cyphertext[i] + n_values[cnt];
            //        printf("%d\n", ch);
            if (ch > 122){
                ch = ch - 26;
            }
            cyphertext[i] = (char)ch;
        }
    }
    
    return cyphertext;
}

int getsocketfd(char *port) {
    int socket_fd;
    struct sockaddr_in sa;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("FAILED TO CREATE SOCKET\n");
        exit(1);
    }
    printf("SUCCESSFULLY CREATED SOCKET\n");

//    memset((char *)&sa, 0, sizeof(sa));
    bzero((char *)&sa, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(atoi(port));
    //    printf("port is : &d\n", ntohs(sa.sinport));

    if (bind(socket_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        exit(1);
    }
    //    listen(socket_fd, 5);
    return socket_fd;
}

int main(int argc, char * argv[]) {
    // insert code here.......
    char *port;
    protocol *recv_proc = (protocol *) malloc(sizeof(protocol));
//    char recv_buff[2048];
//    char send_buff[2048];
    int sockfd, new_fd, pid;
//    struct sockaddr_in client_sa;
    struct sockaddr_storage client_sa;
    socklen_t client_sa_size;
    struct sockaddr_in sa;
    
    port = argv[2];
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("FAILED TO CREATE SOCKET\n");
        exit(1);
    }
    printf("SUCCESSFULLY CREATED SOCKET\n");
    
    //    memset((char *)&sa, 0, sizeof(sa));
    bzero((char *)&sa, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(atoi(port));
    //    printf("port is : &d\n", ntohs(sa.sinport));
    
    if (bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        exit(1);
    }
    
    client_sa_size = sizeof(client_sa);
//    sockfd = getsocketfd(port);
    if(listen(sockfd, 5) == 0){
        printf("[+]Listening....\n");
    }else{
        printf("[-]Error in binding.\n");
    }
//    addr_size = sizeof(struct sockaddr_storage);
    new_fd = accept(sockfd, (struct sockaddr*)&client_sa, &client_sa_size);
    while(1){
        
//        printf("new_fd is : %d\n", new_fd);
        if (new_fd  < 0){
            perror("ACCEPTION ERROR");
            exit(1);
        }
//        printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_fd , inet_ntoa(client_sa.sin_addr) , ntohs(client_sa.sin_port));
//        printf("Connection socket : %d\n", sockfd);
//        pid = fork();
//        if(pid < 0){
//            printf("ERROR in new process creation");
//            break;
//        }
        
//        if(pid == 0){
//            close(sockfd);
//            sockfd = -1;
////            while(1){
////                printf("Start Receiving...");
//////                int recv_cnt = (int)recv(new_fd, recv_proc, 1024, 0);
////                int recv_cnt = read(new_fd, recv_proc, 999984);
////                if (recv_cnt == -1){
////                    perror("RECEIVE ERROR\n");
////                    break;
////                } else if (recv_cnt == 0){
////                    printf("Recieved nothing\n");
////                    break;
////                }
//////                printf("Recieved: %s\n", recv_proc->data);
////                send(new_fd, recv_proc, strlen(recv_proc), 0);
////                bzero(recv_proc, sizeof(recv_proc));
////            }
//            printf("Start Receiving...");
////                int recv_cnt = (int)recv(new_fd, recv_proc, 1024, 0);
//            int recv_cnt = read(new_fd, recv_proc, 10000000);
//            if (recv_cnt == -1){
//                perror("RECEIVE ERROR\n");
//                break;
//            } else if (recv_cnt == 0){
//                printf("Recieved nothing\n");
//                break;
//            }
////                printf("Recieved: %s\n", recv_proc->data);
//            send(new_fd, recv_proc, strlen(recv_proc), 0);
//            bzero((char *)recv_proc, sizeof(recv_proc));
//            close(new_fd);
//            new_fd = -1;
//            exit(EXIT_SUCCESS);
//        }
//        else {
//            printf("Parent\n");
//            close(new_fd);
//        }
        printf("Start Receiving...");
        //                int recv_cnt = (int)recv(new_fd, recv_proc, 1024, 0);
        int recv_cnt = read(new_fd, recv_proc, 10000000);
        if (recv_cnt == -1){
            perror("RECEIVE ERROR\n");
            break;
        } else if (recv_cnt == 0){
            printf("Recieved nothing\n");
            break;
        }
        //                printf("Recieved: %s\n", recv_proc->data);
        send(new_fd, recv_proc, sizeof(recv_proc), 0);
        bzero((char *)recv_proc, sizeof(recv_proc));
        close(new_fd);
        new_fd = -1;
    }

    
    // 일단은 그냥 해보자
    //    while(1){
    //        new_fd = accept(sockfd, (struct sockaddr*)&their_sa, &addr_size);
    ////        printf("new_fd is : %d\n", new_fd);
    //        if (new_fd  < 0){
    //            perror("ACCEPTION ERROR");
    //            exit(1);
    //        }
    //        printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_fd , inet_ntoa(their_sa.sin_addr) , ntohs(their_sa.sin_port));
    //
    //        printf("Start Receiving...");
    //        int recv_cnt = (int)recv(new_fd, recv_proc, 1024, 0);
    //        if (recv_cnt == -1){
    //            perror("RECEIVE ERROR\n");
    //            break;
    //        } else if (recv_cnt == 0){
    //            printf("Recieved nothing\n");
    //            break;
    //        }
    ////                printf("Recieved: %s\n", recv_proc->data);
    //        send(new_fd, recv_proc, strlen(recv_proc), 0);
    //        bzero(recv_proc, sizeof(recv_proc));
    //
    //        close(new_fd);
    //    }
    
    return 0;
}

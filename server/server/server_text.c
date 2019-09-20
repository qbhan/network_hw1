
/*
 ** server.c -- a stream socket server demo
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
//#define PORT "1234"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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
char* get_lower_string(char* word){
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
int* get_n_values(char* keyword){
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
char* decrypt(char* keyword, char* cyphertext){
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

///////////////////////////////////////////////////////////
//Encrypt given text according to keyword.
//Need to handle allocation error.
///////////////////////////////////////////////////////////
char* encript(char* keyword, char* plaintext){
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

int main(int argc, char * argv[])
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char *port = argv[2];
    char recv_buff[1000000];
    char keyword[4];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue; }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1); }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue; }
        break; }
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1); }
    printf("server: waiting for connections...\n");
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue; }
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            protocol *recv_proc = (protocol *) malloc(1000000);
            if(recv(new_fd, recv_proc, 1000000, 0) == -1){
                perror("receive");
            };
            printf("Data received\n%s\n", recv_proc->data);
//            strcpy(recv_buff, recv_proc->data);
//            strcpy(keyword, recv_proc->keyword);
            if(recv_proc->op == 0){
                strcpy(recv_buff, encript((char *)recv_proc->keyword, recv_proc->data));
//                printf("Encript\n%s\n",encript((char *)recv_proc->keyword, recv_proc->data));
                printf("Encript\n%s\n",recv_buff);
            } else if(recv_proc->op == 1){
                strcpy(recv_buff, decrypt((char *)recv_proc->keyword, recv_proc->data));
                printf("Decript\n%s\n",recv_buff);
            }
            strcpy(recv_proc->data, recv_buff);
            printf("recv_proc data\n%s\n",recv_proc->data);
            
            if (send(new_fd, recv_proc, 1000000, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }
    return 0; }

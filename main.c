//
//  main.c
//  Server
//
//  Created by 한규범 on 12/09/2019.
//  Copyright © 2019 Q. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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
    plaintext = strdup(cyphertext);
    
    int* n_values = get_n_values(keyword);
    int keyword_Length = (int)strlen(keyword);
    
    for (int i=0; i<text_Length; i++){
        int cnt = i % keyword_Length;
        int ch = (int)plaintext[i] - n_values[cnt];
        if (ch < 97){
            ch = ch + 26;
        }
        plaintext[i] = (char)ch;
    }

    return plaintext;
}

///////////////////////////////////////////////////////////
//Encrypt given text according to keyword.
//Need to handle allocation error.
///////////////////////////////////////////////////////////
char* encript(const char* keyword, const char* plaintext){
    int text_Length = (int)strlen(plaintext);
//    printf("%d\n", text_Length);
    
    //    char plaintext[text_Length];
    char* cyphertext;
    cyphertext = strdup(plaintext);
    
    int* n_values = get_n_values(keyword);
    int keyword_Length = (int)strlen(keyword);
    
    for (int i=0; i<text_Length; i++){
        int cnt = i % keyword_Length;
        int ch = (int)cyphertext[i] + n_values[cnt];
//        printf("%d\n", ch);
        if (ch > 122){
            ch = ch - 26;
        }
        cyphertext[i] = (char)ch;
    }
    
    return cyphertext;
}



int main(int argc, const char * argv[]) {
    int length;
    char keyword[] = "cake";
    printf("keyword: %s\n", keyword);
    int* n_values = get_n_values(keyword);
    length = (int)strlen(keyword);
    // Print n_values
    printf("[ ");
    for (int i=0; i<length; i++){
        printf("%d ", n_values[i]);
    }
    printf("]\n");
//    printf("%d\n", (int)sizeof("pedaqruw"));
    printf("pedaqruw decrypted: %s\n", decrypt(keyword, "pedaqruw"));
    printf("networks encrypted: %s\n", encript(keyword, "networks"));
    return 0;
    
    
}

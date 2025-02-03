#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "mail.h"


//TO COMPILE:
//    gcc -Wall -o fetchmail main.c mail.c mail.h

//TO RUN:
//    ./fetchmail -u user -p pass [IMAP-compliant server] e.g.unimelb-comp30023-2024.cloud.edu.au -f Test parse
//the user, pass, folder and command don't really matter, just the server (unimelb-comp30023 etc)
//the code attempts to connect to the server on IP address

int main(int argc, char *argv[]){
    int sockfd;

    //read command line in
    commands *comms = read_commands(argc, argv);

    //create socket and connect to the provided server
    sockfd = connect_socket(comms->server_name);
    //printf("%d\n", sockfd);

    if(sockfd == -1){
        free_comms(comms);
        exit(2);
    }

    //number of messages that exist in a folder
    char *exists = (char *)malloc(sizeof(char) * MESSAGELEN);
    int exists_len = 0;
    int *exists_len_pointer = &exists_len;
    //char *exists_string = (char *)malloc(sizeof(char) * MESSAGELEN);

    //int most_recent;

    //communicate with imap//
        //login
    if(login(comms->username, strlen(comms->username), comms->password, strlen(comms->password), sockfd) == -1){
        printf("Login failure\n");
        close(sockfd);
        free_comms(comms);
        free(exists);
        exit(3);
    } else{
        //printf("login successful\n");
    }
        //select folder
    if(select_folder(sockfd, comms->folder, exists, exists_len_pointer) == -1){
        printf("Folder not found\n");
        close(sockfd);
        free_comms(comms);
        free(exists);
        exit(3);
    } else{
        //printf("folder selected successfully\n");
    }
    //printf("There are %s messages in this folder\n", exists);

    if(comms->message_number == NULL){
        comms->message_number = exists;
        if(exists[0] == -1){
            //printf("comms->message_number is now -1\n");
        }
    }

    if(comms->command == PARSE){
        //printf("parse command\n");
        if(giveparse(sockfd, comms->message_number) == -1){
            printf("Message not found\n");
            close(sockfd);
            free_comms(comms);
            free(exists);
            exit(3);
        }
    } else if(comms->command == MIME){
        //printf("mime command\n");

        /*
        if(givemime(sockfd,comms->message_number) == -1){
            printf("Mime failed\n");
            close(sockfd);
            free(comms);
            free(exists);
            exit(4);
        }
        */
    } else if(comms->command == RETRIEVE){
        //printf("retrieve command\n");
        if(giveretrieve(sockfd, comms->message_number) == -1){
            printf("Message not found\n");
            close(sockfd);
            free_comms(comms);
            free(exists);
            exit(3);
        }
    } else if(comms->command == LIST){
        //printf("list command\n");
        if(givelist(sockfd, exists) == -1){
            printf("List failed\n");
            close(sockfd);
            free_comms(comms);
            free(exists);
            exit(3);
        }
    }
    
    //printf("exists is: %s\n", exists);
    close(sockfd);
    free_comms(comms);
    free(exists);
    return 0;
}
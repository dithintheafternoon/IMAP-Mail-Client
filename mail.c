#define _POSIX_C_SOURCE 200112L
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include "mail.h"

//void print_ipv4_addresses(struct addrinfo *res);

char *bound_quotes(char *arg){
    char *new_arg = (char *)malloc(sizeof(char) * (strlen(arg) + QUOTES + 1));
    memset(new_arg, '\0', strlen(arg) + QUOTES);
    new_arg[0] = '\"';
    int i;
    for(i = 0; arg[i] != '\0'; i++){
        new_arg[i+1] = arg[i];
    }
    new_arg[i+1] = '\"';
    new_arg[i+1+1] = '\0';
    return new_arg;
}

void free_comms(commands *comms){
    if(comms->malloced == 1){
        if(comms->user_space == 1){
            free(comms->username);
        }
        if(comms->pass_space == 1){
            free(comms->password);
        }
        if(comms->folder_space == 1){
            free(comms->folder);
        }
    }
    free(comms);
}

//deal with edge cases later
    //need to throw errors when arguments are not the right type
commands *read_commands(int argc, char *argv[]){
    commands *comms = (commands *)malloc(sizeof(commands));

    int last_arg = 0;

    int user = 0;
    int pass = 0;
    int command_received = 0;
    int server = 0;
    int folder = 0;
    int message = 0;

    comms->user_space = 0;
    comms->pass_space = 0;
    comms->folder_space = 0;
    comms->malloced = 0;

    //deal with malformed input first
    if(argc > COMMAND_MAX){
        fprintf(stderr, "Command-line parsing error, argument validation failure\n");
        free(comms);
        exit(1);
    } else if(argc < COMMAND_MIN){
        fprintf(stderr, "Command-line parsing error, argument validation failure\n");
        free(comms);
        exit(1);
    }

    for(int i = 0;i<argc; i++){

        if(i == argc - 1){
            //printf("this is the last command");
            last_arg = 1;
        }

    //mandatory arguments (minus server name)
        if(strcmp(argv[i], "-u") == 0){
            if(last_arg == 1){
                fprintf(stderr, "Command-line parsing error, argument validation failure\n");
                exit(1);
            } else{
                //printf("username: %s detected\n", argv[i+1]);

                if(strstr(argv[i+1], " ") != NULL){
                    comms->username = bound_quotes(argv[i+1]);
                    comms->malloced = 1;
                    comms->user_space = 1;
                } else{
                    comms->username = argv[i+1];
                }
                //printf("    user in comms is now: %s\n",comms->username);
                user = 1;
                i+=1;
            }
        } else if(strcmp(argv[i], "-p") == 0){
            if(last_arg == 1){
                fprintf(stderr, "Command-line parsing error, argument validation failure\n");
                free_comms(comms);
                exit(1);
            }
            else{
                //printf("password: %s detected\n", argv[i+1]);
                if(strstr(argv[i+1], " ") != NULL){
                    comms->password = bound_quotes(argv[i+1]);
                    comms->malloced = 1;
                    comms->pass_space = 1;
                } else{
                    comms->password = argv[i+1];
                }
                //comms->password = argv[i+1];
                //printf("    pass in comms is now: %s\n",comms->password);
                pass = 1;
                i+=1;
            } 
        } 

        //checking for commands
        else if(strcmp(argv[i], "retrieve") == 0){
            comms->command = RETRIEVE;
            command_received = 1;
        } else if(strcmp(argv[i], "parse") == 0){
            comms->command = PARSE;
            command_received = 1;
        } else if(strcmp(argv[i], "mime") == 0){
            comms->command = MIME;
            command_received = 1;
        } else if(strcmp(argv[i], "list") == 0){
            comms->command = LIST;
            command_received = 1;
        }


    //optional arguments
        else if(strcmp(argv[i],"-t") == 0){
            fprintf(stderr, "Warning: TLS not supported. This flag will be ignored\n");
        } else if(strcmp(argv[i], "-f") == 0){
            if(last_arg == 1){
                fprintf(stderr, "Command-line parsing error, argument validation failure\n");
                free_comms(comms);
                exit(1);
            }
            else{
                //printf("folder: %s detected\n", argv[i+1]);
                if(strstr(argv[i+1], " ") != NULL){
                    comms->folder = bound_quotes(argv[i+1]);
                    comms->malloced = 1;
                    comms->folder_space = 1;
                } else{
                    comms->folder = argv[i+1];
                }
                //printf("    folder in comms is now: %s\n",comms->folder);
                folder = 1;
                i+=1;
            }
        } else if(strcmp(argv[i], "-n") == 0){
            if(last_arg == 1){
                fprintf(stderr, "Command-line parsing error, argument validation failure\n");
                free_comms(comms);
                exit(1);
            }
            else{
                //printf("messagenum: %s detected\n", argv[i+1]);
                if(all_digits(argv[i+1]) == 0){
                    fprintf(stderr, "Command-line parsing error, argument validation failure\n");
                    exit(1);
                }
                comms->message_number = argv[i+1];
                //printf("    messnum in comms is now: %s\n",comms->message_number);
                message = 1;
                i+=1;
            }
        }

    //taking whatever's left to be the server name
        else{
            if(strchr(argv[i], '.') == NULL){
                fprintf(stderr, "Warning: server name may be malformed\n");
            }
            comms->server_name = argv[i];
            server = 1;
        }
    }


    //checking if the mandatory args have been provided
    if(user == 0){
        fprintf(stderr, "Error: username not provided\n");
        free_comms(comms);
        exit(1);
    }
    if(pass == 0){
        fprintf(stderr, "Error: password not provided\n");
        free_comms(comms);
        exit(1);
    }
    if(command_received == 0){
        fprintf(stderr, "Error: command not provided or malformed\n");
        free_comms(comms);
        exit(1);
    }
    if(server == 0){
        fprintf(stderr, "Error: server name not provided\n");
        free_comms(comms);
        exit(1);
    } if(message == 0){
        comms->message_number = NULL;
    } if(folder == 0){
        comms->folder = "INBOX";
    }

    return comms;
}


//returns 1 if everything in the string is a digit
int all_digits(char *str){
    int i = 0;
    while(str[i] != '\0'){
        if(isdigit(str[i]) == 0){
            printf("%c detected\n", str[i]);
            return 0;
        }
        i++;
    }
    return 1;
}




//=================================================================================
//==================================================================================
    //SOCKET FUNCTIONS
/*
void print_ipv4_addresses(struct addrinfo *res) {
    struct addrinfo *current;
    char ip[INET_ADDRSTRLEN];

    for (current = res; current != NULL; current = current->ai_next) {
        if (current->ai_family == AF_INET) { // Check for IPv4 address
            struct sockaddr_in *ipv4_addr = (struct sockaddr_in *)current->ai_addr;
            // Convert binary IPv4 address to string format
            inet_ntop(AF_INET, &(ipv4_addr->sin_addr), ip, INET_ADDRSTRLEN);
            printf("IPv4 Address: %s\n", ip);
        }
    }
}
*/

//credit to practical 8 of comp30023
int connect_socket(char *server_name){
    int sockfd, s;
    //int n;
	struct addrinfo hints, *servinfo, *rp;
    char *port = "143";

    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;

    s = getaddrinfo(server_name, port, &hints, &servinfo);
	if (s != 0) {
        hints.ai_family = AF_INET;
        s = getaddrinfo(server_name, port, &hints, &servinfo);
        if(s!=0){
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		    return -1;
        }
	}

    //connect to the first valid result
    for (rp = servinfo; rp != NULL; rp = rp->ai_next) {
	sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (sockfd == -1)
		continue;

	if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1){
        break; // success
    }

	close(sockfd);
	}

	if (rp == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return -1;
	}
    
	freeaddrinfo(servinfo);
    return sockfd;
}

//void write_to_total(char *destination, char *source, int dest_len, int source_len, int dest_buffer_size){
//    if((source_len + dest_len) >= dest_buffer_size){
//        destination = (char *)realloc()
//    }
//}


//==================================================================
    //communication with IMAP

//COMMUNICATING
void write_to_total(char *dest, char *src, int dest_len, int src_len){
    int i = 0;
    while(i <src_len){
        dest[i + dest_len] = src[i];
        i++;
    }

    dest[i+dest_len] = '\0';
}

int check_end(char *src, int src_len, char *tag){
    for(int i = 0;i<src_len;i++){
        //will go through entire message until it find an rn that is attached to a tag
        if((src[i] == '\n') && (src[i-1] == '\r')){
            if(tag == NULL){
                return 1;
            } else{
                char *findtag = strstr(src,tag);
                if(findtag != NULL){
                    if(strstr(findtag, "\r\n") != NULL){
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

char *recv_rn(int soc, char *res, int res_len, char *tag){
    int data_len;

    //total length of the reponse from the server
    int total_len = 0;

    //length of the buffer that stores the response from the server
    int total_res_len = MESSAGELEN + 1; //make is 301 for a 300 byte message + null byte

    //buffer that stores response from the server
    char *total_res = malloc(total_res_len * sizeof(char));
    if(total_res == NULL){
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    while(i < TIMEOUT){
        data_len = recv(soc, res, res_len, 0);


        if(data_len < 0){
            free(total_res);
            return NULL;
        } else{
            if(total_len + data_len >= total_res_len - 1 ){
                total_res_len = total_res_len *2;
                total_res = (char *)realloc(total_res, total_res_len * sizeof(char));
            } 
            write_to_total(total_res, res, total_len, data_len);
            total_len += data_len;
        }

        if(check_end(total_res, total_len, tag) == 1){
            return total_res;   
        }
        i++;
    }
    free(total_res);
    return NULL;
}


//LOGGING IN
int login(char *user, int ulen, char *pass, int plen, int soc){
    char *tag = "A01";
    size_t message_s = MESSAGELEN;

    char *res = malloc(MESSAGELEN * sizeof(char));

    char *message = malloc(message_s * sizeof(char));

    if((ulen + TAGLEN + plen + LOGINLEN) >= (int)message_s){
        message_s = message_s *2;
        message = (char *)realloc(message, message_s * sizeof(char));
    }

    message = (char *)memset(message, 0, message_s * sizeof(char));

    //receive greeting
    char *greetings = recv_rn(soc, res, MESSAGELEN, NULL);
    if(greetings == NULL){
        free(res);
        free(message);
        free(greetings);
        return -1;
    }
    
    sprintf(message, "%s login %s %s\r\n", tag, user, pass);

    if(send(soc, message, ulen + LOGINLEN + plen, 0) < 0){
        free(res);
        free(message);
        free(greetings);
        return -1;
    }

    char *response = recv_rn(soc,res,MESSAGELEN, tag);
    if((strstr(response, "OK") == NULL) && (strstr(response, "ok") == NULL)){
        free(res);
        free(message);
        free(greetings);
        free(response);
        return -1;
    }

    free(res);
    free(message);
    free(greetings);
    free(response);

    return 0;
}

int get_exists(char *response, char *nbuffer, int *exists_len_pointer){
    int bufflen = MESSAGELEN;
    int nbufflen = MESSAGELEN;
    int linelen = 0;
    char *line = (char *)malloc(sizeof(char) * bufflen);

    //int n;
    int nlen = 0;

    for(int i =0;response[i] != '\0';i++){
        if((response[i] == '*') && (response[i+1] == ' ')){
            while((response[i] != '\r') && (response[i+1] != '\n')){
                if(linelen >= (bufflen - 1)){
                    bufflen = bufflen * 2;
                    line = (char *)realloc(line,sizeof(char)*bufflen);
                }
                line[linelen] = response[i];
                linelen++;
                i++;
            }
            line[linelen] = '\0';
            i++;

            if(strstr(line, "EXIST") != NULL){
                for(int i = 2;line[i] != ' ';i++){
                    if(nlen >= (nbufflen - 1)){
                        nbufflen = nbufflen * 2;
                        nbuffer = (char *)realloc(nbuffer, sizeof(char)*nbufflen);
                    }
                    nbuffer[nlen] = line[i];
                    nlen++;
                }
                nbuffer[nlen] = '\0';
                *exists_len_pointer = nlen;
                free(line);
                return 0;
            }
        } else{
            if(strstr(response, "N0") != NULL){
                free(line);
                return -1;
            }
        }
        linelen = 0;
    }

    free(line);
    nbuffer[0] = -1;
    return -1;
}

//SELECTING FOLDER
int select_folder(int soc, char *folder, char *exists, int *exists_len_pointer){
    char *tag = "A02";
    int foldlen = strlen(folder);
    size_t message_s = MESSAGELEN;

    char *res = (char *)malloc(MESSAGELEN * sizeof(char));

    char *message = (char *)malloc(message_s * sizeof(char));

    if((TAGLEN + foldlen + SELFOLDERLEN) >= (int)message_s){
        message_s = message_s *2;
        message = (char *)realloc(message, message_s * sizeof(char));
    }

    message = (char *)memset(message, 0, message_s * sizeof(char));

    sprintf(message, "%s select %s\r\n", tag, folder);

    if(send(soc, message, SELFOLDERLEN + foldlen, 0) < 0){
        free(res);
        free(message);
        return -1;
    }

    char *response = recv_rn(soc,res,MESSAGELEN, tag);
    
    get_exists(response, exists, exists_len_pointer);

    if((strstr(response, "OK") == NULL) && (strstr(response, "ok") == NULL)){
        free(res);
        free(message);
        free(response);
        return -1;
    }

    free(res);
    free(message);
    free(response);
    return 0;
}

int giveretrieve(int soc, char *messagen){
    char *tag ="A03";
    int mnlen = strlen(messagen);
    int mlen = mnlen + RETRIEVELEN;
    char *message = (char *)malloc(sizeof(char) * (mlen + 1));
    char *res = (char *)malloc(sizeof(char) * MESSAGELEN);

    int hlen = 0;
    int *header_len = &hlen;

    sprintf(message, "%s fetch %s BODY.PEEK[]\r\n", tag, messagen);

    if(send(soc, message, mlen, 0) < 0){
        free(res);
        free(message);
        return -1;
    }

    char *response = recv_rn(soc,res,MESSAGELEN, tag);
    
    if(strstr(response, "Invalid messageset") != NULL){
        free(res);
        free(message);
        free(response);
        return -1;
    } else{
        char *rawemail = parseheader(response, header_len, 1);
        //rawemail[*header_len] = '\r';
        //rawemail[*header_len + 1] = '\0';
        printf("%s\n", rawemail);
        free(rawemail);
    }

    free(res);
    free(message);
    free(response);
    return 0;
}

/*
int givereparse(int soc, char *messagen){
    char write_buffer[1024];
    snprintf(write_buffer, 1024, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (FROM)]\r\n",messagen);
    send_msg(soc, write_buffer);
    char read_buffer[1024];
    receive_msg(soc, read_buffer, 1024);
    printf("%s, read_buffer");

    snprintf(write_buffer, 1024, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (TO)]\r\n",messagen);
    send_msg(soc, write_buffer);
    receive_msg(soc, read_buffer, 1024);
    printf("%s, read_buffer");

    snprintf(write_buffer, 1024, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (DATE)]\r\n",messagen);
    send_msg(soc, write_buffer);
    receive_msg(soc, read_buffer, 1024);
    printf("%s, read_buffer");

    snprintf(write_buffer, 1024, "A03 FETCH %s BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n",messagen);
    send_msg(soc, write_buffer);
    receive_msg(soc, read_buffer, 1024);
    printf("%s, read_buffer");
}
*/

char *parseheader(char *response, int *header_len, int retrieve){
    //printf("entered parseheader\n");
    int i = 0;
    int y = 0;
    int start_bit = 0;
    int start_header = 0;
    int hbuff_len = MESSAGELEN;
    int hhbuff_len = MESSAGELEN;
    char *header = (char *)malloc(sizeof(char) * hbuff_len);
    char *mes_len = (char *)malloc(sizeof(char) * hhbuff_len);
    memset(mes_len, '\0', hhbuff_len);

    while(1){
        //finding the irst CRLF to get nbits
        if(response[i] == '{'){
            start_bit = (i + 1);
        } else if((response[i] == '\r') && (response[i+1] == '\n')){

            //no subject
            if((response[i-1] == '}') && (response[i-2] == '2') && (response[i-3] == '{')){
                free(header);
                free(mes_len);
                return NULL;
            } else{
                //find the number of bits in the message
                for(y = start_bit; y < (i-1); y++){
                    mes_len[y - start_bit] = response[y];
                }
                mes_len[y] = '\0';

                //skip over \r\n
                i+=2;
                start_header = i;

                //now we loop to get the entire header and fold if needed
                while((i - start_header + 1) < (atoi(mes_len) - ROOMFORNULLBYTE)){

                    //realloc if necessary
                    if((*header_len) >= (hbuff_len - ROOMFORNULLBYTE)){
                        hbuff_len = hbuff_len * DOUBLE;
                        header = (char *)realloc(header, hbuff_len);
                    }
                    
                    //if we have reached \r\n
                    if((response[i] == '\r') && (response[i+1] == '\n')){

                        //but we are not at the end of the message
                        if((i - start_header +2) != (atoi(mes_len) - ROOMFORNULLBYTE)){
                            if(retrieve == 0){
                                //don't unfold if this is being used for 
                                    //the raw email in retrieve
                                i+=2;
                            }
                        }
                    }
                    header[*header_len] = response[i];
                    (*header_len)++;
                    
                    i++;
                }
                if(retrieve == 1){
                    //printf("this is for retrieve command\n");
                    header[*header_len] = response[i];
                    //printf("%c\n", response[i]);
                    header[*header_len + 1] = response[i+1];
                    //printf("%c\n", response[i+1]);
                    (*header_len)+=2;
                }
                header[*header_len] = '\0';
                
                //printf("header is:%s\n", header);
                free(mes_len);
                return header;
            }
        }
        i++;
    }
    free(header);
    free(mes_len);
    *header_len = -1;
    return NULL;
}

int stripstartend(char *header){
    int whitespace = 0;
    int delete_i = 0;
    int y;
    for(int i = 0; header[i] != '\r';i++){
        if((header[i] == ' ') && (whitespace == 0)){
            delete_i = i + 1;
            whitespace = 1;
            //want to strip the \r as well
            for(y = 0;y < (int)(strlen(header) - (delete_i) - 1);y++){
                header[y] = header[y+delete_i];
            }
            header[y] = '\0';
            return 0;
        }
    }


    return -1;
    //header[i] = '\0';
}

void free_parse(char **to_free, int free_parse_n){
    for(int i = 0;i<=free_parse_n;i++){
        free(to_free[i]);
    }
}

int giveparse(int soc, char *messagen){
    char *tagfrom ="A04";
    char *tagto = "A05";
    char *tagdate = "A06";
    char *tagsubj = "A07";
    int mnlen = strlen(messagen);
    int mlen = mnlen + PARSELEN;

    int free_parse_n = NMALLOCEDSTART - 1;
    char *to_free[NMALLOCEDEND];

    int hlen = 0;
    int *header_len = &hlen;

    char *messagefrom = (char *)malloc(sizeof(char) * (mlen + strlen("FROM") + 1)); //malloc extra character for the null byte
    char *messageto = (char *)malloc(sizeof(char) * (mlen + strlen("TO") + 1));
    char *messagedate = (char *)malloc(sizeof(char) * (mlen + strlen("DATE") + 1));
    char *messagesubject = (char *)malloc(sizeof(char) * (mlen + strlen("SUBJECT") + 1));

    char *res = (char *)malloc(sizeof(char) * MESSAGELEN);

    to_free[0] = messagefrom;
    to_free[1] = messageto;
    to_free[2] = messagedate;
    to_free[3] = messagesubject;
    to_free[4] = res;

    sprintf(messagefrom, "%s fetch %s BODY.PEEK[HEADER.FIELDS (FROM)]\r\n", tagfrom, messagen);
    sprintf(messageto, "%s fetch %s BODY.PEEK[HEADER.FIELDS (TO)]\r\n", tagto, messagen);
    sprintf(messagedate, "%s fetch %s BODY.PEEK[HEADER.FIELDS (DATE)]\r\n", tagdate, messagen);
    sprintf(messagesubject, "%s fetch %s BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n", tagsubj, messagen);

    //============================================================================================

    if(send(soc, messagefrom, mlen + strlen("FROM"), 0) < 0){
        free_parse(to_free, free_parse_n);
        return -1;
    } 

    char *response = recv_rn(soc,res,MESSAGELEN, tagfrom);
    free_parse_n++;
    to_free[free_parse_n] = response;
    if(strstr(response, "Invalid messageset") != NULL){
        free_parse(to_free, free_parse_n);
        return -1;
    }

    char *header = parseheader(response, header_len, 0);
    free_parse_n++;
    to_free[free_parse_n] = header;
    if(header == NULL){
        free_parse(to_free, free_parse_n);
        return -1;
    } else{
        stripstartend(header);
        printf("From: %s\n", header);
    }

    //free(header);
    *header_len = 0;

    //=============================================================================================
    
    if(send(soc, messageto, mlen + strlen("TO"), 0) < 0){
        free_parse(to_free, free_parse_n);
        return -1;
    }

    char *response2 = recv_rn(soc,res,MESSAGELEN, tagto);
    free_parse_n++;
    to_free[free_parse_n] = response2;
    if(strstr(response2, "Invalid messageset") != NULL){
        free_parse(to_free, free_parse_n);
        return -1;
    }

    char *header2 = parseheader(response2, header_len, 0);
    free_parse_n++;
    to_free[free_parse_n] = header2;
    if(header2 == NULL){
        if(*header_len != -1){
            printf("To:\n");
        } else{
            free_parse(to_free, free_parse_n);
            return -1;
        }
    } else{
        stripstartend(header2);
        printf("To: %s\n", header2);
    }

    //free(header);
    *header_len = 0;

    //=============================================================================

    if(send(soc, messagedate, mlen + strlen("DATE"), 0) < 0){
        free_parse(to_free, free_parse_n);
        return -1;
    }

    //printf("about to call recv_rn\n");
    char *response3 = recv_rn(soc,res,MESSAGELEN, tagdate);
    free_parse_n++;
    to_free[free_parse_n] = response3;
    //printf("S: %s\n", response3);
    if(strstr(response3, "Invalid messageset") != NULL){
        free_parse(to_free, free_parse_n);
        return -1;
    }

        
    char *header3 = parseheader(response3, header_len, 0);
    free_parse_n++;
    to_free[free_parse_n] = header3;
    if(header3 == NULL){
        if(*header_len != -1){
            printf("%s\n", header3);
        } else{
            free_parse(to_free, free_parse_n);
            //printf("parse header failed\n");
            return -1;
        }
    } else{
        stripstartend(header3);
        printf("Date: %s\n", header3);
    }

    //free(header);
    *header_len = 0;

    //=======================================================================

    //printf("C: %s\n", messagesubject);
    if(send(soc, messagesubject, mlen + strlen("SUBJECT"), 0) < 0){
        free_parse(to_free, free_parse_n);
        return -1;
    }

    //printf("about to call recv_rn\n");
    char *response4 = recv_rn(soc,res,MESSAGELEN, tagsubj);
    free_parse_n++;
    to_free[free_parse_n] = response4;
    //printf("S: %s\n", response4);
    if(strstr(response4, "Invalid messageset") != NULL){
        free_parse(to_free, free_parse_n);
        return -1;
    }

    char *header4 = parseheader(response4, header_len, 0);
    free_parse_n++;
    to_free[free_parse_n] = header4;
    if(header4 == NULL){
        if(*header_len != -1){
            printf("Subject: <No subject>\n");
        } else{
            free_parse(to_free, free_parse_n);
            //printf("parse header failed\n");
            return -1;
        }
    } else{
        stripstartend(header4);
        printf("Subject: %s\n", header4);
    }


    free_parse(to_free, free_parse_n);
    return 0;
}

int givelist(int soc, char *exists){
    char *res = (char *)malloc(sizeof(char) * MESSAGELEN);
    char *message = (char *)malloc(sizeof(char) * MESSAGELEN);
    char *tag = (char *)malloc(sizeof(char) * MESSAGELEN);
    int subjectlen = strlen("Subject: ");

    int hlen = 0;
    int *header_len = &hlen;

    for(int i = 1;i<=atoi(exists);i++){
        sprintf(tag, "A%d", i);
        sprintf(message, "%s fetch %d BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n", tag, i);

        if(send(soc, message, GIVELISTLEN + (log10(i) + 1) + (log10(i) + 1), 0) < 0){
            free(message);
            free(res);
            free(tag);
            return -1;
        }

        char *response = recv_rn(soc,res,MESSAGELEN, tag);
        
        char *header = parseheader(response, header_len, 0);

        if(header == NULL){
            if(*header_len != -1){
                printf("%d: <No subject>\n", i);
            } else{
                free(res);
                free(tag);
                free(message);
                free(header);
                free(response);
                return -1;
            }
        } else if(header != NULL){
            
            char *choppedheader = header + subjectlen;

            choppedheader[*header_len - subjectlen - 1] = '\0';
            printf("%d: %s\n", i, choppedheader);
        }
        free(response);
        free(header);
        hlen = 0;
    }
    free(message);
    free(res);
    free(tag);
    return 0;
}

//===========================================================================
// unfinished give mime
/*
int givemime(int soc, char *messagen){
    char write_buffer[1024];
    snprintf(write_buffer, 1024, "A03 FETCH %s BODY.PEEK[]\r\n",messagen);
    ssize_t n = send_msg(soc, write_buffer);
    if(n<0){
        printf("write error!\n");
        exit(EXIT_FAILURE);
    }
    char one_line[10000]={0};
    char *token=" boundary=";
    int token_len=strlen(token);
    int count=0;
    char boundary[10000]={0};
    boundary[0]='-';
    boundary[1]='-';
    int start_body=0;
    while(1){
        while(1){
            char c;
            read(soc, &c, sizeof(char));
            one_line[count]=c;
            count++;
            if(c=='\n'){
                one_line[count]='\0';
                break;
            }
        }
        if(strncmp(one_line, token,token_len) == 0){
            extract_boundary(boundary, one_line+token_len);
        }

        //check if the current line is the start of the body
        if(strncmp(one_line, boundary, strlen(boundary)) == 0){
            start_body=1;
            int len=strlen(boundary);
            boundary[len]='-';
            boundary[len+1]='-';
            boundary[len+2]='\0';
            continue;
        }
        if(start_body==1 & strncmp(one_line, boundary, strlen(boundary)) == 0){
            break;
        }
        if(start_body==1){
            printf("%s", one_line);
        }
    }
}
// extract boundary value
void extract_boundary(char *boundary, char *line){
    int len=strlen(boundary);
    line[strlen(len)-1]='\0';
    int i;
    int count=2;
    for(i=0; i<strlen(line); i++){
        if(line[i]!='"' || line[i]!='\''){
            boundary[count]=line[i];
            count++;
        }
    }
    boundary[count]='\0';
}

*/

//int givemime(int soc, char *messagen){
//    return -1;
//}

/*
int givelist(int soc, char *messagen){
    char write_buffer[1024];
    snprintf(write_buffer, 1024, "A03 SEARCH ALL\r\n");
    ssize_t n=send_msg(soc, write_buffer);
    if(n<0){
        printf("write error!\n");
        exit(EXIT_FAILURE);
    }
    char read_buffer[1024];
    n=receive_msg(soc, read_buffer, sizeof(read_buffer));
    printf("%s", read_buffer);

    // extract seq num from read buffer
    int i;
    for(i=0; i<strlen(read_buffer); i++){
        if(read_buffer[i]=='\r' && read_buffer[i+1]=='\n'){
            break;
        }
    }
    read_buffer[i]='\0';
    char *delim=" "; //spliy by space
    char *token=strtok(read_buffer, delim);
    token=strtok(NULL, delim);
    while(token!=NULL){
        snprintf(write_buffer, 1024, "A04 FETCH %s BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n", token);
        send_msg(soc, write_buffer);
        receive_msg(soc, read_buffer, 1024);
        printf("%s", read_buffer);
        token=strtok(NULL, delim);
    }
}

*/
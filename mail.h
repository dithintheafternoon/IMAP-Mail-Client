#define COMMAND_MAX 11
#define COMMAND_MIN 6

#define SECOND_LAST 2

#define MANDATORY_ARGS 4

#define RETRIEVE 0
#define PARSE 1
#define MIME 2
#define LIST 3

#define PORT "143"

#define MESSAGELEN 300
#define TAGLEN 3

//lengths of different messages minus the variables that the user gives
#define LOGINLEN 13
#define SELFOLDERLEN 13
#define RETRIEVELEN 24
#define PARSELEN 40
#define NBUFFLEN 5
#define GIVELISTLEN 45

#define TIMEOUT 1000

#define DOUBLE 2
#define ROOMFORNULLBYTE 2
#define QUOTES 2

#define NMALLOCEDSTART 5
#define NMALLOCEDEND 13
//===================================================================================================

//this structure stores all commands read from the command line
    //will ignore flag -t
typedef struct commands{
    char *username;
    char *password;
    char *folder; //will be assigned INBOX if null
    char *message_number; //will be 0 if NULL
    int command;
    char *server_name;

    int user_space;
    int pass_space;
    int folder_space;

    int malloced;
} commands;

//===================================================================================================

char *bound_quotes(char *arg);
commands *read_commands(int argc, char *argv[]);
void free_comms(commands *comms);

int all_digits(char *str);

int connect_socket(char *server_name);

char *recv_rn(int soc, char *res, int res_len, char *tag);

void write_to_total(char *dest, char *src, int dest_len, int src_len);

int check_end(char *src, int src_len, char *tag);

//---------------------------------------------------------------------

int login(char *user, int ulen, char *pass, int plen, int soc);

int select_folder(int soc, char *folder, char *exists, int *exists_len_pointer);

//int find_most_recent(int soc, int n);

int get_exists(char *response, char *nbuffer, int *exists_len_pointer);

//---------------------------------------------------------------------

//int retrieve_remove_start_end(char *response);
int giveretrieve(int soc, char *messagen);

void free_parse(char **to_free, int free_parse_n);
int giveparse(int soc, char *messagen);
char *parseheader(char *response, int *header_len, int retrieve);
int stripstartend(char *header);

//int givemime(int soc, char *messagen);

//int remfromstart(char *sub, char *dom);
int givelist(int soc, char *exists);

/*
int givemime(int soc, char *messagen);
void extract_boundary(char *boundary, char *line);
int givelist(int soc, char *messagen);
*/
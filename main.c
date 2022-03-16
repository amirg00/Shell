#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define Localhost "127.0.0.1"
#define SERVER_PORT 5060

void getDirFiles(int tcp_port_flag, int sock){
    char curr_working_dir[256];
    if(getcwd(curr_working_dir, sizeof(curr_working_dir)) != NULL){
        DIR *dir = opendir(curr_working_dir);
        struct dirent *entry;
        while((entry = readdir(dir))) {
            if (tcp_port_flag) {
                send(sock, entry->d_name, strlen(entry->d_name) ,0);
            } else {
                printf("%s\n", entry->d_name);  //prints a dir file's name.
            }
        }
        closedir(dir);
    }
    else{
        perror("Unable to read the directory!");
    }
}

char* getInput(){
    int cap = 0;
    char ch;
    char* str = (char*)malloc((cap+1)*sizeof(char));
    char* cpy = str;
    while((ch = getchar()) != EOF && ch != '\n'){
        cap++;
        str = realloc(str, (cap+1) * sizeof(char));
        if(!str)
        {
            printf("Long input!");
            free(cpy);
            return NULL;
        }
        cpy = str;
        str[cap-1] = ch;
    }
    str[cap] = '\0';
    return str;
}

int main() {

    //--------------------------------------------------------/
    /****************** Start Of The Shell *******************/
    //--------------------------------------------------------/

    int sock; // tcp socket for TCP PORT command.
    struct sockaddr_in server_addr;
    socklen_t dataLenSock;
    int TCP_PORT_FLAG = 0; // FLAG IS OFF

    printf("-------------------------------------\n"
           "********** Welcome To Shell! ********\n"
           "-------------------------------------\n");
    while(1){
        char curr_working_dir[256];
        if(getcwd(curr_working_dir, sizeof(curr_working_dir)) != NULL){
            printf("%s>", curr_working_dir);
        }
        char* user_in = getInput();

        //--------------------------------------------------------/
        /******************  Commands Section  *******************/
        //--------------------------------------------------------/

        if (!strncmp("ECHO ", user_in, strlen("ECHO "))){
            user_in += sizeof (char) * strlen("ECHO ");
            printf("%s\n", user_in);
        }

        else if(!strcmp(user_in, "TCP PORT")){
            TCP_PORT_FLAG = 1; // SET FLAG TO TRUE
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0){
                perror("Error in socket");
                continue;
            }
            bzero(&server_addr, sizeof(server_addr));
            server_addr.sin_family      = AF_INET;
            server_addr.sin_port        = htons(SERVER_PORT);
            server_addr.sin_addr.s_addr = inet_addr(Localhost);

            int connRes = connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
            if (connRes == -1){
                perror("Error in socket");
                continue;
            }
            printf("connect succeeded\n");
            write(sock, "hey", strlen("hey"));

        }

        else if (!strcmp(user_in, "LOCAL")){
            printf("TCP Connection...\n");
        }

        // Syntax: COPY <SRC> <DEST>
        else if (!strcmp(user_in, "DIR")){
            getDirFiles(TCP_PORT_FLAG, sock);
        }

        else if (!strncmp("CD ", user_in, strlen("CD "))){
            user_in += sizeof (char) * strlen("CD ");
            if (chdir(user_in) == -1){  // chdir is a system call.
                perror("System cannot find the path specified");
            }
        }

        else if (!strcmp(user_in, "EXIT")){
            printf("Exiting...\n");
            free(user_in);
            if (sock >= 0){
                close(sock);
            }
            exit(1);
        }
        else{
            printf("Invalid syntax. Try again!\n");
        }
        free(user_in);
    }
}
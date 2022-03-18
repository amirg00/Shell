#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define Localhost "127.0.0.1"
#define SERVER_PORT 5060
#define SIZE 4096            // BUFFER SIZE (BYTES)


/**
* The method sends the given file in chunks of 1024 bytes to the server, in order
* to measure the time taken for the server to fully receive the file.
*
* @note the implementation uses fopen, fread and fwrite, which are library calls.
* @param file a given file pointer.
* @param dst_filename a given server socket to send the file to.
*/
void write_file(FILE *file, char* dst_filename) {
    char data[SIZE] = {0};
    FILE *dst_file;
    dst_file = fopen(dst_filename, "w");
    if (dst_file == NULL){
        perror("System cannot find the path specified");
    }
    while ((fread(data, 1, sizeof(data), file)) > 0) {
        printf("%s", data);
        fwrite(&data, sizeof(data), 1, dst_file);
    }
    fclose(file);
    fclose(dst_file);
}

/**
 * Function checks if the flag is on, if so then the message
 * will be sent to the server over the socket (in our case it's a tcp-socket).
 * O.W. function prints the message (through the standard output).
 * @param flag a given flag (in our case it's a tcp_port flag).
 * @param sock a given socket.
 * @param message a given message.
 */
void send_by_flag(int flag, int sock, char* message){
    if (flag) {
        send(sock, message, strlen(message) ,0);
        send(sock, "\n", strlen("\n") ,0);
    } else {
        printf("%s\n", message);
    }
}

/**
 * Function prints or sends message by the flag, when the message is the
 * list of directories' names in the current working directory.
 * @param tcp_port_flag flag which states if 'TCP PORT' command has entered.
 * @param sock a given socket.
 */
void getDirFiles(int tcp_port_flag, int sock){
    char curr_working_dir[256];
    if(getcwd(curr_working_dir, sizeof(curr_working_dir)) != NULL){
        DIR *dir = opendir(curr_working_dir);
        struct dirent *entry;
        while((entry = readdir(dir))) {
            send_by_flag(tcp_port_flag, sock, entry->d_name);
        }
        closedir(dir);
    }
    else{
        perror("Unable to read the directory!");
    }
}

/**
 * Function gets an input from the user, without a certain bound on the input's length.
 * @return the input string
 */
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

void execOtherCommand(char* command){
    char delim[] = " \n\r\t";
    char* token;
    char* vec[256] = {0};
    int k = 0;
    for (token = strtok(command, delim); token; token = strtok(NULL, delim))
    {
        vec[k++] = token;
    }
    vec[k] = NULL;

    int pid = fork();
    if(pid == -1){
        perror("couldn't create a child");
        return;
    }
    if(pid == 0){ // Child process
        execvp(vec[0], vec);
    }
    else{ // Parent process
        wait(NULL);
    }
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

        // ECHO COMMAND: ECHO <SOMETHING>
        if (!strncmp("ECHO ", user_in, strlen("ECHO "))){
            user_in += sizeof (char) * strlen("ECHO ");
            send_by_flag(TCP_PORT_FLAG, sock, user_in);
        }
        // 'TCP PORT' COMMAND: TCP PORT
        else if(!strcmp(user_in, "TCP PORT")){
            TCP_PORT_FLAG = 1; // SET FLAG TO TRUE

            // CREATING THE SERVER SOCKET
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0){
                perror("Error in socket");
                continue;
            }
            // SET THE SERVER IP AND PORT
            bzero(&server_addr, sizeof(server_addr));
            server_addr.sin_family      = AF_INET;
            server_addr.sin_port        = htons(SERVER_PORT);
            server_addr.sin_addr.s_addr = inet_addr(Localhost);

            // CONNECT TO SERVER
            int connRes = connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
            if (connRes == -1){
                perror("Error in socket");
                continue;
            }
            printf("connect succeeded\n");
        }
        // LOCAL COMMAND: LOCAL <SOMETHING>
        else if (!strcmp(user_in, "LOCAL")){ // close socket connections
            TCP_PORT_FLAG = 0; // set back to false
            if (sock >= 0){
                close(sock);
            }
        }
        // DIR COMMAND: DIR
        else if (!strcmp(user_in, "DIR")){
            getDirFiles(TCP_PORT_FLAG, sock);
        }
        // CD COMMAND: CD <SOMETHING>
        else if (!strncmp("CD ", user_in, strlen("CD "))){
            user_in += sizeof (char) * strlen("CD ");
            if (chdir(user_in) == -1){  // chdir is a system call.
                perror("System cannot find the path specified");
            }
        }
        // COPY COMMAND: Syntax: COPY <SRC> <DEST>
        else if(!strncmp("COPY ", user_in, strlen("COPY "))){
            user_in += sizeof (char) * strlen("COPY ");
            char src[256] = ""; char dest[256] = "";
            int space_flag = 0, m = 0, k = 0;
            for (int i = 0; i < strlen(user_in) ; ++i) {
                if (user_in[i] == ' '){
                    space_flag = 1;
                    continue;
                }
                if (!space_flag){
                    src[m++] = user_in[i];
                }
                else{
                    dest[k++] = user_in[i];
                }
            }

            FILE *src_file = fopen(src, "r");
            if (src_file == NULL) {
                perror("System couldn't open specified file");
                continue;
            }
            write_file(src_file, dest);

        }
        // DELETE COMMAND: DELETE <SOMETHING>
        else if(!strncmp("DELETE ", user_in, strlen("DELETE "))){
            user_in += sizeof (char) * strlen("DELETE ");
            if(unlink(user_in) == -1){ // This is a system call
                perror("System couldn't delete the file");
            }

        }
        // EXIT COMMAND: EXIT
        else if (!strcmp(user_in, "EXIT")){
            printf("Exiting...\n");
            free(user_in);
            exit(1);
        }
        else { // OTHER COMMAND: SYSTEM(COMMAND)
            execOtherCommand(user_in);
//            int res_state = system(user_in);
//            if (res_state == -1) {
//                send_by_flag(TCP_PORT_FLAG, sock, "Invalid syntax. Try again!");
//            }
        }
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <dirent.h>

#define Localhost "127.0.0.1"


void getDirFiles(){
    char curr_working_dir[256];
    if(getcwd(curr_working_dir, sizeof(curr_working_dir)) != NULL){
        DIR *dir = opendir(curr_working_dir);
        struct dirent *entry;
        while((entry = readdir(dir))){
            printf("%s\n", entry->d_name);  //prints a dir file's name.
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

    printf("-------------------------------------\n"
           "********** Welcome To Shell! ********\n"
           "-------------------------------------\n");
    while(1){
        char curr_working_dir[256];
        if(getcwd(curr_working_dir, sizeof(curr_working_dir)) != NULL){
            printf("%s>", curr_working_dir);
        }
        char* user_in = getInput();


        if (!strncmp("ECHO ", user_in, strlen("ECHO "))){
            user_in += sizeof (char) * strlen("ECHO ");
            printf("%s\n", user_in);
        }


        else if (!strcmp(user_in, "LOCAL")){
            printf("TCP Connection...\n");
        }

        // Syntax: COPY <SRC> <DEST>
        else if (!strcmp(user_in, "DIR")){
            getDirFiles();
        }

        else if (!strncmp("CD ", user_in, strlen("CD "))){
            user_in += sizeof (char) * strlen("CD ");
            if (chdir(user_in) == -1){  // chdir is a system call.
                perror("System cannot find the path specified");
            }
        }

        else if (!strcmp(user_in, "TCP PORT")){
            printf("TCP Connection...\n");
        }

        else if (!strcmp(user_in, "EXIT")){
            printf("Exiting...\n");
            free(user_in);
            exit(1);
        }
        else{
            printf("Invalid syntax. Try again!\n");
        }
        free(user_in);
    }
}

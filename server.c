#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <dirent.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define Localhost "127.0.0.1"
#define PORT 5060
#define SIZE 1024

int main() {

    int sock, clisock, bind_response;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char buff[SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error in socket\n");
        exit(1);
    }
    printf("Server socket created successfully.\n");

    // set the server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind server
    printf("bind\n");
    bind_response = bind(sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (bind_response < 0) {
        perror("Error: couldn't bind()\n");
        exit(1);
    }
    printf("Bind() successful\n");

    if (listen(sock, 1) == 0) {
        printf("Listening for any income connections...\n");
    } else {
        perror("Listen failed! something has occurred while listening.\n");
        exit(1);
    }
    addr_size = sizeof(clisock);
    clisock = accept(sock, (struct sockaddr *) &new_addr, &addr_size);
    if (clisock < 0){
        perror("server accept has failed!");
        exit(1);
    }

    while (1) {
        bzero(buff, SIZE);
        size_t bytes = read(clisock, buff, SIZE);
        if (bytes < 0) {
            perror("recv failed");
        } else if (bytes == 0) {
            break;
        } else {
            printf("%s", buff);
            bzero(buff, SIZE);
        }
    }
    // closing resources:
    close(clisock);
    close(sock);
    return 0;
}


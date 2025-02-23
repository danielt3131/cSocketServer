#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

int main(int argc, const char **argv) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        return EXIT_FAILURE;
    }
    unsigned short port = atoi(argv[1]);
    struct sockaddr_in saddr, caddr;

    memset(&saddr, '\0', sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(sockfd, &saddr, sizeof(saddr)) < 0) {
        perror("Cant bind port");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, 1000) < 0) {
        return EXIT_FAILURE;
    }

    int clientLength = sizeof(caddr);
    int i = 0;
    while (true) {
        int clientfd = accept(sockfd, &caddr, &clientLength);
        if (clientfd < 0) {
            perror("Connection error");
            return EXIT_FAILURE;
        }

        char buffer[1000];
        read(clientfd, buffer, sizeof buffer);
        int option = atoi(buffer);
        if (option == 1) {
            //sprintf(buffer, "%lu\n", time(NULL));
            write(clientfd, "Sun Feb 23 16:02:34 UTC 2025\n", 30);
        }
        printf("%d\n", i);
        i++;
        close(clientfd);

    }
    return EXIT_SUCCESS;
}
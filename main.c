#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include "server.h"

void acceptClient(int clientfd, char *buffer);

int main(int argc, const char **argv) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 0;

    int sendBuf = 4 * BUFFER;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // Increase TCP send buffer
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendBuf, sizeof(sendBuf));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &sendBuf, sizeof(sendBuf));
    setvbuf(stdout, NULL, _IONBF, 0);
    /*
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
*/
    int nodelay = 1;
    int result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    if (sockfd < 0) {
        perror("Error opening socket");
        return EXIT_FAILURE;
    }
    //unsigned short port = 2000;
    unsigned short port = atoi(argv[1]);
    struct sockaddr_in saddr, caddr;

    memset(&saddr, '\0', sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0) {
        perror("Cant bind port");
        return EXIT_FAILURE;
    }

    if (listen(sockfd, 25) < 0) {
        return EXIT_FAILURE;
    }

    socklen_t clientLength = sizeof(caddr);
    int i = 0;
    printf("Server FD: %d\n", sockfd);
    pthread_attr_t t_attr;
    //printf("Client FD1: %d\n", clientfd);
    pthread_attr_init(&t_attr);
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    //pthread_attr_setstacksize(&t_attr, stacksize);
    pthread_t thread;
    while (true) {
        char *buffer = malloc(BUFFER);
        //int *client = malloc(sizeof(int));
        int clientfd = accept(sockfd, (struct sockaddr*) &caddr, &clientLength);
        if (clientfd < 0) {
            perror("Connection error");
            return EXIT_FAILURE;
        }
        //printf("Client FD: %d\n", clientfd);
        *(int *) buffer = clientfd;
        pthread_create(&thread, &t_attr, serverThread, buffer);
        printf("%d\n", i);
        i++;
    }
    return EXIT_SUCCESS;
}

void acceptClient(int clientfd, char *buffer) {
    pthread_attr_t t_attr;
    //printf("Client FD1: %d\n", clientfd);
    pthread_attr_init(&t_attr);
    size_t stacksize = 85849014272;
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    //pthread_attr_setstacksize(&t_attr, stacksize);
    pthread_t thread;
    //int *client = malloc(sizeof(int));
    *(int *) buffer = clientfd;
    pthread_create(&thread, &t_attr, serverThread, buffer);
}

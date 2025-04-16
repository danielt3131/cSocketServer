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

#include "threadpool.h"
#include "server.h"

#define THREAD_POOL_SIZE 5 
#define WAIT_TIME 100

pthread_t threadPool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *worker(void * arg);
int main(int argc, const char **argv) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    nice(-20);    
    int sendBuf = 5 * BUFFER;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // Increase TCP send buffer
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendBuf, sizeof(sendBuf));
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &sendBuf, sizeof(sendBuf));
    setvbuf(stdout, NULL, _IONBF, 0);
    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
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
    //pthread_attr_t t_attr;
    //printf("Client FD1: %d\n", clientfd);
    //pthread_attr_init(&t_attr);
    //pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);
    //pthread_attr_setstacksize(&t_attr, stacksize);
    pthread_t *threadPool = malloc(THREAD_POOL_SIZE * sizeof(pthread_t));
    if (threadPool == NULL) {
    	perror("Unable to create thread pool");

        return EXIT_FAILURE;
    }
    for (int j = 0; j < THREAD_POOL_SIZE; j++) {
        pthread_create(&threadPool[j], NULL, worker, NULL);
    }
    while (true) {
        //char *buffer = malloc(BUFFER);
        //int *client = malloc(sizeof(int));
	    //usleep(2000);
        int* client = malloc(sizeof(int));
        int clientfd = accept(sockfd, (struct sockaddr*) &caddr, &clientLength);
        *client = clientfd;
        pthread_mutex_lock(&mutex);
        enqueue(client);
        //pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        puts("Added client to work queue\n");
        /*
        if (clientfd < 0) {
            perror("Connection error");
            return EXIT_FAILURE;
        }
        //printf("Client FD: %d\n", clientfd);
        *(int *) buffer = clientfd;
        pthread_create(&thread, &t_attr, serverThread, buffer);
        */
        printf("%d\n", i);
        i++;
    }
    return EXIT_SUCCESS;
}

void *worker(void * arg) {
    nice(-15); 
    while(1) {
        usleep(WAIT_TIME);
        int *client;
        pthread_mutex_lock(&mutex);
        //pthread_cond_wait(&cond, &mutex);
        client = dequeue();
        pthread_mutex_unlock(&mutex);
        if (client != NULL) {
            serverThread(client);
        }
    }
}

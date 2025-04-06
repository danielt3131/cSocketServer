#include "server.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#define BUFFER 100000

void* serverThread(void* clientFd) {
    pthread_detach(pthread_self());
    char buffer[BUFFER];
    printf("ClientFD Address %p\n", clientFd);
    int clientfd = *(int *) clientFd;
    printf("Thread %lu\n", pthread_self());
    printf("Client FD2: %d\n", clientfd);
    struct sysinfo info;
    puts("Connected to client\n");
    //Used for sending output to client
    //Use syscall to read in the option
    read(clientfd, buffer, sizeof buffer);
    int option = atoi(buffer);

    if (option == 1) {
        sprintf(buffer, "%lu\n", time(NULL));
        //write(clientfd, "Sun Feb 23 16:02:34 UTC 2025\n", 30);
        send(clientfd, buffer, strlen(buffer), 0);
    }

    if (option == 2) {
        sysinfo(&info);
        sprintf(buffer, "Uptime: %lu\n", info.uptime);
        send(clientfd, buffer, strlen(buffer), 0);
    }
    if (option == 3) {
        sysinfo(&info);
        sprintf(buffer, "Free memory %lu\n", info.freeram);
        send(clientfd, buffer, strlen(buffer), 0);
    }

    if (option == 4) {
        FILE *commandOutput = popen("netstat", "r");
        while (fgets(buffer, BUFFER, commandOutput) != NULL) {
            send(clientfd, buffer, strlen(buffer), 0);
        }
        pclose(commandOutput);
    }

    if (option == 5) {
        FILE *commandOutput = popen("w", "r");
        while (fgets(buffer, BUFFER, commandOutput) != NULL) {
            send(clientfd, buffer, strlen(buffer), 0);
        }
        pclose(commandOutput);
    }

    if (option == 6) {
        FILE *commandOutput = popen("ps aux", "r");
        while (fgets(buffer, BUFFER, commandOutput) != NULL) {
            send(clientfd, buffer, strlen(buffer), 0);
        }
        pclose(commandOutput);
    }
    // Flush the buffer
    puts("Flushing buffer");

    // Send shutdown signal to client
    if (shutdown(clientfd, SHUT_WR) < 0) {
        perror("Unable to send shutdown signal\n");
        perror(strerror(errno));
    }
    // Close the client socket
    //fclose(client);
    close(clientfd);

    puts("Closed connection\n");
    //exit(EXIT_SUCCESS);
    free(clientFd);
    pthread_exit(NULL);
}

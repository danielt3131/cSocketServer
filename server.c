#include "server.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>


void* serverThread(void* clientFd) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    //char buffer[BUFFER];
    //printf("ClientFD Address %p\n", clientFd);
    int clientfd = *(int *) clientFd;
    free(clientFd);
    char *buffer = malloc(BUFFER);
    //printf("Thread %lu\n", pthread_self());
    //printf("Client FD2: %d\n", clientfd);
    struct sysinfo info;
    puts("Connected to client\n");
    //Used for sending output to client
    //Use syscall to read in the option
    buffer[0] = '\0';
    read(clientfd, buffer, 2);
    int option = atoi(buffer);
    int length = 0;
    if (option == 1) {
        sprintf(buffer, "%lu\n", time(NULL));
        //write(clientfd, "Sun Feb 23 16:02:34 UTC 2025\n", 30);
        send(clientfd, buffer, strlen(buffer), MSG_DONTWAIT);
    }

    if (option == 2) {
        sysinfo(&info);
        sprintf(buffer, "Uptime: %lu\n", info.uptime);
        send(clientfd, buffer, strlen(buffer), MSG_DONTWAIT);
    }
    if (option == 3) {
        sysinfo(&info);
        sprintf(buffer, "Free memory %lu\n", info.freeram);
        send(clientfd, buffer, strlen(buffer), MSG_DONTWAIT);
    }

    if (option == 4) {
        FILE *commandOutput = popen("netstat", "r");
        while ((length = fread(buffer, 1, BUFFER, commandOutput)) > 0) {
            send(clientfd, buffer, length, MSG_DONTWAIT);
        }
        pclose(commandOutput);
    }

    if (option == 5) {
        FILE *commandOutput = popen("w", "r");
        while ((length = fread(buffer, 1, BUFFER, commandOutput)) > 0) {
            send(clientfd, buffer, length, MSG_DONTWAIT);
        }
        pclose(commandOutput);
    }

    if (option == 6) {
        FILE *commandOutput = popen("ps aux", "r");
        while (fgets(buffer, BUFFER, commandOutput)) {
            size_t len = strlen(buffer);
            size_t total_sent = 0;

            while (total_sent < len) {
                ssize_t sent = send(clientfd, buffer + total_sent, len - total_sent, MSG_DONTWAIT);
                if (sent == -1) {
                    perror("send");
                    break;
                }
                total_sent += sent;
            }
        }

        pclose(commandOutput);
    }
    // Flush the buffer
    //puts("Flushing buffer");

    // Send shutdown signal to client
    if (shutdown(clientfd, SHUT_WR) < 0) {
        perror("Unable to send shutdown signal\n");
        perror(strerror(errno));
    }
    // Close the client socket
    //fclose(client);
    close(clientfd);
    free(buffer);
    puts("Closed connection\n");
    //exit(EXIT_SUCCESS);

    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_time_ms = (end.tv_sec - start.tv_sec) * 1000 +
                            (end.tv_nsec - start.tv_nsec) / 1000000;

    printf("Elapsed time: %ld ms\n", elapsed_time_ms);
    return NULL;
    //pthread_exit(NULL);
}

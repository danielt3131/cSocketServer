#include "server.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>

void serverThread(int clientfd) {
    char buffer[1000];
    struct sysinfo info;
    puts("Connected to client\n");
    //Used for sending output to client
    FILE *client = fdopen(clientfd, "r+");

    //Use syscall to read in the option
    read(clientfd, buffer, sizeof buffer);
    int option = atoi(buffer);

    if (option == 1) {
        fprintf(client, "%lu\n", time(NULL));
        //write(clientfd, "Sun Feb 23 16:02:34 UTC 2025\n", 30);
    }

    if (option == 2) {
        sysinfo(&info);
        fprintf(client, "Uptime: %lu\n", info.uptime);
    }
    if (option == 3) {
        sysinfo(&info);
        fprintf(client, "Free memory %lu\n", info.freeram);
    }

    if (option == 4) {
        FILE *commandOuput = popen("netstat", "r");
        while (fgets(buffer, 1000, commandOuput) != NULL) {
            fputs(buffer, client);
        }
        pclose(commandOuput);
    }

    if (option == 5) {
        FILE *commandOuput = popen("w", "r");
        while (fgets(buffer, 1000, commandOuput) != NULL) {
            fputs(buffer, client);
        }
        pclose(commandOuput);
    }

    if (option == 6) {
        FILE *commandOuput = popen("ps aux", "r");
        while (fgets(buffer, 1000, commandOuput) != NULL) {
            fputs(buffer, client);
        }
        pclose(commandOuput);
    }
    // Flush the buffer
    fflush(client);

    shutdown(clientfd, SHUT_WR);    // Send shutdown signal to client
    // Close the client socket
    fclose(client);
    close(clientfd);

    puts("Closed connection\n");
    exit(EXIT_SUCCESS);
}

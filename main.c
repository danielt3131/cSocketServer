#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <time.h>
#include <sys/sysinfo.h>
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
    struct sysinfo info;
    while (true) {
        int clientfd = accept(sockfd, &caddr, &clientLength);
        if (clientfd < 0) {
            perror("Connection error");
            return EXIT_FAILURE;
        }

        char buffer[1000];

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

        printf("%d\n", i);
        i++;
        fclose(client);
        // Close the client socket
        close(clientfd);

    }
    return EXIT_SUCCESS;
}
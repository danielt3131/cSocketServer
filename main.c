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

#define INIT_THREAD_POOL_SIZE 5
#define WAIT_TIME 100
#define WORKER_DELAY 1
#define QUEUE_EMPTY 2
#define POOL_FREE_TARGET 50
#define MAX_PROCESS_TIME 500

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t *threadPool;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

long *processTime;
int numThreads = INIT_THREAD_POOL_SIZE;
int poolAllocationAmount = INIT_THREAD_POOL_SIZE;
void *worker(void * arg);
bool processTimeChecker();  // Checks to see if additional threads need to be created
void createAdditionalWorkers();
void removeAdditionalWorkers(int flag);
bool createWorkerFreeze = false;
void *processCheckWorker(void *arg);

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

    /**
     * Creates thread pool
     */
    pthread_t *threadPool = malloc(INIT_THREAD_POOL_SIZE * sizeof(pthread_t));
    processTime = malloc(INIT_THREAD_POOL_SIZE * sizeof(long));
    //printf("%p\n", processTime);
    if (threadPool == NULL) {
    	perror("Unable to create thread pool");

        return EXIT_FAILURE;
    }
    for (int j = 0; j < INIT_THREAD_POOL_SIZE; j++) {
        // Pass thread number to thread
        int *threadNum = malloc(sizeof(int));
        *threadNum = j;
        pthread_create(&threadPool[j], NULL, worker, threadNum);
        processTime[j] = 0;
    }
    pthread_t timeChecker;
    pthread_create(&timeChecker, NULL, processCheckWorker, NULL);
    while (true) {
        //char *buffer = malloc(BUFFER);
        //int *client = malloc(sizeof(int));
	    //usleep(2000

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
    int workerID = *(int *) arg;
    free(arg);
    nice(-15);
    while(true) {
        usleep(WAIT_TIME);
        int *client;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        pthread_mutex_lock(&mutex);
        //pthread_cond_wait(&cond, &mutex);
        client = dequeue();
        pthread_mutex_unlock(&mutex);
        if (client != NULL) {
            serverThread(client, workerID);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        } else {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        }
    }
}

bool processTimeChecker() {
    if (queueSize > 50) {
        long result = 0;
        for (int i = 0; i < numThreads; i++) {
            result += processTime[i];
        }
        result = result / numThreads;  // Calculate average
        printf("Average Time %ld\n", result);
        // Average response time under 100ms
        if (result < MAX_PROCESS_TIME && !createWorkerFreeze) {
            return true;  // Signal to create new threads
        } else if (result >= MAX_PROCESS_TIME) {
            removeAdditionalWorkers(WORKER_DELAY);  // Remove last batch of workers
            puts("Too many workers\n");
            return false;
        }
    }
    return false;
}

void createAdditionalWorkers() {
    int origNumThreads = numThreads;
    puts("Creating additional workers\n");
    numThreads += 5;
    // Only reallocate if needed
    if (poolAllocationAmount < numThreads) {
        poolAllocationAmount = numThreads;
        printf("Pool %d\n", poolAllocationAmount);
        threadPool = realloc(threadPool, poolAllocationAmount * sizeof(pthread_t));
        processTime = realloc(processTime, poolAllocationAmount * sizeof(long));
    }
    for (int i = origNumThreads; i < numThreads; i++) {
        int *workerID = malloc(sizeof(int));
        *workerID = i;
        pthread_create(&threadPool[i], NULL, worker, workerID);
        processTime[i] = 0;
    }
}
// Remove some additional workers if the average processing time goes above 200ms or if queue is empty
void removeAdditionalWorkers(int flag) {
    int targetAmount = INIT_THREAD_POOL_SIZE;
    if (flag == QUEUE_EMPTY) {
        createWorkerFreeze = false;
    } else {
        targetAmount = numThreads - 5;
        createWorkerFreeze = true;  // Don't create anymore workers until queue is cleared
    }
    puts("Removing additional workers\n");
    for (int i = numThreads - 1; i >= targetAmount; i--) {
        pthread_cancel(threadPool[i]);
        processTime[i] = 0;
    }
    numThreads = targetAmount;
    // Reclaim some of the pool's memory if there is an excessive amount allocated.
    if (poolAllocationAmount > POOL_FREE_TARGET) {
        poolAllocationAmount = POOL_FREE_TARGET;
        threadPool = realloc(threadPool, poolAllocationAmount * sizeof(pthread_t));
        processTime = realloc(processTime, poolAllocationAmount * sizeof(long));
    }
}

void *processCheckWorker(void *arg) {
    while (true) {
        //usleep(250000);
        if (processTimeChecker()) {
            printf("%d workers\n", numThreads);
            createAdditionalWorkers();
            printf("%d workers\n", numThreads);
            usleep(200000);
        }
        if (queueSize == 0 && numThreads > INIT_THREAD_POOL_SIZE) {
            removeAdditionalWorkers(QUEUE_EMPTY);
            printf("%d workers\n", numThreads);
        }
    }
}
#ifndef SERVER_H
#define SERVER_H
#include <pthread.h>
void* serverThread(int *clientFd, int workerID);
extern long *processTime;
extern pthread_mutex_t timeMutex;
#define BUFFER 16384
#endif

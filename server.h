#ifndef SERVER_H
#define SERVER_H
void* serverThread(int *clientFd, int workerID);
extern long *processTime;
#define BUFFER 16384
#endif

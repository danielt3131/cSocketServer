#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <pthread.h>
struct node {
	int *clientfd;
	struct node* next;
};
extern int queueSize;
void enqueue(int *clientfd);
int* dequeue();

#endif

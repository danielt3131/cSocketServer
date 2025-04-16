#include "server.h"
#include "threadpool.h"
#include <stdlib.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>


struct node* head = NULL;
struct node* tail = NULL;
int queueSize = 0;

int* dequeue() {
   // printf("%p\n", head);
    if (head == NULL) {
        queueSize = 0;
        return NULL;
    } else {
        int *result = head->clientfd;
        struct node* temp = head;
        head = head->next;
        if (head == NULL) {
            tail = NULL;
        }
        free(temp);
        //printf("%p\n", head);
        queueSize--;
        return result;
    }
}

void enqueue(int *clientfd) {
    struct node* newNode = malloc(sizeof(struct node));
    //printf("%p | %p\n", tail, newNode);
    newNode->clientfd = clientfd;
    newNode->next = NULL;
    if (tail == NULL) {
        head = newNode;
    } else {
        tail->next = newNode;
    }
    queueSize++;
    tail = newNode;
}




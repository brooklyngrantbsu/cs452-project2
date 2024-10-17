#include <pthread.h>
#include "lab.h"

typedef struct node
{
    void *data;
    struct node *next;
} node_t;

struct queue
{
    node_t *head;
    node_t *tail;
    int currSize;
    int capacity;
    pthread_mutex_t mutex;
    pthread_cond_t notEmpty;
    pthread_cond_t notFull;
};


queue_t queue_init(int capacity)
{
    queue_t queue = (queue_t)malloc(sizeof(struct queue)); // make queue

    // initialize each property
    queue->head = NULL;
    queue->tail = NULL;
    queue->currSize = 0;
    queue->capacity = capacity;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->notEmpty, NULL);
    pthread_cond_init(&queue->notFull, NULL);

    return queue;
}


void queue_destroy(queue_t q) {
    // lock queue while destroying
    pthread_mutex_lock(&q->mutex);
    pthread_cond_broadcast(&q->notEmpty);
    pthread_cond_broadcast(&q->notFull);

    // free all nodes first
    while (q->head != NULL) {
        node_t *tmp = q->head;
        q->head = q->head->next;
        free(tmp);
    }

    pthread_mutex_unlock(&q->mutex); // unlock mutex
    
    // destroy mutex and conds
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->notEmpty);
    pthread_cond_destroy(&q->notFull);

    free(q); // free queue object
}


void enqueue(queue_t q, void *data) {
    pthread_mutex_lock(&q->mutex); // only one edit at a time

    while (q->currSize == q->capacity) { // if at capacity, wait until not
        pthread_cond_wait(&q->notFull, &q->mutex);
    }

    // make node to add
    node_t *new_node = (node_t *)malloc(sizeof(node_t));
    new_node->data = data;
    new_node->next = NULL;

    // add to end
    if (q->tail != NULL) {
        q->tail->next = new_node; // this is new after tail
    } else {
        q->head = new_node; // no nodes it is also head
    }
    q->tail = new_node; // it is tail now
    q->currSize++;

    pthread_cond_signal(&q->notEmpty); // has item in it let know
    pthread_mutex_unlock(&q->mutex); // unlock mutex
}

void *dequeue(queue_t q) {
    pthread_mutex_lock(&q->mutex); // don't change queue while this is going

    while (q->currSize == 0) { // wait until theres actually something to dequeue
        pthread_cond_wait(&q->notEmpty, &q->mutex);
    }

    // simple set head to next in line
    node_t *tmp = q->head;
    void *data = tmp->data;
    q->head = tmp->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(tmp); // free node
    q->currSize--;

    pthread_cond_signal(&q->notFull); // not full anymore (to capacity)
    pthread_mutex_unlock(&q->mutex); // unlock!

    return data;
}

void queue_shutdown(queue_t q) {
    // todo
}

bool is_empty(queue_t q) {
    return q->currSize == 0;
}

bool is_shutdown(queue_t q) {
    return false;// todo
}


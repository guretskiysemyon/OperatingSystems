/*
 * Author: Semyon Guretskiy
 * Date: 25/09/2023
 */


#include "Queue_U.h"
#include <stdio.h>
#include <stdlib.h>


Queue_U* create_queue_u(){
    Queue_U* q = (Queue_U*)malloc(sizeof(Queue_U));     // Allocate space
    if (q == NULL){
        printf("Error! Memory allocating\n");
        exit(1);
    }
    q->first = NULL;
    q->last = NULL;
    pthread_mutex_init(&q->mutex, NULL);    // Initialize mutex and semarphore
    sem_init(&q->full, 0, 0);
    return q;
}


Node* create_node_by_value(int pr, int ind, int cat){
    // Create a News and Node structure.
    News* n = create_new(pr, ind,cat);
    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL){
        printf("Error! Memory allocating\n");
        exit(1);
    }
    node->News = n;
    node->next = NULL;
    return node;
}


Node* create_node_by_new(News* news){

    Node* node = (Node*)malloc(sizeof(Node));
    if (node == NULL){
        printf("Error! Memory allocating\n");
        exit(1);
    }
    node->News = news;
    node->next = NULL;
    return node;
}


void enqueue_u_mut(Queue_U* queue, Node* node){
    pthread_mutex_lock(&queue->mutex);
    enqueue_u(queue, node);
    pthread_mutex_unlock(&queue->mutex);
    sem_post(&queue->full);     // Increase semarphore value
}

void enqueue_u(Queue_U* queue, Node* node){

    // If the queue is empty, the provided node becomes both the first and last node.
    if (queue->first == NULL){
        queue->first = node;
        queue->last = node;
        return;
    }
    // If the queue is not empty, the provided node is added to the end of the queue.
    queue->last->next = node;
    queue->last = node;
}


News* dequeue_u_mut(Queue_U* queue){

    // Decrease semarphore and lock mutex
    sem_wait(&queue->full);
    pthread_mutex_lock(&queue->mutex);
    News* news = dequeue_u(queue);
    pthread_mutex_unlock(&queue->mutex);
    return news;
}


News* dequeue_u(Queue_U* queue){
    Node* node;
    News* news;
    // If queue is empty
    if (queue->first == NULL){
        return NULL;
    }

    // If queue has one element
    if (queue->first == queue->last){
        node = queue->first;
        queue->first = NULL;
        queue->last = NULL;
        news = node->News;
        free(node);
        return news;
    }
    // General case
    node = queue->first;
    queue->first = node->next;
    news = node->News;
    free(node);
    return news;
}


void delete_queue_u(Queue_U* q){
    Node* curr = q->first;

    // Free every element in queue
    while (curr != NULL) {
        Node* temp = curr;
        curr = curr->next;
        free(temp);
    }
    // Destroy semarphore and mutex
    sem_destroy(&q->full);
    pthread_mutex_destroy(&q->mutex);
    free(q);
}
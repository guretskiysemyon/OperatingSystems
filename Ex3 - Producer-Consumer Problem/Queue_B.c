/*
 * Author: Semyon Guretskiy
 * Date: 25/09/2023
 */


#include "Queue_B.h"
#include <stdio.h>
#include <stdlib.h>


Queue_B *create_queue_b(int size)
{
    // Allocate memory for the news array.
    News **news = (News **)malloc(sizeof(News *) * size);
    if (news == NULL)
    {
        printf("Error! Memory allocating\n");
        exit(1);
    }

    // Allocate memory for the Queue_B structure.
    Queue_B *q = (Queue_B *)malloc(sizeof(Queue_B));
    if (q == NULL)
    {
        printf("Error! Memory allocating\n");
        exit(1);
    }

    // Initialize the first news slot to NULL.
    news[0] = NULL;

    // Set the size, first, and last indices of the queue.
    q->size = size;
    q->first = 0;
    q->last = 0;

    // Set the news array and initialize synchronization primitives.
    q->news = news;
    pthread_mutex_init(&q->mutex, NULL);
    sem_init(&q->full, 0, 0);
    sem_init(&q->empty, 0, size);

    return q;
}



int enqueue_b_mut(Queue_B *q, News *n) {
    sem_wait(&q->empty);        //decrease number of empty slots
    pthread_mutex_lock(&q->mutex); //lock mutex
    int result = enqueue_b(q, n);
    pthread_mutex_unlock(&q->mutex);    //unlock mutex
    sem_post(&q->full);             // increase number of full slots.
    return result;
}

int enqueue_b(Queue_B *q, News *n) {
    // Check if the queue is initially empty.
    if (q->first == q->last && q->news[q->first]== NULL)
    {
        q->news[q->first] = n;
        return 0;
    }

    // Calculate the new index for the last element.
    int new_ind = (q->last + 1) % q->size;

    // Check if the new index is equal to the first index (queue is full).
    if (new_ind == q->first){
        return -1;
    }

    // Insert the news item and update the last index.
    q->news[new_ind] = n;
    q->last = new_ind;

    return 0;
}


News* dequeue_b_mut(Queue_B *q){
    sem_wait(&q->full); // Wait until there are items to dequeue (full semaphore).
    pthread_mutex_lock(&q->mutex); // Lock the mutex
    News* result = dequeue_b(q);
    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->empty);    // Increment the 'empty' semaphore to indicate an available slot.
    return result;
}


News* try_dequeue_b_mut(Queue_B *q){

    // Attempt to decrement the 'full' semaphore to check if there are items available.
    if (sem_trywait(&q->full) != 0)
        return NULL;

    // Attempt to acquire the mutex lock for the queue.
    if (pthread_mutex_trylock(&q->mutex)!= 0){
        sem_post(&q->full);
        return NULL;
    }
    // Dequeue a news item from the queue.
    News* result = dequeue_b(q);
    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->empty);
    return result;
}




News *dequeue_b(Queue_B *q)
{
    News *n;

    // Check if the queue is empty (first == last).
    if (q->first == q->last)
    {
        // If the slot is NULL, return NULL (queue is empty).
        if (q->news[q->first] == NULL)
        {
            return NULL;
        }
        // Dequeue the news item from the front of the queue.
        n = q->news[q->first];
        q->news[q->first] = NULL;      // Set clot to NULL
        return n;
    }

    // Dequeue the news item from the front of the queue.
    n = q->news[q->first];
    q->news[q->first] = NULL;
    q->first = (q->first + 1)% q->size;     // Update the 'first' index
    return n;
}

void delete_queue_b(Queue_B* q){
    sem_destroy(&q->full);  // Destroy the 'full' semaphore.
    sem_destroy(&q->empty); // Destroy the 'empty' semaphore.
    pthread_mutex_destroy(&q->mutex);   // Destroy the mutex.
    free(q->news);
    free(q);    

}
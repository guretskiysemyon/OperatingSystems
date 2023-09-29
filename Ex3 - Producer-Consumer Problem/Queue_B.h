/*
 * Author: Semyon Guretskiy
 * Date: 25/09/2023
 */

#ifndef EX3_QUEUE_B_H
#define EX3_QUEUE_B_H

#include "News.h"
#include <semaphore.h>
#include <pthread.h>

/*
 * Struct: Queue_B
 * Description: Bounded Queue structure for storing news.
 *
 * Members:
 *  - size: Size of the queue
 *  - first: Index of the first element in the queue
 *  - last: Index of the last element in the queue
 *  - mutex: Mutex for queue synchronization
 *  - empty: Semaphore representing the number of empty slots in the queue
 *  - full: Semaphore representing the number of filled slots in the queue
 *  - news: Array of pointers to News structures (the actual news items)
 */
typedef struct
{
    int size;
    int first;
    int last;
    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;
    News **news;

} Queue_B;

/*
 * Function: enqueue_b_mut
 * Description: Enqueues a news item into a bounded queue with mutex-based synchronization.
 * 
 * Parameters:
 *  - Queue_B* q: Pointer to the bounded queue struct
 *  - News* n: Pointer to the news item to insert
 * 
 * Return:
 *  - 0: Success
 *  - -1: Failure
 */
int enqueue_b_mut(Queue_B *q, News *n);


/*
 * create_queue_b - Function to create a new bounded queue.
 * 
 * Parameters:
 *  int size - The maximum size of the bounded queue.
 * 
 * Return:
 *  Queue_B* - Pointer to the newly created bounded queue.
 * 
 * Description:
 *  This function allocates memory for a new bounded queue structure and initializes its
 *  attributes, including the array for storing news, mutex for synchronization,
 *  and semaphores for tracking the number of empty and full slots in the queue.
 *  The function returns a pointer to the created queue.
 */
Queue_B *create_queue_b(int size);

/*
 * enqueue_b - Function to enqueue a news item into a bounded queue.
 * 
 * Parameters:
 *  Queue_B* q - Pointer to the bounded queue.
 *  News* n - Pointer to the news item to be enqueued.
 * 
 * Return:
 *  int - 0 on success, -1 if the queue is full.
 */
int enqueue_b(Queue_B *q, News *n);


/*
 * dequeue_b_mut - Function to dequeue a news item from a bounded queue with mutex synchronization.
 * 
 * Parameters:
 *  Queue_B* q - Pointer to the bounded queue.
 * 
 * Return:
 *  News* - Pointer to the dequeued news item.
 */
News* dequeue_b_mut(Queue_B *q);


/*
 * dequeue_b - Function to dequeue a news item from a bounded queue.
 * 
 * Parameters:
 *  Queue_B* q - Pointer to the bounded queue.
 * 
 * Return:
 *  News* - Pointer to the dequeued news item.
 */
News *dequeue_b(Queue_B *q);



/*
 * try_dequeue_b_mut - Attempt to dequeue a news item from a bounded queue using mutex and semaphores.
 * 
 * Parameters:
 *  Queue_B* q - Pointer to the bounded queue.
 * 
 * Return:
 *  News* - Pointer to the dequeued news item if successful, or NULL if not.
 * 
 * Description:
 * If queue is empty, NULL will be return and will be no waiting.
 */
News* try_dequeue_b_mut(Queue_B *q);


/*
 * delete_queue_b - Delete a bounded queue and release associated resources.
 * 
 * Parameters:
 *  Queue_B* q - Pointer to the bounded queue to be deleted.
 */
void delete_queue_b(Queue_B* q);




#endif //EX3_QUEUE_B_H

/*
 * Author: Semyon Guretskiy
 * Date: 25/09/2023
 */


#ifndef EX3_QUEUE_U_H
#define EX3_QUEUE_U_H
#include "News.h"
#include "semaphore.h"
#include <pthread.h>


/*
 * Unbounded Queue Node Structure:
 */
typedef struct Node
{
    News* News;         // Pointer to a News item.
    struct Node* next;      // Pointer to the next node in the queue.
}Node;


/*
 * Unbounded Queue Structure:
 *  - An unbounded queue consists of a linked list of nodes, where each node contains a News item.
 *  - The queue has a 'first' pointer pointing to the first node and a 'last' pointer pointing to the last node.
 *  - It also uses a mutex for thread safety and a semaphore ('full') for synchronization.
 */
typedef struct
{
    Node* first;
    Node* last;
    pthread_mutex_t mutex;
    sem_t full;

}Queue_U;



/*
 * create_queue_u - Create an Unbounded Queue
 * This function dynamically allocates memory for a new unbounded queue structure and initializes its components.
 *
 * Return:
 *   Queue_U* - Pointer to the newly created unbounded queue.
 *
 */
Queue_U* create_queue_u();


/*
 * create_node_by_value - Create a Node with a News Value
 *
 * Parameters:
 *   int pr - Producer ID
 *   int ind - News Index
 *   int cat - Category
 *
 * Return:
 *   Node* - Pointer to the newly created node.
 */
Node* create_node_by_value(int pr, int ind, int cat);


/*
 * enqueue_u - Enqueue a Node into an Unbounded Queue
 *
 * Parameters:
 *   Queue_U* queue - Pointer to the unbounded queue.
 *   Node* node - Pointer to the node to be enqueued.
 */
void enqueue_u(Queue_U* queue, Node* node);


/*
 * enqueue_u_mut - Enqueue a Node into an Unbounded Queue with Mutex Synchronization
 *
 * Parameters:
 *   Queue_U* queue - Pointer to the unbounded queue.
 *   Node* node - Pointer to the node to be enqueued.
 */
void enqueue_u_mut(Queue_U* queue, Node* node);


/*
 * dequeue_u - Dequeue a Node from an Unbounded Queue
 *
 * Parameters:
 *   Queue_U* queue - Pointer to the unbounded queue.
 *
 * Return:
 *   News* - Pointer to the news item dequeued from the queue.
 *           Returns NULL if the queue is empty.
 */
News* dequeue_u(Queue_U* queue);


/*
 * dequeue_u_mut - Dequeue a Node from an Unbounded Queue with Mutex and Semaphore Synchronization
 *
 * This function dequeues a node from an unbounded queue while ensuring thread safety using a mutex
 * and semaphores. It waits for the 'full' semaphore to be signaled before attempting to dequeue.
 *
 * Parameters:
 *   Queue_U* queue - Pointer to the unbounded queue.
 *
 * Return:
 *   News* - Pointer to the news item dequeued from the queue.
 *           Returns NULL if the queue is empty.
 */
News* dequeue_u_mut(Queue_U* queue);



/*
 * create_node_by_new - Create a Node with a Given News Pointer
 * This function allocates memory for a new node and initializes it with the provided 'News' pointer.
 *
 * Parameters:
 *   News* news - Pointer to the 'News' item to be stored in the new node.
 *
 * Return:
 *   Node* - Pointer to the newly created node with the provided 'News' item.
 *           Returns NULL if memory allocation fails.
 */
Node* create_node_by_new(News* news);


/*
 * delete_queue_u - Delete a Queue of Nodes
 *
 * This function deletes a queue of 'Node' elements, freeing memory associated with each node
 * and destroying synchronization primitives used in the queue.
 *
 * Parameters:
 *   Queue_U* q - Pointer to the Queue_U structure to be deleted.
 */
void delete_queue_u(Queue_U* q);



#endif //EX3_QUEUE_U_H

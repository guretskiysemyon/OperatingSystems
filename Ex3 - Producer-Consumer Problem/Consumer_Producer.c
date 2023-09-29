/*
 * Author: Semyon Guretskiy
 * Date: 25/09/2023
 * Consumer-Producer Problem Implementation.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "News.h"
#include "Queue_B.h"
#include "Queue_U.h"


#define N_CO_EDIT 3

pthread_t *producers;
pthread_t *dispatcher;
pthread_t *co_editors;
pthread_t *screen_manager;


/*
 * Producer_arg - Structure for Producer Thread Arguments
 *
 * This structure is used to pass arguments to producer threads.
 */
typedef struct {
    Queue_B *queue; // Pointer to the bounded queue for news
    int n_news;     // Number of news to produce
    int index;      // Producer index
} Producer_arg;



/*
 * Co_Editors_Arg - Structure for Co-Editors Thread Arguments
 *
 * This structure is used to pass arguments to co-editors threads.
 * It contains pointers to a bounded queue 'q_b' and an unbounded queue 'q_u'.
 */
typedef struct {
    Queue_U *q_u;
    Queue_B *q_b;
} Co_Editors_Arg;

/*
 * Dispatcher_Arg - Structure for Dispatcher Thread Arguments
 *
 * This structure is used to pass arguments to the dispatcher thread.
 * It contains arrays of pointers to bounded queues 'q_b', unbounded queues 'q_edit',
 * and the number of producers 'num_prod'.
 */
typedef struct {
    Queue_B **q_b;
    Queue_U **q_edit;
    int num_prod;
} Dispatcher_Arg;


enum CO_EDITORS {
    DONE = -1,
    SPORT,
    NEWS,
    WEATHER
};


/*
 * Prod_Conf - Structure for Producer Configuration
 *
 * This structure is used to store configuration information for a producer.
 * It contains the producer's ID 'prod_id', the number of news to produce 'n_news',
 * and the queue size 'q_size' for the bounded queue where news is placed.
 */
typedef struct {
    int prod_id;
    int n_news;
    int q_size;
} Prod_Conf;


/*
 * Conf - Overall Configuration Structure
 *
 * This structure is used to store the overall configuration for the program.
 * It contains an array of producer configurations 'prodArg', the number of producers 'n_pr',
 * and the size of the shared memory queue 'sm_q_size'.
 */
typedef struct{
    Prod_Conf * prodArg;
    int n_pr;
    int sm_q_size;
}Conf;



/*
 * produce - Function for producing news articles and enqueuing them in a bounded queue.
 * 
 * Parameters:
 *  void *arg - A pointer to the Producer_arg structure containing producer arguments.
 * 
 * The produce function generates news articles based on random categories (SPORT, NEWS, or WEATHER)
 * and enqueues them into the given bounded queue. It continues producing until the specified
 * number of news articles (n_news) is reached.
 * 
 * Once all news articles are produced, a special "DONE" news article is enqueued to signal
 * the end of production.
 * 
 * Return:
 *  None
 */
void *produce(void *arg) {

    // Extract arguments
    Producer_arg *arguments = (Producer_arg *) arg;
    Queue_B *queue = arguments->queue;
    int n = arguments->n_news;
    int index = arguments->index;


    int randN;
    int sport = 0, news_counter = 0, weather = 0;
    News *news;

    // Produce news articles with random category.
    for (int i = 1; i <= n; i++) {
        randN = rand() % 3;
        switch (randN) {
            case SPORT:
                news = create_new(index, sport++, SPORT);
                break;
            case NEWS:
                news = create_new(index, news_counter++, NEWS);
                break;
            case WEATHER:
                news = create_new(index, weather++, WEATHER);
                break;
            default:
                break;
        }

        // Enqueue the generated news article
        enqueue_b_mut(queue, news);

    }

    // Enqueue a special "DONE" news article to signal the end of production
    news = create_new(DONE, DONE, DONE);
    enqueue_b_mut(queue, news);

    return NULL;
}


/*
 * consume - Function for consuming news articles from multiple bounded queues
 * and distributing them to corresponding editors.
 * The consume function continuously dequeues news articles from multiple bounded queues
 * (queueB) in a round-robin fashion. It checks the category of each news article and enqueues
 * it into the appropriate editor queue (q_editors) based on the category (SPORT, NEWS, or WEATHER).
 * The function continues consuming news articles until it encounters the "DONE" news article
 * from all producer queues, at which point it terminates.
 *  
 * Parameters:
 *  void *arg - A pointer to the Dispatcher_Arg structure containing dispatcher arguments.
 * 
 * Return:
 *  None
 */
void *consume(void *arg) {

    // Extract arguments
    Dispatcher_Arg *d = (Dispatcher_Arg *) arg;
    Queue_B **queueB = d->q_b;
    Queue_U **q_editors = d->q_edit;
    int n = d->num_prod;

    int is_consume = 1;
    int r = 0;
    int done_count = 0;

    News *news;
    while (is_consume) {
        // Dequeue a news article from a bounded queue in round-robin fashion
        news = try_dequeue_b_mut(queueB[r]);
        r = (r + 1) % n;
        if (!news)
            continue;
        Node *node = create_node_by_new(news);

        // Enqueue the news article into the corresponding editor queue based on category
        switch (news->category) {
            case SPORT:
                enqueue_u_mut(*(q_editors + SPORT), node);
                break;
            case NEWS:
                enqueue_u_mut(*(q_editors + NEWS), node);
                break;
            case WEATHER:
                enqueue_u_mut(*(q_editors + WEATHER), node);
                break;
            case DONE:
                ++done_count;
                free(news);
                free(node);
                if (done_count == n) {
                    is_consume = 0;
                }
                break;
            default:
                break;
        }
    }

    // Enqueue "DONE" news articles into each editor queue to signal completion
    enqueue_u_mut(*(q_editors + SPORT), create_node_by_value(DONE, DONE, DONE));
    enqueue_u_mut(*(q_editors + NEWS), create_node_by_value(DONE, DONE, DONE));
    enqueue_u_mut(*(q_editors + WEATHER), create_node_by_value(DONE, DONE, DONE));

    return NULL;
}


/*
 * co_edit - Function for cooperative editing of news articles.
 * The co_edit function dequeues news articles from a Queue_U (editor queue), performs
 * cooperative editing by enqueuing the news article into a Queue_B (bounded queue),
 * and introduces a delay for non-"DONE" news articles to simulate editing time.
 * 
 * The function continues co-editing news articles until it encounters the "DONE" news article,
 * at which point it terminates.
 * 
 * Parameters:
 *  void *arg - A pointer to the Co_Editors_Arg structure containing co-editing arguments.
 * 
 * Return:
 *  None
 */
void *co_edit(void *arg) {
    // Extract arguments
    Co_Editors_Arg *qs = (Co_Editors_Arg *) arg;
    Queue_B *queueB = qs->q_b;
    Queue_U *queueU = qs->q_u;

    int is_consume = 1;
    News *news;
    while (is_consume) {
        // Dequeue a news article from the editor queue (Queue_U)
        news = dequeue_u_mut(queueU);

        // Enqueue the news article into the bounded queue (Queue_B)
        enqueue_b_mut(queueB, news);

        // Introduce a delay (simulating editing time) for non-"DONE" news articles
        if (news->category != DONE)
            usleep(100000);
        else
            is_consume = 0;

    }

    return NULL;
}


/*
 * print_to_screen - Print a message to the screen based on news category, producer ID, and index.
 * 
 * Parameters:
 *  int prod_id - Producer ID
 *  int cat - News category (SPORT, NEWS, or WEATHER)
 *  int index - News index
 * 
 * Return:
 *  None
 */
void print_to_screen(int prod_id, int cat, int index){
    switch (cat) {
        case SPORT:
            printf("Producer %d SPORTS %d\n", prod_id, index);
            break;
        case NEWS:
            printf("Producer %d NEWS %d\n", prod_id, index);
            break;
        case WEATHER:
            printf("Producer %d WEATHER %d\n", prod_id, index);
            break;
        default:
            break;
    }
}



/*
 * screen_manage - Manage the printing of news messages to the screen.
 * 
 * Parameters:
 *  Queue_B *queueB - Bounded queue of news messages
 * 
 * This function dequeues news messages from the given bounded queue and prints them to the screen.
 * It keeps track of the number of "DONE" messages received to determine when to exit.
 * 
 * Return:
 *  None
 */
void *screen_manage(void *arg) {
    // Extract arguments
    Queue_B *queueB = (Queue_B *) arg;

    int is_consume = 1;
    int done_number = 0;   // Count till 3
    News *news;
    int i = 0;


    while (is_consume) {

        // Dequeue from screen manager queue
        news = dequeue_b_mut(queueB);

        // If it's not DONE message, print on the screen.
        if (news->category == DONE)
            done_number++;
        else {
            print_to_screen(news->producer, news->category, news->index);
            ++i;
        }
        free(news);

        // Every editor has finished, screen manager finishes too.
        if (done_number == 3) {
            printf("DONE\n");
            is_consume = 0;
        }

    }

    return NULL;
}


/*
 * read_conf - Read configuration data from a file and create a Conf structure.
 * 
 * Parameters:
 *  const char* path - Path to the configuration file.
 * 
 * Return:
 *  Conf* - Pointer to the Conf structure containing configuration data.
 *          Returns NULL on failure.
 */
Conf* read_conf(const char* path) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        exit(1);
    }

    Prod_Conf* arguments = NULL;
    int objectCount = 0;
    int maxObjects = 0;

    Prod_Conf temp_arg;

    // Read regular arguments
    while (fscanf(file, "%d", &temp_arg.prod_id) == 1
           && fscanf(file, "%d", &temp_arg.n_news) == 1
           && fscanf(file, "%d", &temp_arg.q_size) == 1){


        if (objectCount >= maxObjects) {
            maxObjects += 5;
            Prod_Conf* newObjects = realloc(arguments, maxObjects * sizeof(Prod_Conf));
            if (newObjects == NULL) {
                exit(1);
            }
            arguments = newObjects;
        }


        arguments[objectCount] = temp_arg;
        objectCount++;
    }

    if (!feof(file) || ferror(file))
        exit(1);

    fclose(file);



    Conf* conf = (Conf*) malloc(sizeof (Conf));
    conf->prodArg = arguments;
    conf->n_pr = objectCount;
    conf->sm_q_size = temp_arg.prod_id;

    return conf;
}




int main(int argc, char const *argv[]) {

    // Seed the random number generator with the current time
    srand(time(NULL));

    // Check for the correct number of command line arguments
    if (argc != 2){
        printf("Wrong number of arguments!");
        exit(1);
    }

    // Read the configuration data from the specified file
    Conf* conf = read_conf(argv[1]);

    // Get the number of producers
    int n_prod = conf->n_pr;

    // Validate that buffer size is not 0 for each producer
    for (int i = 0 ; i < n_prod; i++){
        if (conf->prodArg->q_size == 0){
            printf("Buffer cannot be size of 0\n");
            exit(1);
        }
    }

    // Create an array of Queue_B pointers for producer queues
    Queue_B**  queue_prods = (Queue_B**)malloc(n_prod* sizeof (Queue_B*));
    if (queue_prods == NULL)
        exit(1);

    // Create a shared memory queue
    Queue_B *queue_sm = create_queue_b(conf->sm_q_size);
    // Create queues for co-editors
    Queue_U *queues_editors[N_CO_EDIT] = {create_queue_u(), create_queue_u(), create_queue_u()};

    // Initialize Co-Editors arguments
    Co_Editors_Arg coEditorsArg[N_CO_EDIT];
    for (int i = 0; i < N_CO_EDIT; i++) {
        coEditorsArg[i].q_b = queue_sm;
        coEditorsArg[i].q_u = queues_editors[i];
    }


    Prod_Conf*  p;

    // Create an array of Producer_arg structures
    Producer_arg* pr_arg = (Producer_arg *) malloc(n_prod * sizeof(Producer_arg));
    if (pr_arg == NULL)
        exit(1);


    // Create producer queues and fill Producer_arg structures
    for (int i = 0; i < n_prod; i++) {
        p = &conf->prodArg[i];
        Queue_B *q = create_queue_b(p->q_size);
        pr_arg[i].queue = q;
        pr_arg[i].index = p->prod_id - 1;
        pr_arg[i].n_news = p->n_news;
        queue_prods[i] = q;
    }

    // Allocate space for threads.
    producers = (pthread_t *) malloc(n_prod * sizeof(pthread_t));
    if (producers == NULL)
        exit(1);

    dispatcher = (pthread_t *) malloc(sizeof(pthread_t));
    if (dispatcher == NULL)
        exit(1);

    co_editors = (pthread_t *) malloc(N_CO_EDIT * sizeof(pthread_t));
    if (co_editors == NULL)
        exit(1);

    screen_manager = (pthread_t *) malloc(sizeof(pthread_t));
    if (screen_manager == NULL)
        exit(1);


    // Create Dispatcher_Arg structure for the dispatcher thread
    Dispatcher_Arg dis;
    dis.q_b = queue_prods;
    dis.q_edit = queues_editors;
    dis.num_prod = n_prod;

    int err;
    // Create producer threads
    for (int i = 0; i < n_prod; i++) {
        err = pthread_create(producers + i, NULL, &produce, (void *) &pr_arg[i]);
        if (err != 0)
            exit(1);
    }

    // Create the dispatcher thread
    err = pthread_create(dispatcher, NULL, &consume, (void *) &dis);
    if (err != 0)
        exit(1);

    
    // Create co-editor threads
    for (int i = 0; i < N_CO_EDIT; i++) {
        err = pthread_create(co_editors + i, NULL, &co_edit, (void *) &coEditorsArg[i]);
        if (err != 0)
            exit(1);
    }

    // Create the screen manager thread
    err =  pthread_create(screen_manager, NULL, &screen_manage, (void *) queue_sm);
    if (err != 0)
        exit(1);


    // Wait for producer threads to finish
    for (int i = 0; i < n_prod; i++){
        pthread_join(*(producers + i), NULL);
    }

    // Wait for the dispatcher thread to finish
    pthread_join(*dispatcher, NULL);

    // Wait for co-editor threads to finish
    for (int i = 0; i < N_CO_EDIT; i++) {
        pthread_join(*(co_editors + i), NULL);
    }

    // Wait for the screen manager thread to finish
    pthread_join(*screen_manager, NULL);


    // Free allocated memory
    free(conf->prodArg);
    free(conf);
    free(queue_prods);

    free(queue_sm->news);
    free(queue_sm);

    for (int i = 0; i < N_CO_EDIT; i++) {
       delete_queue_u(queues_editors[i]);
    }

    for( int i =0; i < n_prod; i++){
        delete_queue_b(pr_arg[i].queue);
    }
    free(pr_arg);

    free(producers);
    free(dispatcher);
    free(co_editors);
    free(screen_manager);

    return 0;
}

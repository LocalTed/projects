#include "segel.h"
#include "request.h"
#include "queue.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

//  locks & vars:
pthread_cond_t Q_not_empty, Q_not_full, Qs_empty;
pthread_mutex_t wait_Q_lock, run_Q_lock;
pthread_t* threads;

// HW3: Parse the new arguments too
void getargs(int *port, int *thread_num, int *queue_size, char **schedalg, int argc, char *argv[])
{
    if (argc < 2 || argc != 5)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *thread_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    *schedalg = argv[4];
}

void do_work(Node *queue_args[])
{
    Node *wait_first = (Node *)queue_args[0];
    Node *wait_last = (Node *)queue_args[1];
    Node *run_first = (Node *)queue_args[2];
    Node *run_last = (Node *)queue_args[3];
    
    int thread_id = 0;
    while(threads[thread_id] != pthread_self())
    {
        thread_id++;
    }

    threads_stats stats = (threads_stats)malloc(sizeof(threads_stats));
    stats->id = thread_id;
    stats->dynm_req = 0;
    stats->stat_req = 0;
    stats->total_req = 0;
    struct timeval dispatch;

    while (1)
    {
        // remove from wait queue
        pthread_mutex_lock(&wait_Q_lock); // wait lock

        while (wait_first->next == wait_last)
        {
            pthread_cond_wait(&Q_not_empty, &wait_Q_lock);
        }
        gettimeofday(&dispatch, NULL);
        Node *curr = pop_first(wait_first, wait_last);
        pthread_mutex_unlock(&wait_Q_lock); // wait unlock

        // insert to run queue
        pthread_mutex_lock(&run_Q_lock); // run lock
        push_node(run_first, run_last, curr);
        pthread_mutex_unlock(&run_Q_lock); // run unlock

        // do work
        stats->total_req++;
        timersub(&dispatch, &curr->arrival, &dispatch);
        requestHandle(curr->data, curr->arrival, dispatch, stats);
        Close(curr->data);

        // remove from run queue
        pthread_mutex_lock(&run_Q_lock); // run lock
        pop_node(run_first, run_last, curr);
        pthread_cond_signal(&Q_not_full);
        pthread_mutex_unlock(&run_Q_lock); // run unlock

        // signal main if Qs are empty
        pthread_mutex_lock(&wait_Q_lock); // wait unlock
        pthread_mutex_lock(&run_Q_lock);  // run unlock

        if (wait_first->next == wait_last && run_first->next == run_last)
        {
            pthread_cond_signal(&Qs_empty);
        }

        pthread_mutex_unlock(&wait_Q_lock); // wait unlock
        pthread_mutex_unlock(&run_Q_lock);  // run unlock
    }
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, thread_num, queue_size;
    char *schedalg;
    struct sockaddr_in clientaddr;
    struct timeval arrival;

    getargs(&port, &thread_num, &queue_size, &schedalg, argc, argv);

    pthread_cond_init(&Q_not_empty, NULL);
    pthread_cond_init(&Q_not_full, NULL);
    pthread_cond_init(&Qs_empty, NULL);

    pthread_mutex_init(&wait_Q_lock, NULL);
    pthread_mutex_init(&run_Q_lock, NULL);

    // TODO: create QUEUE(S)
    Node *wait_first = init_queue();
    Node *wait_last = wait_first->next;
    Node *run_first = init_queue();
    Node *run_last = run_first->next;
    Node *queue_args[] = {wait_first, wait_last, run_first, run_last};

    threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);
    for (unsigned int i = 0; i < thread_num; i++)
    {
        pthread_create(&threads[i], NULL, (void *)do_work, queue_args);
    }

    listenfd = Open_listenfd(port);
    while (1)
    {

        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
        gettimeofday(&arrival, NULL);

        pthread_mutex_lock(&wait_Q_lock);
        pthread_mutex_lock(&run_Q_lock);
        if (get_size(wait_first) + get_size(run_first) == queue_size)
        {
            pthread_mutex_unlock(&run_Q_lock);

            if (strcmp("block", schedalg) == 0)
            {
                pthread_cond_wait(&Q_not_full, &wait_Q_lock);
            }
            else if (strcmp("dt", schedalg) == 0) // drop_tail
            {
                Close(connfd);
                pthread_mutex_unlock(&wait_Q_lock);
                continue;
            }
            else if (strcmp("dh", schedalg) == 0) // drop_head
            {
                if(!get_size(wait_first)){
                    Close(connfd);
                    pthread_mutex_unlock(&wait_Q_lock);
                    continue;
                }
                Close(pop_first(wait_first, wait_last)->data);
            }
            else if (strcmp("bf", schedalg) == 0) // block_flush
            {
                pthread_cond_wait(&Qs_empty, &wait_Q_lock);
                Close(connfd);
                pthread_mutex_unlock(&wait_Q_lock);
                continue;
            }
            else if (strcmp("random", schedalg) == 0) // drop_random
            {
                int size = get_size(wait_first);
                for (int i = 0; i < (size / 2) + (size % 2); i++)
                {
                    Close(pop_from(wait_first, wait_last, rand() % (size - i)));
                }
                if(size == 0){
                    Close(connfd);
                    pthread_mutex_unlock(&wait_Q_lock);
                    continue;
                }
            }
            else
            {
                // invalid!!!
                // TODO: check what in this case
                unix_error("error:: invalid arguments\n");
                exit(-1);
            }
        }
        else
        {

            pthread_mutex_unlock(&run_Q_lock);
        }

        push_data(wait_first, wait_last, connfd, arrival);
        pthread_cond_signal(&Q_not_empty);
        pthread_mutex_unlock(&wait_Q_lock);
    }
}

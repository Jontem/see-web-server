#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "connectionqueue.h"

struct QueuedConnection *next_in_line;
pthread_mutex_t queue_mutex;

void queue_connection(int socket_desc)
{

    pthread_mutex_lock(&queue_mutex);

    if (next_in_line == NULL)
    {

        struct QueuedConnection *temp = malloc(sizeof(struct QueuedConnection));
        temp->client_sock = socket_desc;
        temp->next = NULL;
        next_in_line = temp;
        pthread_mutex_unlock(&queue_mutex);
        return;
    }

    struct QueuedConnection *last = next_in_line;
    while (last->next != NULL)
    {
        last = last->next;
    }

    struct QueuedConnection *temp = malloc(sizeof(struct QueuedConnection));
    temp->client_sock = socket_desc;
    temp->next = NULL;
    last->next = temp;
    pthread_mutex_unlock(&queue_mutex);
}

int pop_connection()
{
    if (next_in_line == NULL)
    {
        return 0;
    }

    pthread_mutex_lock(&queue_mutex);
    struct QueuedConnection *popped = next_in_line;
    next_in_line = popped->next;

    int socket_desc = popped->client_sock;
    free(popped);
    pthread_mutex_unlock(&queue_mutex);
    return socket_desc;
}

void printQueuedConnections()
{

    if (next_in_line == NULL)
    {
        puts("No connections in queue");
        return;
    }

    int count = 0;

    struct QueuedConnection *next = next_in_line;
    while (next != NULL)
    {
        count++;
        next = next->next;
    }

    printf("Connections in queue: %d\n", count);
}
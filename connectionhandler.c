#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include "connectionhandler.h"
#include "connectionqueue.h"

const char HEADER_SEP[] = "\r\n\r\n";

void handle_connection(int worker_id, int client_sock);
int write_response(int client_sock, char body[]);
void *connection_worker(void *args);

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_t *workers;

void create_workers(int num_workers)
{
    workers = malloc(sizeof(pthread_t) * num_workers);
    for (int i = 0; i < num_workers; i++)
    {
        char worker_name[20] = "\0";
        int *worker_id = malloc(sizeof(int));
        *worker_id = i;
        pthread_create(&workers[i], NULL, &connection_worker, worker_id);
    }
}

void signal_connection()
{
    pthread_cond_signal(&cond1);
}

void *connection_worker(void *args)
{
    int worker_id = (*(int *)args);
    printf("Worker: %d started\n", worker_id);
    while (1)
    {
        int client_sock = pop_connection();
        if (client_sock > 0)
        {
            handle_connection(worker_id, client_sock);
            continue;
        }

        printf("Worker: %d waiting for signal\n", worker_id);

        // In our case we dont use a predicate worth locking?
        // https://stackoverflow.com/a/14925150/3534549
        // pthread_mutex_lock(&lock);
        pthread_cond_wait(&cond1, &lock);
        // pthread_mutex_unlock(&lock);
        printf("Worker: %d woke up\n", worker_id);
    }
}
void handle_connection(int worker_id, int client_sock)
{
    int read_size;
    char client_message[2000] = "\0", request_headers[1000] = "\0";

    //Receive a message from client
    while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
    {
        if (read_size == 0)
        {
            puts("Client disconnected ");
        }
        else if (read_size == -1)
        {
            perror("recv failed ");
        }

        // Need to put a null byte at the end.
        client_message[read_size] = 0;
        char *end_header_pointer = strstr(client_message, HEADER_SEP);
        if (end_header_pointer == NULL)
        {
            strcat(request_headers, client_message);
        }
        else
        {
            size_t str_length = end_header_pointer - client_message;
            strncat(request_headers, client_message, str_length + strlen(HEADER_SEP));
        }

        if (strstr(request_headers, HEADER_SEP) == 0)
        {
            puts("Waiting for more data\n");
            continue;
        }

        write_response(client_sock, "hello body");
        printf("Worker%d: Connection handled. Socket closed\n", worker_id);
        break;
    }
}

int write_response(int client_sock, char body[])
{
    char headers[2000] = "";
    char new_line[] = "\r\n";

    char content_size[50];
    sprintf(content_size, "%lu", sizeof(char) * strlen(body));
    char content_length[2000] = "Content-Length: ";
    strcat(content_length, content_size);
    strcat(content_length, "\r\n");

    strcat(headers, "HTTP/1.1 200 OK\r\n");
    strcat(headers, "Content-Type: text/plain\r\n");
    strcat(headers, content_length);

    write(client_sock, headers, strlen(headers));
    write(client_sock, new_line, strlen(new_line));
    write(client_sock, body, strlen(body));
    write(client_sock, new_line, strlen(new_line));
    close(client_sock);

    return 1;
}
#ifndef CONNECTIONQUEUE_HEADER_FILE
#define CONNECTIONQUEUE_HEADER_FILE

struct QueuedConnection
{
    int client_sock;
    struct QueuedConnection *next;
};

void queue_connection(int socket_desc);
int pop_connection();
void printQueuedConnections();

#endif

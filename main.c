#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "connectionqueue.h"
#include "connectionhandler.h"

int socket_desc;

void cleanup()
{
    close(socket_desc);
    puts("\nShutting down...");
    exit(0);
}

void intHandler(int dummy)
{
    puts("\nCTRL-C received...");
    cleanup();
}

int main(int argc, char *argv[])
{
    signal(SIGINT, intHandler);

    int socket_desc, client_sock, c;
    struct sockaddr_in server, client;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket ");
    }
    puts("Socket created ");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // Fix for address already in use error
    int yes = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("bind failed.Error ");
        return 1;
    }

    puts("bind done ");

    listen(socket_desc, 3);

    create_workers(3);

    puts("Waiting for incoming connections...");
    int con_count = 0;
    while (1)
    {
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock < 0)
        {
            perror("accept failed ");
            return 1;
        }

        queue_connection(client_sock);
        signal_connection();
        con_count++;
    }

    return 0;
}
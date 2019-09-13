#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <signal.h>
#include <stdlib.h>

int socket_desc;

void string2hexString(char *input, char *output)
{
    int loop;
    int i;

    i = 0;
    loop = 0;

    while (input[loop] != '\0')
    {
        sprintf((char *)(output + i), "%02X", input[loop]);
        loop += 1;
        i += 2;
    }
    //insert NULL at the end of the output string
    output[i++] = '\0';
}

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

const char header_sep[] = "\r\n\r\n";

int main(int argc, char *argv[])
{
    signal(SIGINT, intHandler);

    int socket_desc, client_sock, c, read_size;
    struct sockaddr_in server, client;
    char client_message[2000];

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

    puts("Waiting for incoming connections...");

    while (1)
    {
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
        if (client_sock < 0)
        {
            perror("accept failed ");
            return 1;
        }
        puts("Connection accepted ");

        char request_headers[1000] = "\0";
        //Receive a message from client
        while ((read_size = recv(client_sock, client_message, 2000, 0)) > 0)
        {
            // Need to put a null byte at the end.
            client_message[read_size] = 0;
            char *end_header_pointer = strstr(client_message, header_sep);
            if (end_header_pointer == NULL)
            {
                strcat(request_headers, client_message);
            }
            else
            {
                size_t str_length = end_header_pointer - client_message;
                strncat(request_headers, client_message, str_length + strlen(header_sep));
            }

            if (strstr(request_headers, header_sep) == 0)
            {
                puts("Waiting for more data\n");
                continue;
            }

            puts("Got complete headers\n");
            puts("Headers: \n");
            puts(request_headers);
            puts("\n");

            write_response(client_sock, "hello body");
            break;
        }

        if (read_size == 0)
        {
            puts("Client disconnected ");
            fflush(stdout);
        }
        else if (read_size == -1)
        {
            perror("recv failed ");
        }
    }

    return 0;
}
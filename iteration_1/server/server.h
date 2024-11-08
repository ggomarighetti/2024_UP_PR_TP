#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080

typedef struct {
    int client_socket;
} thread_data_t;

void* handle_connection(void* arg);
int setup_server(int port);
void start_server(int server_socket);

#endif
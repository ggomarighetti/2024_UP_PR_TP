#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 8080
#define DEFAULT_HOST "127.0.0.1"

int start_client(const char* host, int port);
int send_ping(int client_socket);
int receive_pong(int client_socket);

#endif
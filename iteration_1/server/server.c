#include "server.h"

void *handle_connection(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    int client_socket = data->client_socket;
    char buffer[BUFFER_SIZE];

    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Solicitud recibida: %s\n", buffer);

        if (strncmp(buffer, "PING", 4) == 0)
        {
            const char *response = "PONG";
            send(client_socket, response, strlen(response), 0);
            printf("Respuesta enviada: %s\n", response);
        }
    }

    close(client_socket);
    free(data);

    return NULL;
}

int setup_server(int port)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Error creando socket");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error en bind");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 10) < 0)
    {
        perror("Error en listen");
        close(server_socket);
        return -1;
    }

    return server_socket;
}

void start_server(int server_socket)
{
    printf("Servidor iniciado y escuchando conexiones...\n");

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0)
        {
            perror("Error en accept");
            continue;
        }

        thread_data_t *thread_data = malloc(sizeof(thread_data_t));
        thread_data->client_socket = client_socket;

        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (pthread_create(&thread, &attr, handle_connection, thread_data) != 0)
        {
            perror("Error creando thread");
            free(thread_data);
            close(client_socket);
        }

        pthread_attr_destroy(&attr);
    }
}

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;

    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    int server_socket = setup_server(port);
    if (server_socket < 0)
    {
        return 1;
    }

    start_server(server_socket);

    close(server_socket);

    return 0;
}
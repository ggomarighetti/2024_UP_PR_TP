#include "server.h"

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
    printf("Servidor HTTP iniciado en modo non-blocking y escuchando conexiones...\n");

    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    fd_set read_fds, write_fds, except_fds;
    int max_fd = server_socket;

    int client_sockets[FD_SETSIZE];
    for (int i = 0; i < FD_SETSIZE; i++)
    {
        client_sockets[i] = -1;
    }

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&except_fds);

        FD_SET(server_socket, &read_fds);
        FD_SET(server_socket, &except_fds);

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            int client_socket = client_sockets[i];
            if (client_socket != -1)
            {
                FD_SET(client_socket, &read_fds);
                FD_SET(client_socket, &except_fds);
                if (client_socket > max_fd)
                {
                    max_fd = client_socket;
                }
            }
        }

        int activity = select(max_fd + 1, &read_fds, &write_fds, &except_fds, NULL);
        if (activity < 0 && errno != EINTR)
        {
            perror("Error en select");
            break;
        }

        if (FD_ISSET(server_socket, &read_fds))
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

            if (new_socket >= 0)
            {
                printf("Nueva conexión HTTP aceptada\n");
                fcntl(new_socket, F_SETFL, O_NONBLOCK);

                for (int i = 0; i < FD_SETSIZE; i++)
                {
                    if (client_sockets[i] == -1)
                    {
                        client_sockets[i] = new_socket;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            int client_socket = client_sockets[i];
            if (client_socket == -1)
                continue;

            if (FD_ISSET(client_socket, &except_fds))
            {
                close(client_socket);
                client_sockets[i] = -1;
                continue;
            }

            if (FD_ISSET(client_socket, &read_fds))
            {
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

                if (bytes_read > 0)
                {
                    printf(buffer);
                    buffer[bytes_read] = '\0';

                    if (strstr(buffer, "GET /imagen.jpg") != NULL)
                    {
                        FILE *file = fopen("imagen.jpg", "rb");
                        if (file != NULL)
                        {
                            const char *header = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n";
                            send(client_socket, header, strlen(header), 0);

                            char file_buffer[8192];
                            size_t bytes;
                            while ((bytes = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0)
                            {
                                send(client_socket, file_buffer, bytes, 0);
                            }
                            fclose(file);
                        }
                        else
                        {
                            const char *not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
                            send(client_socket, not_found, strlen(not_found), 0);
                        }
                    }
                    else
                    {
                        const char *bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
                        send(client_socket, bad_request, strlen(bad_request), 0);
                    }
                }

                close(client_socket);
                client_sockets[i] = -1;
            }
        }
    }

    for (int i = 0; i < FD_SETSIZE; i++)
    {
        if (client_sockets[i] != -1)
        {
            close(client_sockets[i]);
        }
    }
    close(server_socket);
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
#include "client.h"

int start_client(const char* host, int port) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creando socket");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        perror("Error en dirección IP");
        close(client_socket);
        return -1;
    }
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error conectando");
        close(client_socket);
        return -1;
    }
    
    return client_socket;
}

int send_ping(int client_socket) {
    const char* message = "PING";
    if (send(client_socket, message, strlen(message), 0) < 0) {
        perror("Error enviando mensaje");
        return -1;
    }
    
    printf("Solicitud enviada: %s\n", message);
    return 0;
}

int receive_pong(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Respuesta recibida: %s\n", buffer);
        return 0;
    }
    return -1;
}

int main(int argc, char* argv[]) {
    const char* host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    
    if (argc > 1) host = argv[1];
    if (argc > 2) port = atoi(argv[2]);
    
    int client_socket = start_client(host, port);
    if (client_socket < 0) {
        return 1;
    }

    if (send_ping(client_socket) < 0 || receive_pong(client_socket) < 0) {
        close(client_socket);
        return 1;
    }
    
    close(client_socket);
    return 0;
}
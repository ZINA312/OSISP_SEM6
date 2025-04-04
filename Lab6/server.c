#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int sockfd;
    char username[32];
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to_all(char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].sockfd != sender_sock) {
            send(clients[i].sockfd, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_user(char *username, char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (strcmp(clients[i].username, username) == 0) {
            send(clients[i].sockfd, message, strlen(message), 0);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int sockfd = *((int *)arg);
    free(arg);
    char buffer[BUFFER_SIZE];
    char username[32];
    int read_size;

    if ((read_size = recv(sockfd, username, sizeof(username), 0)) > 0) {
        username[read_size] = '\0';

        pthread_mutex_lock(&clients_mutex);
        int username_taken = 0;
        for (int i = 0; i < num_clients; i++) {
            if (strcmp(clients[i].username, username) == 0) {
                username_taken = 1;
                break;
            }
        }
        if (username_taken) {
            send(sockfd, "Username taken. Disconnecting...", 32, 0);
            close(sockfd);
            pthread_mutex_unlock(&clients_mutex);
            pthread_exit(NULL);
        } else {
            clients[num_clients].sockfd = sockfd;
            strcpy(clients[num_clients].username, username);
            num_clients++;
            pthread_mutex_unlock(&clients_mutex);

            sprintf(buffer, "%s joined the chat!", username);
            send_to_all(buffer, sockfd);
        }

        while ((read_size = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
            buffer[read_size] = '\0';
            if (strncmp(buffer, "/", 1) == 0) {
                char *recipient = strtok(buffer + 1, " ");
                char *message = strtok(NULL, "");
                if (recipient && message) {
                    sprintf(buffer, "[PM from %s] %s", username, message);
                    send_to_user(recipient, buffer, sockfd);
                }
            } else {
                char formatted_buffer[BUFFER_SIZE * 2];
                snprintf(formatted_buffer, sizeof(formatted_buffer), "%s: %s", username, buffer);
                send_to_all(formatted_buffer, sockfd);
            }
        }
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].sockfd == sockfd) {
            clients[i] = clients[num_clients - 1];
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close(sockfd);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    printf("Server listening on port %s...\n", argv[1]);

    while ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len))) {
        printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int *new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)new_sock);
    }

    close(server_sock);
    return 0;
}
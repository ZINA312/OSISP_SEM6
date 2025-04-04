#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int sockfd;
char username[32];

void *receive_handler(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int read_size = recv(sockfd, buffer, sizeof(buffer), 0);
        if (read_size > 0) {
            buffer[read_size] = '\0';
            printf("%s\n", buffer);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server IP> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    send(sockfd, username, strlen(username) + 1, 0);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_handler, NULL);

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strncmp(buffer, "/quit", 5) == 0) {
            break;
        }
        send(sockfd, buffer, strlen(buffer), 0);
    }

    close(sockfd);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 2048

typedef struct {
    int client_id;
    int shop;
} Message;

void error_handler(char *msg) {
    perror(msg);
    exit(1);
}

struct sockaddr_in listener_addr;
int server_sock;

void send_to_listener(char buffer[]) {
    int l_len = sizeof(listener_addr);
    if (sendto(server_sock, buffer, BUFSIZE, 0, (struct sockaddr *)&listener_addr, l_len) < 0) {
        error_handler("Failed to send packet.\n");
    }
    memset(buffer, 0, BUFSIZE);
}

int main(int argc, char **argv) {
    int client_sock, port_num, client_len, recv_len, client_shop1_port, client_shop2_port;

    int client_shop1_sock, client_shop2_sock, server_len_1, server_len_2;

    char buffer[BUFSIZE];
    struct sockaddr_in server_addr, client_addr, client_shop1_addr, client_shop2_addr;

    if (argc != 8) {
        printf("Usage: %s <port> <store1_ip> <store1_port> <store2_ip> <store2_port> <listener_ip> <listener_port>\n", argv[0]);
        exit(1);
    }

    port_num = atoi(argv[1]);

    int port_listener = atoi(argv[7]);

    listener_addr.sin_family = AF_INET;
    listener_addr.sin_addr.s_addr = inet_addr(argv[6]);
    listener_addr.sin_port = htons(port_listener);

    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        error_handler("Failed to create socket.");
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port_num);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error_handler("Failed to bind socket.");
    }

    client_shop1_port = atoi(argv[3]);

    client_shop2_port = atoi(argv[5]);

    // Инициализация первого сервера-продавца
    client_shop1_addr.sin_family = AF_INET;
    client_shop1_addr.sin_addr.s_addr = inet_addr(argv[2]);
    client_shop1_addr.sin_port = htons(client_shop1_port);

    // Инициализация второго сервера-продовца
    client_shop2_addr.sin_family = AF_INET;
    client_shop2_addr.sin_addr.s_addr = inet_addr(argv[4]);
    client_shop2_addr.sin_port = htons(client_shop2_port);

    server_len_1 = sizeof(client_shop1_addr);
    server_len_2 = sizeof(client_shop2_addr);
    while (1) {
        memset(buffer, 0, BUFSIZE);
        client_len = sizeof(client_addr);
        Message message;

        if ((recv_len = recvfrom(server_sock, (void*)&message, sizeof(message), 0, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            error_handler("Failed to receive packet.");
        }
        
        // Код для обработки списка покупок и вывода информации в терминал
        printf("Received packet from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Message received from id: %d\n", message.client_id);
        printf("Shopping list: %d\n", message.shop);
        printf("Sending shopping list to seller...\n");

        sprintf(buffer, "From server: Received packet from %s:%d\nMessage received from id: %d\nShopping list: %d\nSending shopping list to seller...\n", 
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message.client_id, message.shop);
        send_to_listener(buffer);
        memset(buffer, 0, BUFSIZE);

        // Отправляем первому продавцу
        if (message.shop == 1 || message.shop == 3) {
            if (sendto(server_sock, (const void*)&message, sizeof(message), 0, (struct sockaddr *)&client_shop1_addr, server_len_1) < 0) {
                error_handler("Failed to send packet.\n");
            }
        }
        
        // Отправляем второмцу продавцу
        if (message.shop == 2 || message.shop == 4) {
            if (sendto(server_sock, (const void*)&message, sizeof(message), 0, (struct sockaddr *)&client_shop2_addr, server_len_2) < 0) {
                error_handler("Failed to send packet.\n");
            }
        }
    }

    return 0;
}
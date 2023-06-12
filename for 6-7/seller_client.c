#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 2048

void error_handler(char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    int client_id;
    int shop;
} Message;

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
    int client_sock, port_num, client_len, recv_len;
    char buffer[BUFSIZE];
    struct sockaddr_in server_addr, client_addr;

    if (argc != 4) {
        printf("Usage: %s <port> <listener_ip> <listener_port>\n", argv[0]);
        exit(1);
    }

    port_num = atoi(argv[1]);
    int listener_port = atoi(argv[3]);

    listener_addr.sin_family = AF_INET;
    listener_addr.sin_addr.s_addr = inet_addr(argv[2]);
    listener_addr.sin_port = htons(listener_port);

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

    int number;
    while (1) {
        memset(buffer, 0, BUFSIZE);
        client_len = sizeof(client_addr);
        Message message;
        if ((recv_len = recvfrom(server_sock, (void*)&message, sizeof(message), 0, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            error_handler("Failed to receive packet.");
        }


        // Код для обработки списка покупок и вывода информации в терминал
        printf("Received packet from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("Message received from server. Client id: %d\n", message.client_id);
        printf("Shopping list: %d\n", message.shop);
        printf("Processing shop...\n");

        sprintf(buffer, "From seller: Received packet from %s:%d\nMessage received from server. Client id: %d\nShopping list: %d\n", 
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), message.client_id, message.shop);
        send_to_listener(buffer);
        memset(buffer, 0, BUFSIZE);

        sleep(2);
        printf("Shop processed!\n");
    }

    return 0;
}

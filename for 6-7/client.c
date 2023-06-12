#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#define BUFSIZE 2048

void error_handler(char *msg) {
    perror(msg);
     exit(1);
}

struct client
{
    int client_id;
    int* shopping_list;
};

typedef struct 
{
    int client_id;
    int shop;
} Message;

struct sockaddr_in server_addr, listener_addr;
int sock;

int shopping_lists[3][4] = {
    {1, 2, 3, 4},
    {1, 2, 1, 2},
    {3, 4, 3, 4}
};

void send_to_listener(char buffer[]) {
    int l_len = sizeof(listener_addr);
    if (sendto(sock, buffer, BUFSIZE, 0, (struct sockaddr *)&listener_addr, l_len) < 0) {
        error_handler("Failed to send packet.\n");
    }
    memset(buffer, 0, BUFSIZE);
}


void client_process(struct client cl)
{
    int server_len, send_len;
    char int_buffer[sizeof(int)];
    char buffer[BUFSIZE];
    server_len = sizeof(server_addr);

    // Отправляем список покупок на сервера
    memset(buffer, 0, BUFSIZE);
    printf("Client now going to the store!\n");
    sprintf(buffer, "From client: Client %d now going to the store!\n", cl.client_id);
    send_to_listener(buffer);
    memset(buffer, 0, BUFSIZE);

    for (int i = 0; i < 4; i++) {
        Message message;
        message.client_id = cl.client_id;
        message.shop = cl.shopping_list[i];
        if (sendto(sock, (const void*)&message, sizeof(message), 0, (struct sockaddr *)&server_addr, server_len) < 0) {
            error_handler("Failed to send packet.\n");
        }

        printf("Sent shopping list to server.\n");
        sprintf(buffer, "From client: Cliend %d sent shopping list to server.", cl.client_id);
        send_to_listener(buffer);
        memset(buffer, 0, BUFSIZE);
        sleep(cl.client_id * 2);
    }
    printf("%d bought all! Thanks!\n", cl.client_id);
    sprintf(buffer, "From client: %d bought all! Thanks!\n", cl.client_id);
    send_to_listener(buffer);
    memset(buffer, 0, BUFSIZE);
}

int creating_clients(int n) {
    if (n == 0) {
        return 0;
    }
    if (fork() == 0) {
        struct client cl;
        cl.shopping_list = shopping_lists[n - 1];
        cl.client_id = n;
        client_process(cl);
        return 0;
    }
    return creating_clients(n - 1);
}

int main(int argc, char **argv) {
    // Данные ждя отправки на сервер
    int port_num_1, server_len_1, port_listener;

    if (argc != 5) {
        printf("Usage: %s <server_ip_1> <port_1> <listener_ip> <listener_port>\n", argv[0]);
        exit(1);
    }

    port_num_1 = atoi(argv[2]);

    port_listener = atoi(argv[4]);

    memset(&server_addr, 0, sizeof(server_addr));
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        error_handler("Failed to create socket.\n");
    }
    // Инициализация адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(port_num_1);

    // Инициализация адреса сервера общего вывода информации
    listener_addr.sin_family = AF_INET;
    listener_addr.sin_addr.s_addr = inet_addr(argv[3]);
    listener_addr.sin_port = htons(port_listener);

    creating_clients(3);
}
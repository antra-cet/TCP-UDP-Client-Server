#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "clientHelper.h"

/*
    Used the 8-th laboratory to implement the TCP clients
*/

void client_usage(char *file)
{
    fprintf(stderr, "Usage: %s server_address server_port\n", file);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, n, ret;
    char buffer[BUFLEN];
    fd_set read_fds;
    fd_set tmp_fds;

    setvbuf(stdout, NULL, _IONBF, BUFLEN);

    // Error if the command is incorrect
    if (argc < 4) {
        client_usage(argv[0]);
    }

    // Set the file descriptors on zero
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Identifying the port given
    int port = atoi(argv[3]);
    DIE(port == 0, "port");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    ret = inet_aton(argv[1], &serv_addr.sin_addr);
    DIE(ret == 0, "inet_aton");

    int neagle = 1;
    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &neagle, sizeof(int));
    DIE(ret < 0, "setsockopt");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "connect");

    strcpy(buffer, argv[1]);
    ret = send(sockfd, buffer, strlen(buffer), 0);
    DIE(ret < 0, "send");

    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);

    struct client_tcp client;
    int ok = 0;
    client.id = atoll(argv[1]);

    // Adding the client in the database
    for (int i = 0; i < clients_database.tcp_size; i++) {
        if (client.id == clients_database.tcp_client[i].id) {
            ok = 1;
            break;
        }
    }

    if (ok == 0) {
        clients_database.tcp_size++;
        clients_database.tcp_client[clients_database.tcp_size] = client;
    }

    while (1) {
        tmp_fds = read_fds;
        ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        // stdin read
        if (FD_ISSET(0, &tmp_fds)) {
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);

            // If exit command is given, then stop the server and close
            // the socket
            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            }

            // If subscribe command is given
            if (strncmp(buffer, "subscribe", 9) == 0) {
                ret = send(sockfd, buffer, strlen(buffer), 0);
                DIE(ret < 0, "send");

                printf("Subscribed to topic.\n");
                continue;
            }

            // If unsubscribe command is given
            if (strncmp(buffer, "unsubscribe", 11) == 0) {
                ret = send(sockfd, buffer, strlen(buffer), 0);
                DIE(ret < 0, "send");

                printf("Unsubscribed from topic.\n");
                continue;
            }

            n = send(sockfd, buffer, strlen(buffer), 0);
            DIE(n < 0, "send");
        }

        // server read
        if (FD_ISSET(sockfd, &tmp_fds)) {
            struct udp_message m;
            memset(&m, 0, sizeof(struct udp_message));

            char command[MAX_LEN];
            strcpy(command, m.payload);
            
            ret = recv(sockfd, &m, sizeof(struct udp_message), 0);
            DIE(ret < 0, "recieve server");

            // If  the command is exit
            if (strncmp(command, "exit", 4) == 0) {
                close(sockfd);
                break;
            }

            if (strcmp(m.data_type, "INT") == 0) {
                int message = ntohl(*(uint32_t *) (m.payload + 1));
                if (m.payload[0] == 1) {
                    message *= -1;
                }
                char final_message[MAX_LEN];
                snprintf(final_message, MAX_LEN, "%d", message);

                printf("%lld:%d - %s - INT - %s\n", m.ip, m.port, m.top.name, final_message);
            } else {
                if (strcmp(m.data_type, "SHORT_REAL") == 0) {
                    unsigned short message = ntohl(*(uint32_t *)(m.payload + 1));
                    char final_message[MAX_LEN];
                    snprintf(final_message, MAX_LEN, "%u", message);

                    char l1, l2;
                    int size_m = strlen(final_message);
                    l2 = final_message[size_m - 2];
                    l1 = final_message[size_m - 1];

                    final_message[size_m - 2] = '.';
                    final_message[size_m - 1] = l2;
                    final_message[size_m] = l1;
                    final_message[size_m + 1] = '\0';

                    printf("%lld:%d - %s - SHORT_REAL - %s\n", m.ip, m.port, m.top.name, final_message);
                } else {
                    if (strcmp(m.data_type, "FLOAT") == 0) {
                        long message = ntohl(*(uint32_t *)(m.payload + 1));
                        char final_message[MAX_LEN];
                        printf("%lld:%d - %s - FLOAT - %s\n", m.ip, m.port, m.top.name, final_message);
                    }
                }
            }

            printf("%s", buffer);
        }
    }

    close(sockfd);

    return 0;
}

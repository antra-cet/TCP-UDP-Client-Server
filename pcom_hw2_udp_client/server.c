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

int main(int argc, char *argv[]) {
    // Error if the command is incorrect
    if (argc < 2) {
        fprintf(stderr, "Usage: %s server_port\n", argv[0]);
        exit(0);
    }

    setvbuf(stdout, NULL, _IONBF, BUFLEN);

    // Retrieving the port
    int port = atoi(argv[1]);
    DIE(port == 0, "port");

    fd_set read_fds;
    fd_set tmp_fds;
    char buffer[BUFLEN];

    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    int socktcp = socket(AF_INET, SOCK_STREAM, 0);
    DIE(socktcp < 0, "socket");

    int sockudp = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(sockudp < 0, "socket udp");

    int optionValue = 1;
    setsockopt(socktcp, IPPROTO_TCP, TCP_NODELAY, &optionValue, sizeof(int));

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((uint16_t) port);
    int ret = inet_aton(argv[1], &serv_addr.sin_addr);
    DIE(ret == 0, "inet_aton");

    bind(sockudp, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));

    FD_SET(socktcp, &read_fds);
    FD_SET(sockudp, &read_fds);
    FD_SET(0, &read_fds);

    // TODO INITIALIZE CLIENTS_DATABASE SIZES AND SHIT
    clients_database.tcp_size = 0;
    clients_database.udp_size = 0;

    while(1) {
        tmp_fds = read_fds;
        int maxSocket;
        if (socktcp > sockudp) {
            maxSocket = socktcp;
        } else {
            maxSocket = sockudp;
        }

        int n = select(maxSocket + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(n == -1, "select");

        // stdin read
        if (FD_ISSET(0, &tmp_fds)) {
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);

            if (strcmp(buffer, "exit\n") == 0) {
                for  (int i = 0; i < clients_database.tcp_size;  i++) {
                    memset(buffer, 0, BUFLEN);
                    strcpy(buffer, "CLIENT_EXIT");
                    send(clients_database.tcp_client[i].id, buffer, strlen(buffer), 0);
                    close(clients_database.tcp_client[i].id);
                }

                break;
            }
        }
    }
}
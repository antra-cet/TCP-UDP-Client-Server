#ifndef _CLIENTHELPER_H
#define _CLIENTHELPERS_H 1

#include <stdio.h>
#include <stdlib.h>
#define DIE(assertion, call_description)  \
    do                                    \
    {                                     \
        if (assertion)                    \
        {                                 \
            fprintf(stderr, "(%s, %d): ", \
                    __FILE__, __LINE__);  \
            perror(call_description);     \
            exit(EXIT_FAILURE);           \
        }                                 \
    } while (0)

#define BUFLEN 1520
#define MAX_CLIENTS 200
#define MAX_LEN 1520
#define MAX_TOPICS 300

struct topic {
    char name[MAX_LEN];
    int sf;
};

struct client_tcp {
    long long id;
    char data_type[20];
    char message[MAX_LEN];
    char payload[MAX_LEN];
    struct topic topics[MAX_TOPICS];
    struct topic del_topics_sf1[MAX_TOPICS];
};

struct udp_message {
    char data_type[20];
    int port;
    char message[MAX_LEN];
    char payload[MAX_LEN];
    struct topic top;
    long long ip;
};

struct client_udp {
    long long ip;
    char data_type[20];
    int port;
    char message[MAX_LEN];
    char payload[MAX_LEN];
    struct topic topics[MAX_TOPICS];
};

struct database {
    struct client_udp udp_client[MAX_CLIENTS];
    struct client_tcp tcp_client[MAX_CLIENTS];
    struct topic topics[MAX_TOPICS];

    int tcp_size, udp_size;
};

// Declaring the database used;
struct database clients_database;

#endif
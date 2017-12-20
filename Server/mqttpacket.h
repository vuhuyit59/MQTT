
#ifndef MQTTPACKET_H
#define MQTTPACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAXSTR 1024
#define SA  struct sockaddr
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 70


#define MAXCLIENTS 20
#define MAXTOPIC 21
#define MAXACCOUNT 40

#define HEADER_LEN 1
#define CONNECT 1
#define CONNACK 2
#define PUBLISH 3
#define GET 4
#define CREATE 5
#define CREATEACK 6
#define SENDLIST 7
#define SUBSCRIBE 8
#define SUBACK 9
#define SENDFILE 10
#define DOWNFILE 11
#define FILEACK 12
#define DISCONNECT 14

typedef struct  {
    char name[32];
    char pwd[12];
}Accounts;

typedef struct  {
    int socket;
    char username[32];
}Clients;


typedef struct {
    char name[50];
    int count;
    char *listClient[50];
} Topic;

char *MQTT_Connect_Encode(char *name, char *pwd);

Accounts MQTT_Connect_Decode(char *buff);

char *MQTT_Ack_Encode(int retCode, int x);

int MQTT_Ack_Decode(char *packet);

char *MQTT_Subcribe_Encode(char *topicName);

char *MQTT_Publish_Encode(char *topicName, char *msg);

char **MQTT_Publish_Decode(char *packet);

char *MQTT_Create_Topic(char *name);

char *MQTT_Get(int opt);

void MQTT_Send_Decode(char *s);

// char *MQTT_Sendfile(char *target, char *msg, int opt);

// char *MQTT_Pubrec(char *msg);

// char *MQTT_Add(int, char *, char *);

// char *MQTT_Fileack(int x);

// void process_send_file(int, char *);

// void process_recv_file(int sockfd, char *filename);

// bool authentication(struct Accounts, struct Accounts acc[], int n);

// bool create_account(struct Accounts a, struct Accounts acc[], int n);

// bool chat_user(int sockfd, struct Clients [], int, char *, bool);

// bool chat_room(int sockfd, struct Rooms [], int, struct Clients [], int, char *, bool);

// void MQTT_Pubrec_Decode(char *msg);

// void process_send_file(int, char *);

#endif 
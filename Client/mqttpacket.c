#include "mqttpacket.h"


char *MQTT_Connect_Encode(char *name, char *pwd) {
    size_t len;
    len = HEADER_LEN + strlen(name) + 1 + strlen(pwd) + 2;
    char *packet = malloc(len);
    *packet = CONNECT << 4;
    strcat(packet, name);
    strcat(packet, ",");
    strcat(packet, pwd);
    strcat(packet,"-;");
    return packet;
}

Accounts MQTT_Connect_Decode(char *buff){
    char *p = buff;
    p = p + 1;
    Accounts a;
    strcpy(a.name, strtok(p, ","));
    if (a.name[strlen(a.name) - 1] == '\n')
        a.name[strlen(a.name) - 1] = '\0';
    strcpy(a.pwd, strtok(NULL, ","));
    if (a.name == NULL || a.pwd == NULL) {
        strcpy(a.name, "1");
        strcpy(a.pwd, "1");
    }
    return a;
}

char *MQTT_Ack_Encode(int retCode, int x){
    char *packet = malloc(3);
    * packet = x << 4;
    *packet |= retCode;
    strcat(packet,"-;");
    return packet;
}

int MQTT_Ack_Decode(char *packet){
     return *packet & 15;
}

char *MQTT_Subcribe_Encode(char *topicName){
    size_t len;
    len = HEADER_LEN + strlen(topicName) + 2;
    char *packet = malloc(len);
    *packet = SUBSCRIBE << 4;
    strcat(packet, topicName);
    strcat(packet,"-;");
    return packet;
}

char *MQTT_Publish_Encode(char *topicName, char *msg){
    size_t len;
    len = HEADER_LEN + strlen(topicName) + strlen(msg) + 3;
    char *packet = malloc(len);
    *packet = PUBLISH << 4;
    strcat(packet,topicName);
    strcat(packet,",");
    strcat(packet,msg);
    strcat(packet,"-;");
    return packet;
}

char **MQTT_Publish_Decode(char *packet){
    char **s = malloc(strlen(packet));
    packet = packet + 1;
    s[0] = strtok(packet, ",");
    s[1] = strtok(NULL, "-;");
    return s;
}

char *MQTT_Create_Topic(char *name){
    size_t len;
    len = HEADER_LEN + strlen(name)+2;
    char *packet = malloc(len);
    *packet = CREATE << 4;
    strcat(packet,name);
    return packet;
}

char *MQTT_Get(int opt){
    char *packet = malloc(3);
    *packet = GET << 4;
    *packet |= opt;
    strcat(packet,"-;");
    return packet;    
}

void MQTT_Send_Decode(char *s){
    char *p;
    s = s+1;
    p =strtok(s,",");
    while (p != NULL){
        printf("%s\n",p);
        p = strtok(NULL,",");
    }
    return ;
}


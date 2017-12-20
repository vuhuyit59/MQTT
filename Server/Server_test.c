#include "mqttpacket.h"

Clients *client[MAXCLIENTS];

Topic *topic[MAXTOPIC];

Accounts *ac[MAXACCOUNT];

int nocl=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void MQTT_Send(int confd){
    char *packet = malloc(1024);
    *packet = SENDLIST << 4;
    int i;
    strcat(packet,"List Of Clients:");
    for(i=0;i<nocl;i++){
        if(client[i]){
        	Clients *a ;
        	a = client[i];
            strcat(packet,a->username);
            strcat(packet,",");
            i++;
        }
    }
    strcat(packet,"List of topics :");
    for(i=0;i<MAXTOPIC;i++){
        if(topic[i]){
            strcat(packet,topic[i]->name);
            strcat(packet,",");
            i++;
        }
    }
    strcat(packet,"-;");
    printf("%s\n",packet);
    write(confd,packet,strlen(packet));
    return;
}

int noc =0 ;
void add_account(Accounts *a);

void add_topic(Topic *a);

int isExistedAccount(Accounts *a);

Topic *isExistedTopic(char *name);

Clients *isExistedClient(char *name);

void MQTT_Send_Topic(int confd);

void MQTT_Send_Client(int confd);

void connect_ex(char *t, int confd);

void publish_ex(char *t,int confd);

void send_Topic_Msg(Topic *a , char *msg, int confd);

void send_Client_Msg(Clients *a , char *msg, int confd);

void *doit(void *fd);


int main (int argc, char **argv){
	int listenfd;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	pthread_t tid;
	int *connfd;
	if( listenfd < 0){
		perror("socket");
		exit(1);
	}
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(9222);
	bind(listenfd,(struct sockaddr *) &servaddr, sizeof(servaddr));
	if(listen(listenfd, 5) < 0 ){
		printf("Failed to listen\n");
		return 1;
	}
	printf("Server ready !\n");
	for(;;){
		clilen =sizeof(cliaddr);
		connfd = malloc(sizeof(int));
		*connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
		pthread_create(&tid,NULL,&doit,(void *) connfd);
		}
	close(listenfd);
	return 0;
}

void *doit(void *fd){
	int connfd = *((int  *) fd);
	free(fd);
	char recvBuff[1024];
	int n=0;
	while(1){
		if((n = (read(connfd,recvBuff,sizeof(recvBuff)-1))) > 0){
			char *token;
		  	token = strtok(recvBuff,"-;");
		  	while(token!= NULL){
		  		int ctype = *token >> 4;
		  		char *temp = token;
		  		token = strtok(NULL,"-;");
		  		switch(ctype){
			  		case CONNECT :
			  			connect_ex(temp,connfd);
			  			continue;
			  		case PUBLISH :
			  			publish_ex(temp,connfd);
			  			continue;
			  		case GET:
			  			MQTT_Send(connfd);
			  			continue;
			  		default:
			  			break;
		  		}
		  	}
			memset(recvBuff,0,sizeof(recvBuff));
		}
	}
	return NULL;
}


void add_client(Clients *cl){
	// int i;
	// for(i=0;i<MAXCLIENTS;i++){
	// 	if(!client[i]){
	// 		client[i] = cl;
	// 		noc++;
	// 		printf("%s login.\n",cl->username);
	// 		return;
	// 	}
	// }
	// return;
	pthread_mutex_lock(&mutex);
    // client[nocl]->socket = cl->socket;
    // strcpy(client[nocl]->username, cl->username);
    client[nocl] = cl;
    nocl++;
    pthread_mutex_unlock(&mutex);
}

void add_account(Accounts *a){
	int i;
	for(i=0;i<MAXACCOUNT;i++){
		if(!ac[i]){
			ac[i] = a;
			return;
		}
	}
	return;
}

void add_topic(Topic *a){
	int i;
	for(i=0;i<MAXTOPIC;i++){
		if(!topic[i]){
			topic[i] = a;
			return;
		}
	}
	return;
}

int isExistedAccount(Accounts *a){
	int i;
	for(i=0; i< MAXACCOUNT ; i++){
		if(ac[i]){
			if(ac[i]->name == a->name){
				if(ac[i]->pwd == a->pwd){
					return 1;
				}else{
					return 2;
				}
			}
		}
	}
	add_account(a);
	return 1;
} 

Topic *isExistedTopic(char *name){
	int i;
	Topic *a;
	for(i=0; i< MAXTOPIC ; i++){
		if(topic[i]){
			if(topic[i]->name == name){
				a = topic[i];
				return a;
			}
		}
	}
	return NULL;
} 

Clients *isExistedClient(char *name){
	int i;
	for(i=0; i< MAXCLIENTS ; i++){
		if(client[i]){
			if(client[i]->username == name){
				return client[i];
			}
		}
	}
	return NULL;
}

void connect_ex(char *t, int confd){
	Accounts a;
	a = MQTT_Connect_Decode(t);
	int opt = isExistedAccount(&a);
	char *s = MQTT_Connack_Encode(opt,CONNACK);
	write(confd,s,strlen(s));
	if(opt == 0 || opt == 2){
		close(confd);
		exit(0);
	}
	Clients cl;
	cl.socket = confd;
	strcpy(cl.username,a.name);
	add_client(&cl);
}

void publish_ex(char *t,int confd){
	char **s;
	s = MQTT_Publish_Decode(t);
	Topic *a;
	a = isExistedTopic(s[0]);
	if(a == NULL){
		Clients *c ;
		c = isExistedClient(s[0]);
		if(c != NULL){
			send_Client_Msg(c, s[1], confd);
			return ;
		}
	}
	char *temp = s[0];
	strcat(temp,":");
	strcat(temp,s[1]);
	send_Topic_Msg(a,temp, confd);
	return;
}

void send_Topic_Msg(Topic *a, char *msg ,int confd){
	int i;
	char *s;
	for(i=0;i < a->count ;i++){
		Clients *c;
		c = isExistedClient(a->listClient[i]);
		if(c != NULL){
			s = MQTT_Publish_Encode(a->name,msg);
			write(c->socket,s,strlen(s));
		}
	}
	return;
}

void send_Client_Msg(Clients *a, char *msg ,int confd){
	char *s;
	s = MQTT_Publish_Encode(a->username,msg);
	write(a->socket,s,strlen(s));
	return;
}
// void MQTT_Send_Topic(int confd){
//     char *packet = malloc(1024);
//     *packet = SENDLIST << 4;
//     int i;
//     for(i=0;i<MAXTOPIC;i++){
//         if(topic[i]){
//             strcat(packet,topic[i]->name);
//             strcat(packet,",");
//             i++;
//         }
//     }
//     strcat(packet,"-;");
//     // write(confd,packet,strlen(packet));
//     return;
// }


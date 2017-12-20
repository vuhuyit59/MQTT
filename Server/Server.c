#include "mqttpacket.h"

Clients client[MAXCLIENTS];

Topic topic[MAXTOPIC];

Accounts ac[MAXACCOUNT];

int nocl=0,notp=0,noac=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void MQTT_Send(int confd){
    char *packet = malloc(1024);
    *packet = SENDLIST << 4;
    int i;
    strcat(packet,"List Of Clients:");
    for(i=0;i<=nocl;i++){
        if(client[i].username != NULL && strlen(client[i].username) > 0 ){
        	Clients a ;
        	a = client[i];
            strcat(packet,a.username);
            strcat(packet,",");
        }
    }
    strcat(packet,"List of topics :");
    for(i=0;i<=notp;i++){
        if(topic[i].name != NULL && strlen(topic[i].name) > 0){
            strcat(packet,topic[i].name);
            strcat(packet,",");
        }
    }
    strcat(packet,"-;");
    // printf("%s\n",packet);
    write(confd,packet,strlen(packet));
}

int noc =0 ;
void add_account(Accounts a);

void add_topic(Topic a);

int isExistedAccount(Accounts a);

int isExistedTopic(char *name);

int isExistedClient(char *name, int confd);

void MQTT_Send_Topic(int confd);

void MQTT_Send_Client(int confd);

void connect_ex(char *t, int confd);

void publish_ex(char *t,int confd);

void send_Topic_Msg(Topic a , char *msg, int confd);

void send_Client_Msg(Clients a , char *msg, int confd);

void *doit(void *fd);

void add_client(Clients cl);

void subcribe_ex (char *t, int confd);

void create_ex(char* t, int confd);

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
	servaddr.sin_port = htons(9656);
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
			// printf("%s\n",recvBuff);
		  	token = strtok(recvBuff,"-;");
		  	while(token!= NULL){
		  		int ctype = *token >> 4;
		  		char *temp = token;
		  		token = strtok(NULL,"-;");
		  		if( ctype < 0){
		  			ctype = -(ctype);
		  		}
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
			  		case SUBSCRIBE:
			  			subcribe_ex(temp,connfd);
			  			continue;
			  		case CREATE:
			  			create_ex(temp,connfd);
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


void add_client(Clients cl){
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
    client[nocl].socket = cl.socket;
    strcpy(client[nocl].username, cl.username);
    client[nocl] = cl;
    nocl++;
    pthread_mutex_unlock(&mutex);
}

void add_account(Accounts a){
	// int i;
	// for(i=0;i<MAXACCOUNT;i++){
	// 	if(ac[i].name != NULL){
	// 		ac[i] = a;
	// 		return;
	// 	}
	// }
	// return;
	pthread_mutex_lock(&mutex1);
   	ac[notp] =a;
    notp++;
    pthread_mutex_unlock(&mutex1);
}

void add_topic(Topic a){
	// int i;
	// for(i=0;i<MAXTOPIC;i++){
	// 	if(topic[i]!=NULL){
	// 		topic[i] = a;
	// 		return;
	// 	}
	// }
	pthread_mutex_lock(&mutex);
   	topic[notp] =a;
    notp++;
    pthread_mutex_unlock(&mutex);
	return;
}

int isExistedAccount(Accounts a){
	int i;
	for(i=0; i< noac ; i++){
		if(ac[i].name){
			if(!strcmp(ac[i].name, a.name)){
				if(ac[i].pwd == a.pwd){
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

int isExistedTopic(char *name){
	int i;
	for(i=0; i < notp ; i++){
		if(topic[i].name != NULL){
			if(!strcmp(topic[i].name, name)){
					return i;
			}
		}
	}
} 

int isExistedClient(char *name, int n){
	int i;
	for(i=0; i< nocl ; i++){
		if(client[i].username){
			if(!strcmp(client[i].username,name)){
				break;
			}
		}
		if(client[i].socket){
			if(client[i].socket == n){
				break;
			}
		}
	}
	return i;
}

void connect_ex(char *t, int confd){
	Accounts a;
	a = MQTT_Connect_Decode(t);
	int opt = isExistedAccount(a);
	char *s = MQTT_Ack_Encode(opt,CONNACK);
	write(confd,s,strlen(s));
	if(opt == 0 || opt == 2){
		close(confd);
		exit(0);
	}
	Clients cl;
	cl.socket = confd;
	strcpy(cl.username,a.name);
	add_client(cl);
}

void publish_ex(char *t,int confd){
	char **s;
	char *msg,*tp;
	s = MQTT_Publish_Decode(t);
	msg = s[1];
	tp = s[0];
	Topic a;
	a = topic[isExistedTopic(tp)-1];
	if(a.name == NULL || strlen(a.name) < 1){
		Clients c ;
		c = client[isExistedClient(tp,-1)];
		if(c.username != NULL){
			send_Client_Msg(c, msg, confd);
			return ;
		}
	}else{
	// char *temp = tp;
	// strcat(temp,":");
	// strcat(temp,msg);
	s = MQTT_Publish_Decode(t);
	send_Topic_Msg(a,send_Topic_Msg,confd);
	}
	return;
}

void send_Topic_Msg(Topic a, char *msg ,int confd){
	int i;
	char *s;
	Clients c;
	for(i=0; i <= a.count ;i++){
		c = client[isExistedClient(a.listClient[i],confd)-1];
		if(c.username != NULL && c.socket != confd){
			s = MQTT_Publish_Encode(a.name,msg);
			printf("%s\n",s);
			write(c.socket,s,strlen(s));
		}
	}
	return;
}

void send_Client_Msg(Clients a, char *msg ,int confd){
	char *s;
	Clients b = client[isExistedClient("",confd)];
	strcat(b.username,":");
	strcat(b.username,msg);
	s = MQTT_Publish_Encode(a.username,b.username);
	write(a.socket,s,strlen(s));
	return;
}

void subcribe_ex (char *t, int confd){
	char *topic2 = malloc (strlen(t)) ,*s;
	strncpy(topic2,t+1,strlen(t));
	int a = isExistedTopic(topic2);
	if (topic[a].name == NULL || strlen(topic[a].name) < 1){
		s = MQTT_Ack_Encode(0,SUBACK);
	}else{
		Clients c;
		c = client[isExistedClient(topic[a].name,-1)];
		topic[a].listClient[topic[a].count] = c.username;
		topic[a].count++;
		s = MQTT_Ack_Encode(1,SUBACK);
	}
	write(confd,s,strlen(s));
}
void create_ex(char* t, int confd){
	char *topic2 = malloc(strlen(t)),*s;
	strncpy(topic2,t+1,strlen(t));
	int a = isExistedTopic(topic2);
	Topic f;
	if (topic[a].name == NULL || strlen(topic[a].name) < 1){
		strcpy(f.name, topic2);
		f.count = 0;
		s = MQTT_Ack_Encode(1,CREATEACK);
		Clients c;
		c = client[isExistedClient("",confd)];
		f.listClient[f.count]= c.username;
		f.count++;
		add_topic(f);
	}else{
		s = MQTT_Ack_Encode(0,CREATEACK);
	}
	write(confd,s,strlen(s));
}
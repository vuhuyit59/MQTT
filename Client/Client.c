#include "mqttpacket.h"

char username[32],pass[12];
struct sockaddr_in serv_addr;

static void *recvMess(void *arg);
static void *sendMess(void *arg);

void ack_ex(char *recvbuf){
	int opt = MQTT_Ack_Decode(recvbuf);
	// printf("%d\n",opt );
	if(opt==0){
		printf("Failed.\n");
		// close(sockfd);
		exit(0);
	}else {
		printf("Successed\n");
		printf(".-------------------------------------------------.\n");
        printf("|1. Type: @username@ to chat                        |\n");
        printf("|2. Type: .//getlist/ to get list users and rooms   |\n");
        printf("|3. Type: .//create/topicname to create topic       |\n");
        printf("|4. Type: .//subcr/topicname to subcribe topic      |\n");
        printf("|5. Type: exit(0) to exit program                   |\n");
        printf("*-------------------------------------------------*\n");
        printf(">>>");
	}
}
void publish_ex(char * recvbuff){
	char **mess = MQTT_Publish_Decode(recvbuff);
	printf("%s : %s\n",mess[0],mess[1]);
}
void sendlist_ex(char *recvbuff){
	recvbuff = recvbuff +1;
	char *token = strtok(recvbuff,",");
	while( token != NULL){
		token = strtok(NULL,",");
	}
}
void command_ex(char *s, int confd);

int main(int argc, char *argv[]){
  if(argc != 2){
	printf("Please enter ip address.\n");
  }else{
	  int *sockfd;
	  sockfd = malloc(sizeof(int));
	  pthread_t tid1 , tid2;
	  if((*sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
	    {
	      printf("\n Error : Could not create socket \n");
	      return 1;
	    }
	  if(inet_aton(argv[1], (struct in_addr *) &serv_addr) == 0){
	  	printf("IP address is invalid.");
	  	return 0;
	  }
	  serv_addr.sin_family = AF_INET;
	  serv_addr.sin_port = htons(4444);
	  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	  if(connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
	      printf("\n Error : Connect Failed \n");
	      return 1;
   	 }
	  printf("Enter your username :");
	  scanf("%s",username);
	  printf("Enter password:");
	  scanf("%s",pass);
	  char *connect_packet = MQTT_Connect_Encode(username,pass);
	  connect_packet[strlen(connect_packet)] = '\0';
	  // printf("%s\n",connect_packet );
	  write(*sockfd,connect_packet,strlen(connect_packet));
	  pthread_create(&tid1,NULL,&sendMess,(void *) sockfd); // send mesage thread 
	   pthread_create(&tid2,NULL,&recvMess,(void *) sockfd); // read message thread
	   pthread_join(tid1,NULL);
	   pthread_join(tid2,NULL);
	}
	return 0;

}

static void *recvMess(void *arg){
	int sockfd = *((int  *) arg);
	char *recvbuff = malloc(MAXSTR);
	 while(1){
		  	while((read(sockfd,recvbuff,MAXSTR)) > 0){
			  	char *token;
			  	token = strtok(recvbuff,"-;");
			  	while( token!= NULL){
				  	if (token[strlen(token) - 1] == '\n'){
			            token[strlen(token) - 1] = '\0';
				  	}
				  	if (token[strlen(token) - 1] == '\r'){
			            token[strlen(token) - 1] = '\0';
				  	}
			  		int ctype = (*token >> 4) & 15;
			  		char *temp = token;
			  		token = strtok(NULL,"-;");
			  		switch(ctype){
			  			case CONNACK:
			  				ack_ex(temp);
			  				continue;
			  			case PUBLISH:
			  				publish_ex(temp);
			  				continue;
			  			case CREATEACK:
			  				ack_ex(temp);
			  				continue;
			  			case SUBACK:
			  				ack_ex(temp);
			  				continue;
			  			case SENDLIST:
			  				sendlist_ex(temp);
			  			case DISCONNECT:
			  				exit(0);
			  			default:
			  				continue;
			  		}
			  	}
			  	memset(recvbuff,0,strlen(recvbuff));
		  	}
		}
		return NULL;
}

static void *sendMess(void *arg){
	int connfd;
	connfd = *( ( int *) arg);
 	pthread_detach(pthread_self()); 
 	fflush(stdin);
	char sendBuff[1024];
	while(1){
		memset(sendBuff,0,sizeof(sendBuff));
		fgets(sendBuff,sizeof(sendBuff),stdin);
		sendBuff[strlen(sendBuff)-1]='\0';
		if(strlen(sendBuff) > 1){
			command_ex(sendBuff,connfd);
		}
		if(strstr(sendBuff,"exit(0)")){
			break;
		}
	}
	return NULL;
}

void command_ex(char *s, int confd){
	char *a;
	if(strstr(s,".//getlist/")){
		a = MQTT_Get(0);
	}else if(strstr(s,"@")){
		char *topic2,*token;
		token = strtok (s+1,"@");
		topic2 = token;
		token = strtok(NULL,"");
		a = MQTT_Publish_Encode(topic2,token);
	}else if(strstr(s,".//create/")){
		char *topic2= malloc( strlen(s)-10);
		strncpy(topic2,s+10,strlen(s)-10);
		a = MQTT_Create_Topic(topic2);
	}else if(strstr(s,".//subcr/")){
		char *topic2 = malloc( strlen(s)-9);
		strncpy(topic2,s+9,strlen(s)-9);
		a = MQTT_Subcribe_Encode(topic2);
	}else if(strstr(s,"exit(0)")){
		exit(0);
		close(confd); 
	}else{
		printf("Wrong comand\n");
		return;
	}
	write(confd,a,strlen(a));

}
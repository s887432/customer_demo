#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
// MQTT
#include "MQTTClient.h"
// TCP
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr()
//UART
#include <termios.h>
#include <fcntl.h>

//define declarations
#define SIZE     50
#define MSG_KEYID_MQTT	1234
#define MSG_KEYID_TCP	5678

// MQTT
#define MQTT_ADDRESS     "tcp://"
#define MQTT_PORT		":1883"
#define TOPIC       "MCHP/MobileTron/demo"
#define CLIENTID    "ExampleClientSub"
#define QOS         1
#define TIMEOUT     10000L

// TCP Client/Server
#define MAX 256
#define TCP_PORT 1812
#define SA struct sockaddr

// UART
struct termios  savetty;

typedef struct msgbuf {
         long    mtype;
         char    data[SIZE];
} data_buf;

typedef struct argu {
	char device_name[SIZE];
	char server_addr[SIZE];
	char uart_device[SIZE];
} str_argu;

void *thMqttProc(void*);
void *thUartProc(void*);
void *thTcpProc(void*);

pthread_mutex_t lock;

volatile MQTTClient_deliveryToken deliveredtoken;

// MQTT processing
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    //printf("##Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    //printf("\nConnection lost\n");
    //printf("     cause: %s\n", cause);
}

void *thMqttProc(void* argv)
{
	str_argu  *argu = (str_argu*)argv;
	MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
	char addr[MAX];
	int count = 0;
	char sBuf[MAX];

	int msqid;
	key_t key = MSG_KEYID_MQTT;
	data_buf  rbuf;

	printf("Hello I am MQTT Thread...\r\n");

	sprintf(addr, "%s%s%s", MQTT_ADDRESS, (char*)argu->server_addr, MQTT_PORT);
	
	if ((msqid = msgget(key, 0666)) < 0) {
		perror("[thMqtt] msgget");
		exit(1);
	}else{ 
		(void) fprintf(stderr,"[thMqtt] msgget: msgget succeeded: msqid = %d\n", msqid);
	}
	
    MQTTClient_create(&client, addr, argu->device_name, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    
	conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

	while(1) {
		//receive the answer with message type 1
		if (msgrcv(msqid, &rbuf, sizeof(data_buf), 1, 0) == -1) {
			printf("[thMqtt] msgrcv (%d)", errno);
			exit(1);
		}
		printf("[thMqtt] (MQTT)%s\r\n", rbuf.data);
		sprintf(sBuf, "(MQTT)[%s] %s", argu->device_name, rbuf.data);

		pubmsg.payload = sBuf;
		pubmsg.payloadlen = strlen(sBuf);
		pubmsg.qos = QOS;
		pubmsg.retained = 0;
		deliveredtoken = 0;

		MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
		
		while(deliveredtoken != token);    
	}

	MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}
// end of MQTT processing

// UART processing
int uartInit(char *device_name)
{
	int fd;
    struct termios  tty;
    int     rc;
    speed_t     spd;

	fd = open(device_name, O_RDWR | O_NOCTTY);

	if (fd < 0) {
        printf("failed to open: %d, %s", fd, strerror(errno));
        return -1;
    }

    rc = tcgetattr(fd, &tty);
    if (rc < 0) {
        printf("failed to get attr: %d, %s\r\n", rc, strerror(errno));
        return -2;
    }

	savetty = tty;    /* preserve original settings for restoration */

    spd = B115200;
    cfsetospeed(&tty, (speed_t)spd);
    cfsetispeed(&tty, (speed_t)spd);

    cfmakeraw(&tty);

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 10;

    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;    /* no HW flow control? */
    tty.c_cflag |= CLOCAL | CREAD;
    rc = tcsetattr(fd, TCSANOW, &tty);
    if (rc < 0) {
        printf("failed to set attr: %d, %s\r\n", rc, strerror(errno));
        return -3;
    }

	return fd;
}

void *thUartProc(void* argv)
{
	str_argu  *argu = (str_argu*)argv;
	int msqid_mqtt;
	int msgflg = IPC_CREAT | 0666;
	key_t key_mqtt = MSG_KEYID_MQTT;
	int msqid_tcp;
	key_t key_tcp = MSG_KEYID_TCP;
	data_buf sbuf;
	int sfd;
	unsigned char   buf[80];
	unsigned char	sendBuffer[80];
	int		dataIndex = 0;
    int     reqlen = 79;
    int     rdlen;
	
	pthread_mutex_lock(&lock);
	printf("Hello I am UART Thread...\r\n");
	
	if ((msqid_mqtt = msgget(key_mqtt, msgflg )) < 0) {
		perror("[thUart] msgget");
		exit(1);
	} else{ 
		printf("[thUart] msgget: msgget succeeded: msqid = %d\n", msqid_mqtt);
	}

	if ((msqid_tcp = msgget(key_tcp, msgflg )) < 0) {
		perror("[thUart] msgget");
		exit(1);
	} else{ 
		printf("[thUart] msgget: msgget succeeded: msqid = %d\n", msqid_tcp);
	}
	pthread_mutex_unlock(&lock);

	//send message type 1
	sbuf.mtype = 1;

	int count = 1;
	int keepGoing = 1;

	sfd = uartInit(argu->uart_device);

    if (sfd < 0) {
        printf("failed to open: %d, %s", sfd, strerror(errno));
        exit (-1);
    }
    printf("opened sfd=%d for reading\r\n", sfd);

	do
	{

		unsigned char   *p = buf;
    
        rdlen = read(sfd, buf, reqlen);
        if (rdlen > 0) {

			dataIndex = 0;
			for(int i=0; i<rdlen; i++)
			{
				if( ((rdlen-i) >= 2) && (*(p+i) == '\n' && *(p+i+1) == '\r') )
				{	
					sendBuffer[dataIndex] = 0;
					printf("[UART Receive]%s\n\r", sendBuffer);
					dataIndex = 0;
					i++;

					sprintf(sbuf.data, "%s", sendBuffer); 
				
					printf("[thUart] Sending: %s\r\n", sbuf.data);
					//send data from thUart to thMqtt
					if (msgsnd(msqid_mqtt, &sbuf, sizeof(data_buf), IPC_NOWAIT) < 0) {
						perror("[thUart] msgsnd");
						exit(1);
					}
					if (msgsnd(msqid_tcp, &sbuf, sizeof(data_buf), IPC_NOWAIT) < 0) {
						perror("[thUart] msgsnd");
						exit(1);
					}

				}
				else
					sendBuffer[dataIndex++] = *(p+i);
			}
        }



		sleep(1);
	}while(keepGoing);

	printf("[thUart] finished\r\n");
}
// end of UART processing

// TCP processing
void *thTcpProc(void *argv)
{
	str_argu  *argu = (str_argu*)argv;
    int rc;
	int count = 0;
	char sBuf[MAX];

	int msqid;
	key_t key = MSG_KEYID_TCP;
	data_buf  rbuf;

	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	printf("Hello I am TCP Thread...\r\n");

	if ((msqid = msgget(key, 0666)) < 0) {
		perror("[thTcp] msgget");
		exit(1);
	}else{ 
		(void) fprintf(stderr,"[thTcp] msgget: msgget succeeded: msqid = %d\n", msqid);
	}
	
	printf("[TCP] server:%s, Port:%s\r\n", argu->server_addr, argu->device_name);
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argu->server_addr);
	servaddr.sin_port = htons(TCP_PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
		!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	while(1) {
		//receive the answer with message type 1
		if (msgrcv(msqid, &rbuf, sizeof(data_buf), 1, 0) == -1) {
			printf("[thTcp] msgrcv (%d)", errno);
			exit(1);
		}
		printf("[thTcp] (TCP)%s\r\n", rbuf.data);
		sprintf(sBuf, "(TCP)[%s] %s", argu->device_name, rbuf.data);

		write(sockfd, sBuf, strlen(sBuf));
	}

	// close the socket
	close(sockfd);
}
// end of TCP processing

int main(int argc, char* argv[])
{
	str_argu  argu;
	pthread_t thUart;
	pthread_t thMqtt;
	pthread_t thTcp;
	int ret_val_t1;
	int ret_val_t2;
	int ret_val_t3;

	if( argc != 4 )
	{
		printf("USAGE: demo BROKER_IP DEVICE_NAME UART_DEVICE\r\n");
		exit(1);
	}

	strcpy(argu.server_addr, argv[1]);
	strcpy(argu.device_name, argv[2]);
	strcpy(argu.uart_device, argv[3]);

	//create thUart
	ret_val_t1 = pthread_create( &thUart, NULL, thUartProc, &argu);
	if(ret_val_t1)
	{
		fprintf(stderr,"Error - pthread_create() return value: %d\n",ret_val_t1);
		exit(EXIT_FAILURE);
	}

	//create thMqtt
	ret_val_t2 = pthread_create( &thMqtt, NULL, thMqttProc, &argu);
	if(ret_val_t2)
	{
		fprintf(stderr,"Error - pthread_create() return value: %d\n",ret_val_t2);
		exit(EXIT_FAILURE);
	}

	//create thTcp
	ret_val_t3 = pthread_create( &thTcp, NULL, thTcpProc, &argu);
	if(ret_val_t3)
	{
		fprintf(stderr,"Error - pthread_create() return value: %d\n",ret_val_t3);
		exit(EXIT_FAILURE);
	}

	printf("pthread_create() for thread UART returns: %d\n",ret_val_t1);
	printf("pthread_create() for thread MQTT returns: %d\n",ret_val_t2);
	printf("pthread_create() for thread TCP returns: %d\n",ret_val_t3);

	pthread_join( thUart, NULL);
	pthread_join( thMqtt, NULL); 
	pthread_join( thTcp, NULL); 

	exit(EXIT_SUCCESS);
}
	

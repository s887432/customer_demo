#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "MQTTClient.h"

#define DATETIME_SIZE  32
#define MAX 256

//#define ADDRESS     "tcp://localhost:1883"
//#define ADDRESS     "tcp://10.160.39.61:1883"
#define ADDRESS     "tcp://"
#define PORT		":1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "MCHP/MobileTron/demo"

#define QOS         1

#define TIMEOUT     10000L



volatile MQTTClient_deliveryToken deliveredtoken;

int GetDateTime(char * psDateTime)
{
    time_t nSeconds;
    struct tm * pTM;
    
    time(&nSeconds);
    pTM = localtime(&nSeconds);

    sprintf(psDateTime, "%04d-%02d-%02d %02d:%02d:%02d",
            pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
            pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
            
    return 0;
}

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

int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;
	char addr[32];
	int count = 0;

	if( argc != 3 )
	{
		printf("USAGE: mqtt_sub BROKER_IP ID\r\n");
		exit(1);
	}

	sprintf(addr, "%s%s%s", ADDRESS, argv[1], PORT);
	
	time_t t = time(NULL);
	struct tm *tm;
	char s[MAX];
	int i = 0;
	char DateTime[DATETIME_SIZE];
	
    MQTTClient_create(&client, addr, argv[2], MQTTCLIENT_PERSISTENCE_NONE, NULL);
    
	conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    
	printf("%s:%d\r\n", __func__, __LINE__);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

	printf("%s:%d\r\n", __func__, __LINE__);

	while(1) {
		GetDateTime(DateTime);
		sprintf(s, "%s,%s,%02d,%02d,%02d", argv[2], DateTime, ((count/10000)%10000), ((count/100)%100), (count%100));
		printf("Sending to broker: %s\r\n", s);

		pubmsg.payload = s;
		pubmsg.payloadlen = strlen(s);
		pubmsg.qos = QOS;
		pubmsg.retained = 0;
		deliveredtoken = 0;

		MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
		
		while(deliveredtoken != token);    

		sleep(1);
	}
	MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    
	return rc;
}

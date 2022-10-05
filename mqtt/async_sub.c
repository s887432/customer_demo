#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#include <errno.h>
#include <time.h>

#define _DATETIME_SIZE  32


//#define ADDRESS     "tcp://localhost:1883"
//#define ADDRESS     "tcp://10.160.39.61:1883"
#define ADDRESS     "tcp://"
#define PORT		":1883"
#define TOPIC       "MCHP/MobileTron/demo"
#define CLIENTID    "ExampleClientSub"
#define QOS         1
#define TIMEOUT     10000L
volatile MQTTClient_deliveryToken deliveredtoken;


int GetDateTime(char * psDateTime)
{
    time_t nSeconds;
    struct tm * pTM;
    
    time(&nSeconds);
    pTM = localtime(&nSeconds);

    /* 系統日期和時間,格式: yyyymmddHHMMSS */
    sprintf(psDateTime, "%04d-%02d-%02d %02d:%02d:%02d",
            pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday,
            pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
            
    return 0;
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;
	char DateTime[_DATETIME_SIZE];

	GetDateTime(DateTime);
	printf("[%s]: ", DateTime);

    payloadptr = message->payload;
	printf("%s\r\n", payloadptr);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}
int main(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
	char addr[64];

	if( argc != 2 )
	{
		printf("USAGE: mqtt_sub BROKER_IP\r\n");
		exit(1);
	}

	sprintf(addr, "%s%s%s", ADDRESS, argv[1], PORT);

    MQTTClient_create(&client, addr, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);
    do
    {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}



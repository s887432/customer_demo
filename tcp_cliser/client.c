#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <time.h>

#define DATETIME_SIZE  32

#define MAX 256
#define PORT 1812
#define SA struct sockaddr

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

// payload
// <DATE_TIME>,<DATA_1>,<DATA_2>,<DATA_3>
// DATE_TIME format
// YYYY-MM-DD HH-MM-SS

void func(int sockfd, char *device_name)
{
	char buff[MAX];
	char DateTime[DATETIME_SIZE];

	int count = 0;
	int n;

	for (;;) {
		GetDateTime(DateTime);
		sprintf(buff, "[%s] %s,%02d,%02d,%02d", device_name, DateTime, ((count/10000)%10000), ((count/100)%100), (count%100));
		printf("Send to server: [%s]%s\r\n", device_name, buff);

		write(sockfd, buff, strlen(buff));

		sleep(1);
	}
}

int main(int argc, char **argv)
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	if( argc != 3 ) {
		printf("USAGE: client SERVER_IP DEVICE_NAME\r\n");
		return -1;
	}

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
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
		!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func(sockfd, argv[2]);

	// close the socket
	close(sockfd);
}


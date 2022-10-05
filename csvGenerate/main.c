#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX 256
#define DATETIME_SIZE  32
#define MAX_OUTPUT_ITEM 100

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

int main(int argc, char **argv)
{
	FILE *fp;
	char DateTime[DATETIME_SIZE];
	char buff[MAX];
	int i;
	int count = 0;

	if( (fp = fopen("output.csv", "w")) == NULL ) {
		printf("Open output.csv error\r\n");
		return -1;
	}

	for(i=0; i<MAX_OUTPUT_ITEM; i++) {
		GetDateTime(DateTime);
		sprintf(buff, "%s,%02d,%02d,%02d\r\n", DateTime, ((count/10000)%10000), ((count/100)%100), (count%100));
		printf("Write to file: %s", buff);

		fwrite(buff, strlen(buff), 1, fp);

		count++;
	}

	fclose(fp);	
}


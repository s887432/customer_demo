#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <termios.h>
#include <string.h>

#define UART_DEVICE_NODE "/dev/ttyS1"

struct termios  savetty;

int uartInit(char *node_name)
{
	int fd;
    struct termios  tty;
    int     rc;
    speed_t     spd;

	fd = open("/dev/ttyS2", O_RDWR | O_NOCTTY);

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

int main(int argc, char **argv)
{
	int sfd;
    unsigned char   buf[80];
    int     reqlen = 79;
    int     rdlen;
    int     pau = 0;

	printf("uartRAW testing...\r\n");

    sfd = uartInit(UART_DEVICE_NODE);

    if (sfd < 0) {
        printf("failed to open: %d, %s", sfd, strerror(errno));
        exit (-1);
    }
    printf("opened sfd=%d for reading\r\n", sfd);

    do {
        unsigned char   *p = buf;
    
        rdlen = read(sfd, buf, reqlen);
        if (rdlen > 0) {
            if (*p == '\r')
                pau = 1;

            printf("read: %d bytes\r\n", rdlen);

			for(int i=0; i<rdlen; i++)
			{
				printf("%02X ", buf[i]);
			}
			printf("\r\n");
        } else {
            printf("failed to read: %d, %s\r\n", rdlen, strerror(errno));
        }
    } while (!pau);

    tcsetattr(sfd, TCSANOW, &savetty);
    close(sfd);

	exit (0);
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "modbus.h"
#include "modbusd.h"

int verbose = 0;

int main(int argc, char **argv)
{
	int c;
	int port = MODBUS_PORT;
	int use_stderr = 1;
        int true = 1;
        int false = 0;
	int s;
	int r;
	struct sockaddr_in laddr;
	struct sockaddr_in raddr;
	int size;
	pthread_t thread;

        while ((c = getopt (argc, argv, "vp:s")) != -1)
        {
                switch (c)
                {
                        case 'v':
                                verbose = 1;
                        break;
                        case 's':
                                use_stderr = 0;
                        break;
                        case 'p':
                                port = atoi(optarg);
                        break;
                        case 'h':
                        default:
                                fprintf(stderr, "usage:\n"
						"\t-v\t\tverbose\n"
						"\t-s\t\tlog only to syslog\n"
						"\t-p <port>\tport\n"
						"\t-h\t\thelp\n"
				);
                                exit(-1);
                }
        }
        if (use_stderr)
        {
                openlog(NULL, LOG_PERROR, LOG_USER);
        }
        else
        {
                openlog(NULL, 0, LOG_USER);
        }

        VERBOSE("socket\n");
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0)
        {
                ERROR("%m: socket");
                return -1;
        }
        VERBOSE("setsockopt SO_REUSEADDR\n");
        if(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true)) < 0)
        {
                ERROR("%m: SO_REUSEADDR");
                return -1;
        }
        memset(&laddr, 0, sizeof(laddr));
        laddr.sin_family = AF_INET;
        laddr.sin_addr.s_addr = htonl(INADDR_ANY);
        laddr.sin_port = htons(port);
        VERBOSE("bind\n");
        if (bind(s, (struct sockaddr *)&laddr, sizeof(struct sockaddr)) < 0)
        {
                ERROR("%m: bind");
                return -1;
        }
        VERBOSE("listen\n");
        if (listen(s, 16) < 0)
        {
                ERROR("%m: listen");
                return -1;
	}
	for (;;)
	{
		size = sizeof(struct sockaddr);
		r = accept(s, (struct sockaddr *)&raddr, &size);
        	if (r < 0)
		{
			ERROR("%m: accept");
			return -1;
		}
		VERBOSE("accepted %s:%d\n", inet_ntoa(raddr.sin_addr), ntohs(raddr.sin_port));
		pthread_create(&thread, NULL, modbus, (void*)&r);
	}
}

/*
 * Copyright (C) 2022 Timo Vesalainen <timo.vesalainen@iki.fi>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <dlfcn.h>

#include "modbus.h"
#include "modbusd.h"

struct modbusd_ctx ctx;
int use_stderr = 1;

struct plugin *plugins[256];

int add_plugin(const char* arg)
{
	int unit;
	char *parg;
	char *lib;
	char *larg;
	void *plib;
	int (*init)(struct modbusd_ctx*, const char*);

	parg = strdup(arg);
	unit = atoi(parg);
	lib = index(parg, ':');
	if (lib == NULL)
	{
		fprintf(stderr, "illegal unit %s\n", parg);
		exit(-1);
	}
	lib++;
	larg = index(lib, ':');
	if (larg != NULL)
	{
		*larg = 0;
		larg++;
		VERBOSE("add unit %d lib %s %s\n", unit, lib, larg);
	}
	else
	{
		VERBOSE("add unit %d lib %s\n", unit, lib);
	}
	plib = dlopen(lib, RTLD_LAZY);
	if (plib == NULL)
	{
		fprintf(stderr, "couldn't load %s: %s\n", lib, dlerror() );
		exit(-1);
	}

	struct plugin *plugin = (struct plugin*)calloc(1, sizeof(plugin));
	plugins[unit] = plugin;
	plugin->read_holding_registers = dlsym(plib, "read_holding_registers");
	plugin->write_multiple_registers = dlsym(plib, "write_multiple_registers");
	init = dlsym(plib, "init");
	if (init != NULL)
	{
		init(&ctx, larg);
	}
}

int main(int argc, char **argv)
{
	int c;
	int port = MODBUS_PORT;
        int true = 1;
        int false = 0;
	int s;
	int r;
	struct sockaddr_in laddr;
	struct sockaddr_in raddr;
	int size;
	pthread_t thread;

        while ((c = getopt (argc, argv, "vp:sl:")) != -1)
        {
                switch (c)
                {
                        case 'v':
                                ctx.verbose = 1;
                        break;
                        case 's':
                                use_stderr = 0;
                        break;
                        case 'p':
                                port = atoi(optarg);
                        break;
                        case 'l':
                                add_plugin(optarg);
                        break;
                        case 'h':
                        default:
                                fprintf(stderr, "usage:\n"
						"\t-v\t\t\tverbose\n"
						"\t-s\t\t\tlog only to syslog\n"
						"\t-p <port>\t\tport\n"
						"\t-l <unit>:<lib>:<arg>\tadd plugin\n"
						"\t-h\t\t\thelp\n"
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

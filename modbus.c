
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "modbus.h"
#include "modbusd.h"

void *modbus(int *s)
{
	int rc;
	struct modbus_tcp tcp_hdr;
	struct modbus_req req;
	int len;

	VERBOSE("new thread %d\n", *s);
	for (;;)
	{
		rc = read(*s, &tcp_hdr, sizeof(tcp_hdr));
		if (rc < 7)
		{
			ERROR("%m: read1 rc=%d", rc);
			pthread_exit(NULL);
		}
		len = ntohs(tcp_hdr.len);
		VERBOSE("tx=%d proto=%d len=%d unit=%d\n", 
			tcp_hdr.transaction_id,
			tcp_hdr.protocol_id,
			len, 
			tcp_hdr.unit_id);
		rc = read(*s, &req, len-1);
		if (rc < len-1)
		{
			ERROR("%m: read2 rc=%d", rc);
			pthread_exit(NULL);
		}
 
	}
}

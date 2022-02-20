
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/uio.h>

#include "plugin.h"
#include "modbus.h"
#include "modbusd.h"

int readn(int s, void *data, size_t len)
{
	int rc;

	while (len > 0)
	{
		rc = read(s, data, len);
		if (rc < 0)
		{
			ERROR("%m: read rc=%d", rc);
			pthread_exit(NULL);
		}
		len -= rc;
		data += rc;
	}
}
int write_error(int s, struct modbus_tcp *tcp_hdr, int function, int error)
{
	struct modbus_err err;
	struct iovec io[2];
	int rc;

	err.error_code = ERROR_CODE(function);
	err.exception_code = error;
	tcp_hdr->len = htons(3);
	io[0].iov_base = tcp_hdr;
	io[0].iov_len = sizeof(struct modbus_tcp);
	io[1].iov_base = &err;
	io[1].iov_len = sizeof(err);
	rc = writev(s, io, 2);
	if (rc < 0)
	{
		ERROR("%m: writev rc=%d", rc);
		pthread_exit(NULL);
	}
}


void *modbus(void *v)
{
	int s = *(int*)v;
	int rc;
	struct modbus_tcp tcp_hdr;
	struct modbus_req req;
	struct modbus_err err;
	int len;
	struct plugin *plugin;

	VERBOSE("new thread %d\n", s);
	for (;;)
	{
		readn(s, &tcp_hdr, sizeof(tcp_hdr));
		len = ntohs(tcp_hdr.len);
		VERBOSE("tx=%d proto=%d len=%d unit=%d\n", 
			tcp_hdr.transaction_id,
			tcp_hdr.protocol_id,
			len, 
			tcp_hdr.unit_id);
		readn(s, &req, len-1);
		plugin = plugins[tcp_hdr.unit_id]; 
		if (plugin == NULL)
		{
			write_error(s, &tcp_hdr, req.function, GATEWAY_PATH_UNAVAILABLE);
			continue;
		}
	}
}

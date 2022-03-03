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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
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
int write_response(int s, struct modbus_tcp *tcp_hdr, void *rsp, size_t rsp_size)
{
	struct iovec io[2];
	int rc;

	tcp_hdr->len = htons(rsp_size + 1);
	io[0].iov_base = tcp_hdr;
	io[0].iov_len = sizeof(struct modbus_tcp);
	io[1].iov_base = rsp;
	io[1].iov_len = rsp_size;
	rc = writev(s, io, 2);
	if (rc < 0)
	{
		ERROR("%m: writev rc=%d", rc);
		pthread_exit(NULL);
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
	int true = 1;
	struct modbus_tcp tcp_hdr;
	struct modbus_req req;
	struct modbus_rsp rsp;
	struct modbus_err err;
	int len;
	struct plugin *plugin;
	struct modbus_arg arg;
	unsigned int address;
	unsigned int quantity;

	VERBOSE("new thread %d\n", s);

	if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &true, sizeof(true)) < 0)
	{
		ERROR("TCP_NODELAY failed\n");
		pthread_exit(NULL);
	}
	
	for (;;)
	{
		readn(s, &tcp_hdr, sizeof(tcp_hdr));
		len = ntohs(tcp_hdr.len);
		VERBOSE("tx=%d proto=%d len=%d unit=%d\n", 
			ntohs(tcp_hdr.transaction_id),
			ntohs(tcp_hdr.protocol_id),
			len, 
			tcp_hdr.unit_id);
		readn(s, &req, len-1);
		plugin = plugins[tcp_hdr.unit_id]; 
		if (plugin == NULL)
		{
			write_error(s, &tcp_hdr, req.function, GATEWAY_PATH_UNAVAILABLE);
			continue;
		}
		switch (req.function)
		{
			case 3:	/* Read Holding Registers */
				if (plugin->read_holding_registers != NULL)
				{
					address = ntohs(req.address);
					quantity = ntohs(req.quantity);
					if (quantity > 125)
					{
						write_error(s, &tcp_hdr, req.function, ILLEGAL_DATA_VALUE );
						break;
					}
					rc = plugin->read_holding_registers(s, address, quantity, &arg);
					if (rc == 0)
					{
						rsp.function = req.function;
						rsp.bytes = 2*quantity;
						memcpy(&(rsp.data), &arg, 2*quantity);
						write_response(s, &tcp_hdr, &rsp, 2*quantity+2);
					}
					else
					{
						if (rc > 0)
						{
							write_error(s, &tcp_hdr, req.function, rc );
						}
						else
						{
							ERROR("%m: Read Holding Registers\n");
							write_error(s, &tcp_hdr, req.function, SLAVE_DEVICE_FAILURE );
						}
					}
				}
				else
				{
					write_error(s, &tcp_hdr, req.function, ILLEGAL_FUNCTION );
				}
				break;
			case 16:	/* Write Multiple Registers */
				if (plugin->write_multiple_registers != NULL)
				{
					address = ntohs(req.address);
					quantity = ntohs(req.quantity);
					if (quantity > 125)
					{
						write_error(s, &tcp_hdr, req.function, ILLEGAL_DATA_VALUE );
						break;
					}
					rc = plugin->write_multiple_registers(s, address, quantity, &req.data);
					if (rc == 0)
					{
						write_response(s, &tcp_hdr, &req, 5);
					}
					else
					{
						if (rc > 0)
						{
							write_error(s, &tcp_hdr, req.function, rc );
						}
						else
						{
							ERROR("%m: Write Multiple Registers\n");
							write_error(s, &tcp_hdr, req.function, SLAVE_DEVICE_FAILURE );
						}
					}
				}
				else
				{
					write_error(s, &tcp_hdr, req.function, ILLEGAL_FUNCTION );
				}
				break;
			default:
			write_error(s, &tcp_hdr, req.function, ILLEGAL_FUNCTION );
			break;
		}
	}
}

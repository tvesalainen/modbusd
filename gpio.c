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

#include <linux/gpio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <arpa/inet.h>

#include "plugin.h"

const char *dev;

int read_registers(__u16 address, __u16 quantity, struct modbus_arg *data);
int poll_register(int fd, __u16 address, struct modbus_arg *data);

int init(const char* arg)
{
	int fd;

	fprintf(stderr, "init arg %s\n", arg);
	dev = arg;

	fd = open(dev, O_RDONLY);
	if (fd < 0)
	{
		ERROR("init of %s failed\n", dev);
		return -1;
	}
	close(fd);
	return 0;
}

int read_holding_registers(int fd, __u16 address, __u16 quantity, struct modbus_arg *data)
{
	int hi;
	int lo;

	hi = address>>8;
	lo = address&0xff;
	switch (hi)
	{
		case 0:
			return read_registers(lo, quantity, data);
			break;
		case 1:
			return poll_register(fd, lo, data);
			break;
		default:
			ERROR("%d hi not supported\n", hi);
			return ILLEGAL_DATA_ADDRESS;
			break;
	}
}
int read_registers(__u16 address, __u16 quantity, struct modbus_arg *data)
{
	int fd;
	int rc;
	int ii;
	struct gpiohandle_request req;
	struct gpiohandle_data dat;

	if (quantity > GPIOHANDLES_MAX)
	{
		ERROR("more than %d handles requested\n", GPIOHANDLES_MAX);
		return ILLEGAL_DATA_VALUE;
	}
	fd = open(dev, O_RDONLY);
	if (fd < 0)
	{
		ERROR("open %s failed\n", dev);
		return -1;
	}
	for (ii=0;ii<quantity;ii++)
	{
		req.lineoffsets[ii] = address + ii;
	}
	req.flags = GPIOHANDLE_REQUEST_INPUT;
	req.lines = quantity;
	rc = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	close(fd);
	if (rc < 0)
	{
		ERROR("GPIO_GET_LINEHANDLE_IOCTL of %s failed\n", dev);
		return -1;
	}
	rc = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &dat);
	if (rc < 0)
	{
		ERROR("GPIOHANDLE_GET_LINE_VALUES_IOCTL of %s failed\n", dev);
		return -1;
	}
	close(req.fd);
	for (ii=0;ii<quantity;ii++)
	{
		data->array_short[ii] = htons(dat.values[ii]);
	}
	return OK;
}

int poll_register(int sock, __u16 address, struct modbus_arg *data)
{
	int fd;
	int rc;
	int ii;
	struct gpiohandle_data dat;
	struct gpioevent_request req;
	struct gpioevent_data event;
	struct pollfd pfd[2];

	fd = open(dev, O_RDONLY);
	if (fd < 0)
	{
		ERROR("open %s failed\n", dev);
		return -1;
	}
	req.lineoffset = address;
	strcpy(req.consumer_label, "modbusd");
	req.handleflags = 0;
	req.eventflags = GPIOEVENT_REQUEST_RISING_EDGE | GPIOEVENT_REQUEST_FALLING_EDGE;
	rc = ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req);
	close(fd);
	if (rc < 0)
	{
		ERROR("GPIO_GET_LINEEVENT_IOCTL of %s failed\n", dev);
		return -1;
	}
	pfd[0].fd = sock;
	pfd[0].events = POLLHUP;
	pfd[1].fd = req.fd;
	pfd[1].events = POLLIN;
	rc = poll(pfd, 2, -1);
	if (rc < 0)
	{
		ERROR("poll of %s failed\n", dev);
		return -1;
	}
	if (pfd[1].revents != POLLIN)
	{
		return -1;
	}
	rc = read(req.fd, &event, sizeof(event));
	if (rc < 0)
	{
		ERROR("read event of %s failed\n", dev);
		return -1;
	}
	close(req.fd);
	data->array_short[0] = htons(event.id == GPIOEVENT_EVENT_RISING_EDGE ? 1 : 0);
	return OK;
}

int write_multiple_registers(int sock, __u16 address, __u16 quantity, struct modbus_arg *data)
{
	int fd;
	int rc;
	int ii;
	struct gpiohandle_request req;
	struct gpiohandle_data dat;

	if (quantity > GPIOHANDLES_MAX)
	{
		return ILLEGAL_DATA_VALUE;
	}
	fd = open(dev, O_RDONLY);
	if (fd < 0)
	{
		ERROR("open %s failed\n", dev);
		return -1;
	}
	for (ii=0;ii<quantity;ii++)
	{
		req.lineoffsets[ii] = address + ii;
	}
	req.flags = GPIOHANDLE_REQUEST_OUTPUT;
	req.lines = quantity;
	rc = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	close(fd);
	if (rc < 0)
	{
		ERROR("GPIO_GET_LINEHANDLE_IOCTL of %s failed\n", dev);
		return -1;
	}
	for (ii=0;ii<quantity;ii++)
	{
		dat.values[ii] = ntohs(data->array_short[ii]);
	}
	rc = ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &dat);
	if (rc < 0)
	{
		ERROR("GPIOHANDLE_SET_LINE_VALUES_IOCTL of %s failed\n", dev);
		return -1;
	}
	close(req.fd);
	return OK;
}

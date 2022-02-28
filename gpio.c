
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

int init(const char* arg)
{
	int fd;

	fprintf(stderr, "init arg %s\n", arg);
	dev = arg;

	fd = open(dev, O_RDONLY);
	if (fd < 0)
	{
		perror(dev);
		return -1;
	}
	close(fd);
	return 0;
}

int read_holding_registers(__u16 address, __u16 quantity, struct modbus_arg *data)
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
		perror(dev);
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
		perror(dev);
		return -1;
	}
	rc = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &dat);
	if (rc < 0)
	{
		perror(dev);
		return -1;
	}
	close(req.fd);
	for (ii=0;ii<quantity;ii++)
	{
		data->array_short[ii] = htons(dat.values[ii]);
	}
	return OK;
}

int write_multiple_registers(__u16 address, __u16 quantity, struct modbus_arg *data)
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
		perror(dev);
		return -1;
	}
	for (ii=0;ii<quantity;ii++)
	{
		req.lineoffsets[ii] = address + ii;
fprintf(stderr, "off[%d]=%d\n", ii, address + ii);
	}
	req.flags = GPIOHANDLE_REQUEST_OUTPUT;
	req.lines = quantity;
	rc = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	close(fd);
	if (rc < 0)
	{
		perror(dev);
		return -1;
	}
	for (ii=0;ii<quantity;ii++)
	{
		dat.values[ii] = ntohs(data->array_short[ii]);
fprintf(stderr, "dat[%d]=%d\n", ii, dat.values[ii]);
	}
	rc = ioctl(req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &dat);
	if (rc < 0)
	{
		perror(dev);
		return -1;
	}
	close(req.fd);
	return OK;
}

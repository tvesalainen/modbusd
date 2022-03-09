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

#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include "plugin.h"

/*
	Address
	10-9	Output 32bit 
		0=raw 
		1=1000000*volts, 0x7fffffff overflow, 0x80000000 underflow
		2=ADC Pi same as Output but ADC Pi 10K/6.8K voltage divider calculated
	8-7	PGA Gain
	6-5	Sample Rate
	4-2	Device Address
	1-0	Channel
*/
#define OUT(x) ((x>>9)&0x3)
#define PGA(x) ((x>>7)&0x3)
#define GAIN(x) (1<<PGA(x))
#define RATE(x) ((x>>5)&0x3)
#define BITS(x) (2*(((x>>5)&0x3)+6))
#define ADDR(x) ((x>>2)&0x7)
#define CHANNEL(x) (x&0x3)
#define ADDRESS(a) (0xd<<3|a)
#define REGISTER(rdy, c, oc, s, g) (rdy<<7|c<<5|oc<<4|s<<2|g)

useconds_t SLEEP[] = {41667, 16667, 66667, 266667};
struct modbusd_ctx *ctx;
char dev[PATH_MAX];

#define VERBOSE(...) if (ctx->verbose) fprintf(stderr, __VA_ARGS__)

int init(struct modbusd_ctx *mctx, const char* arg)
{
	int adapter;
	int fd;

	fprintf(stderr, "init arg %s\n", arg);
	ctx = mctx;
	adapter = atoi(arg);
	snprintf(dev, sizeof(dev), "/dev/i2c-%d", adapter);

	fd = open(dev, O_RDONLY);
	if (fd < 0)
	{
		ERROR("init of %s failed\n", dev);
		return -1;
	}
	close(fd);
	return 0;
}

int read_holding_registers(int sock, __u16 address, __u16 quantity, struct modbus_arg *data)
{
	__u8 addr = ADDRESS(ADDR(address));
	__u8 reg = REGISTER(1, CHANNEL(address), 0, RATE(address), PGA(address));
	int fd;
	int rc;
	__s8 dat[4];
	int out = OUT(address);
	int pga = PGA(address);
	int bits = BITS(address);
	int raw;
	int ready;

	VERBOSE("out=%d pga=%d rate=%d bits=%d addr=%d ch=%d\n",
		OUT(address),
		PGA(address),
		RATE(address),
		BITS(address),
		ADDR(address),
		CHANNEL(address));
	VERBOSE("addr=%x reg=%x\n", addr, reg);

	if (quantity != 2)
	{
		ERROR("quantity %d != 2 (output must be 32 bits)\n", quantity);
		return -1;
	}
	fd = open(dev, O_RDWR);
	if (fd < 0)
	{
		ERROR("open %s failed\n", dev);
		return -1;
	}
	if  (ioctl(fd, I2C_SLAVE, addr) < 0)
	{
		ERROR("I2C_SLAVE %s failed\n", dev);
		return -1;
	}
	if (i2c_smbus_write_byte(fd, reg) < 0)
	{
		ERROR("i2c_smbus_write_byte_data %s failed\n", dev);
		return -1;
	}
	if (usleep(SLEEP[RATE(address)]) < 0)
	{
		ERROR("usleep %s failed\n", dev);
		return -1;
	}
	rc = read(fd, &dat, sizeof(dat));
	if (rc < 0)
	{
		ERROR("read %s failed\n", dev);
		return -1;
	}
	VERBOSE("in=%02x%02x%02x%02x\n", dat[0], dat[1], dat[2], dat[3]);
	switch (bits)
	{
		case 12:
			raw = (dat[0]&0xf)<<8|dat[1];
			ready = dat[2]&0x80;
			break;
		case 14:
			raw = (dat[0]&0x3f)<<8|dat[1];
			ready = dat[2]&0x80;
			break;
		case 16:
			raw = dat[0]<<8|dat[1];
			ready = dat[2]&0x80;
			break;
		case 18:
			raw = (dat[0]&0x3)<<16|dat[1]<<8|dat[2];
			ready = dat[3]&0x80;
			break;
		default:
			ERROR("illegal %d bits\n", bits);
			return -1;
	}
	VERBOSE("raw=%d ready=%x\n", raw, ready);
	if (ready)
	{
		ERROR("measurement not ready\n");
		return -1;
	}
	switch (out)
	{
		case 0:
			data->array_int[0] = htonl(raw);
			return 0;
		case 1:
			data->array_int[0] = htonl(1000*((2048*raw)>>(bits-1+pga)));
			return 0;
		case 2:
			data->array_int[0] = htonl(1000*(168*((2048*raw)>>(bits-1+pga))/68));
			return 0;
		default:
			ERROR("illegal %d out\n", out);
			return -1;
	}
}

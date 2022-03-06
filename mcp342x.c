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
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include "plugin.h"

/*
	Address
	9-9	ADC Pi
	8-7	PGA Gain
	6-5	Sample Rate
	4-2	Device Address
	1-0	Channel
*/
#define ADC(x) ((x>>9)&0x1)
#define PGA(x) ((x>>7)&0x3)
#define RATE(x) ((x>>5)&0x3)
#define ADDR(x) ((x>>2)&0x7)
#define CHANNEL(x) (x&0x3)
#define ADDRESS(a) (0xd<<3|a)
#define REGISTER(rdy, c, oc, s, g) (rdy<<7|c<<5|oc<<4|s<<2|g)

struct data
{
	union
	{
		struct 
		{
			__s32 pad:4;
			__s32 value:12;
			__s32 ready:1;
			__s32 channel:2;
			__s32 mode:1;
			__s32 rate:2;
			__s32 pga:2;
		} bits_12;
		struct 
		{
			__s32 pad:2;
			__s32 value:14;
			__s32 ready:1;
			__s32 channel:2;
			__s32 mode:1;
			__s32 rate:2;
			__s32 pga:2;
		} bits_14;
		struct 
		{
			__s32 value:16;
			__s32 ready:1;
			__s32 channel:2;
			__s32 mode:1;
			__s32 rate:2;
			__s32 pga:2;
		} bits_16;
		struct 
		{
			__s32 pad:6;
			__s32 value:18;
			__s32 ready:1;
			__s32 channel:2;
			__s32 mode:1;
			__s32 rate:2;
			__s32 pga:2;
		} bits_18;
	};
};
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
	struct data dat;

	VERBOSE("adc=%d pga=%d rate=%d addr=%d ch=%d\n",
		ADC(address),
		PGA(address),
		RATE(address),
		ADDR(address),
		CHANNEL(address));
	VERBOSE("addr=%x reg=%x\n", addr, reg);

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

	VERBOSE("%d %x\n", dat.bits_12.value, dat.bits_12.ready);
}

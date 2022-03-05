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

#include "plugin.h"

/*
	Address
	8-7	PGA Gain
	6-5	Sample Rate
	4-2	Device Address
	1-0	Channel
*/
#define PGA(x) ((x>>7)&0x3)
#define RATE(x) ((x>>5)&0x3)
#define ADDR(x) ((x>>2)&0x7)
#define CHANNEL(x) (x&0x3)
#define ADDRESS(a) (0xd<<3|a)
#define REGISTER(rdy, c, oc, s, g) (rdy<<7|c<<5|oc<<4|s<<2|g)

char dev[PATH_MAX];

int init(const char* arg)
{
	int adapter;
	int fd;

	fprintf(stderr, "init arg %s\n", arg);
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

	VERBOSE("pga=%d rate=%d addr=%d ch=%d\n",
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
}

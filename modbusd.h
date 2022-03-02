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

#ifndef _MODBUSD_H
#define _MODBUSD_H

#define MODBUS_PORT 502

extern int verbose;

struct plugin
{
	int (*read_holding_registers)(int, __u16, __u16, struct modbus_arg*);
	int (*write_multiple_registers)(int, __u16, __u16, struct modbus_arg*);
};

extern struct plugin *plugins[256];

#endif /* _MODBUSD_H */

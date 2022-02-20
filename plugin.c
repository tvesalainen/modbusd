
#include <linux/types.h>

#include "plugin.h"

int read_holding_registers(__u16 address, __u16 quantity, __u16 *data)
{
	int ii;

	for (ii=0;ii<quantity;ii++)
	{
		data[ii] = address;
	}
	return OK;
}
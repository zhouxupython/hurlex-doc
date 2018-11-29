#include "utils.h"
#include "debug.h"

uint32_t get_addr_store_val(uint32_t addr)
{
	if(!addr)
	{
		printk("^^^^^^^^^^ addr error\n");
	}
	uint32_t val = *((uint32_t*)addr);
	
	printk("val = %d\n", val);
	
	return val;
}


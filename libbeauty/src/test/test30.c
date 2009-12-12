/* To test one function with many args. */
#include <stdint.h>

int test30 ( int64_t value1, int64_t value2, int64_t value3, int64_t value4, int64_t value5, int64_t value6, int64_t value7, int64_t value8 )
{
	int64_t local;
	local = 0;
	local += value1;
	local += value2;
	local += value3;
	local += value4;
	local += value5;
	local += value6;
	local += value7;
	local += value8;
	
	return local;
}


/* To test one function call with many parameters in a way that one can automate the test. */
/* Input values should range from 0-7 */
#include <stdint.h>

int test18 ( int64_t value1, int64_t value2, int64_t value3, int64_t value4, int64_t value5, int64_t value6, int64_t value7, int64_t value8 ) {
	int64_t tmp = 0;
	tmp = (tmp * 8) + value1;
	tmp = (tmp * 8) + value2;
	tmp = (tmp * 8) + value3;
	tmp = (tmp * 8) + value4;
	tmp = (tmp * 8) + value5;
	tmp = (tmp * 8) + value6;
	tmp = (tmp * 8) + value7;
	tmp = (tmp * 8) + value8;
	return tmp;
}


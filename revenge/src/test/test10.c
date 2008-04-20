/* A very simple function to test memory stores. */

static mem1 = 0x123;

int test10(int value1 );

int test10(int value1 ) {
	int local;
	local = value1 + mem1;	
	return local;
}


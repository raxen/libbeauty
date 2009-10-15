/* A very simple function to test two memory stores. */

static int mem1 = 0x123;
static int mem2 = 0x1234;

int test12(int value1 );

int test12(int value1 ) {
	int local;
	local = value1 + mem1 + mem2;	
	return local;
}


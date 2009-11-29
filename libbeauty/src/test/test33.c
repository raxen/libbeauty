/* A very simple function to test pointers to memory stores. */

static mem1 = 0x123;
static *memory = &mem1;

int test33(int value1 );

int test33(int value1 ) {
	int local;
	local = value1 + *memory;	
	return local;
}

int main() {
	return test33(0x100);
}


/* To test one function calling another in a single .o file. */

int test22a ( int value );
int test22b ( int value );

int test22a ( int value ) {
	return value+0x123;
}

int test22b ( int value ) {
	int local;
	local = test21a(value);
	local = local + 0x100;
	return local;
}


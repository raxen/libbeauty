/* A very simple function to test filtering out of stack variables. */
/* Just slightly more complex that test1.c */

int test2 ( int value );

int test2 ( int value ) {
	int local;
	local = value;	
	local+=0x123;
	local+=0x111;
	return local;
}


/* To test one function calling another in a single .o file. */
#include <stdio.h>

int test23a ( int value );
int test23b ( int value );

int test23a ( int value ) {
	printf ("Fred");
	return value+0x123;
}

int test23b ( int value ) {
	int local;
	local = test23a(value);
	local = local + 0x100;
	return local;
}


/* A very simple function to test a for loop. */

int test9(int value1 );

int test9(int value1 ) {
	int local1, n;
	for (n = 0; n < 10; n++)
		local1 = value1 + 10;
	return local1;
}


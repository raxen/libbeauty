/* A very simple function to test an if statement. */
/* with a simple forward goto. */

int test7(int value1 );

int test7(int value1 ) {
	int local1;
	if (value1 > 5) {
		local1 = value1 + 10;
	}
	return local1;
}


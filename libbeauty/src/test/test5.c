/* A very simple function to test two parameters. */
/* Using a negation also identifies parameter ordering. */

int test5(int value1, int value2 );

int test5(int value1, int value2 ) {
	int local1, local2;
	local1 = value1;
	local2 = value2;
	return local1 - local2;
}


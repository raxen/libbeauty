/* A very simple function to test a goto statement. */

int test8(int value1 );

int test8(int value1 ) {
	int local1;
	if (value1 > 5) {
		local1 = value1 + 10;
		goto label1;
	}
	local1 = value1 + 20;
label1:
	return local1;
}


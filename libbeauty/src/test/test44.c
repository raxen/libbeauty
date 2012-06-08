/* A very simple function to test a do while() loop. */

int test44(int var1) {
	int n;
	do {
		n++;
		var1--;
	} while (n < 10);
	return var1;
}


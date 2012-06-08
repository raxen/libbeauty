/* A very simple function to test a nested for loop. */

int test39();

int test39() {
	int local1, local2, n, m;
	for (n = 0; n < 10; n++) {
		local2+=10;
		for (m = 0; m < 5; m++) {
			local1+=5;
		}
	}
	return local1 + local2;
}


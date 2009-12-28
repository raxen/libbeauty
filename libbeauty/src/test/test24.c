/* A very simple function to test memory arrays. */

static int mem[] = { 1,2,3,4,5,6,7,8,9 };

int test24(void) {
	int local, n;
	local = 0;
	for (n = 0; n < sizeof(mem); n++) {
		local = local + mem[n];
	}
	return local;
}


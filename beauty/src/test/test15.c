/* A very simple function to test memory stores. */

struct simple {
	int one;
	int two;
};

static struct simple fred[2] = { { 1, 2 }, {3, 4}};

int test15(int value1 );

int test15(int value1 ) {
	int local, n;
	for (n = 0; n < 2; n++) {
		local = fred[n].one;
		local += fred[n].two;
	}
	return local;
}


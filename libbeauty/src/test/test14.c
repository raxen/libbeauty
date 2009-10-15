/* A very simple function to test a forward goto. */

int test14();

int test14() {
	int local1 = 0;
	local1 += 2;
	goto forward_jump;
	local1 += 3;
	forward_jump:
	return local1;
}


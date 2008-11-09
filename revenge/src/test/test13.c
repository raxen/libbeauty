/* A very simple function to test an endless goto loop. */
/* with a simple backwards goto. */

int test13();

int test13() {
	int local1 = 0;
	loop:
		local1++;
	goto loop;
	return;
}


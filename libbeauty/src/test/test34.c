/* A very simple function to test one function calling another. */

int test34(int value1 );

int test34(int value1 ) {
	int local;
	local = value1 * 2;	
	return local;
}

int main() {
	return test34(0x100);
}


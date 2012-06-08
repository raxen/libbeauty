/* A very simple function to split program flow, twice. */

int test42(int var1, int var2)
{
	int n;
	if (var1 == 1) {
		for(n = 0; n < 10; n++) {
			var2 = var2 + 1;
		}
	}
	return var1 + var2;
}	


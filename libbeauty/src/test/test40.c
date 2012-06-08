/* A very simple function to split program flow, twice. */

int test40(int var1, int var2, int var3)
{
	if (var1 == 1) {
		var2 = var2 + 1;
	} else {
		var2 = var2 + 2;
	}
	if (var3 == 2) {
		var2 = var2 + 3;
	} else {
		var2 = var2 + 4;
	}
	return var2;
}	


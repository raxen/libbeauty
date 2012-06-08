/* A very simple function to split program flow, twice. */

int test41(int var1, int var2)
{
	var2 = var2 + 2;
	if (var1 == 1) {
//		var2 = var2 + 3;
		var2 = var2 + 4;
	} else {
		var1 = 2;
	}
	return var1 + var2;
}	


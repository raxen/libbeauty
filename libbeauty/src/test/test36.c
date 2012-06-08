/* A very simple function to split program flow. */

int test36(int var1, int var2)
{
	if (var1 == 2) {
		var2 ++;
	} else {
		var2 = var2 + 4;
	}
	var2 ++;
	return var2;
}	


/* A very simple function to test one call param. */

int test37(int *value1 );

int test37(int *value1 )
{
	*value1 = 1;
	return 10;
}

int main()
{
	int value1;

	return test37(&value1);
}


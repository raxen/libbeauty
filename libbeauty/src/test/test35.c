/* A very simple function to test multiple register parameters. */

int test35(int *value1, int *value2, int *value3, int *value4, int *value5, int *value6, int *value7, int *value8 );

int test35(int *value1, int *value2, int *value3, int *value4, int *value5, int *value6, int *value7, int *value8 )
{
	*value1 = 1;
	*value2 = 2;
	*value3 = 3;
	*value4 = 4;
	*value5 = 5;
	*value6 = 6;
	*value7 = 7;
	*value8 = 8;
	return 10;
}

int main()
{
	int value1;
	int value2;
	int value3;
	int value4;
	int value5;
	int value6;
	int value7;
	int value8;

	return test35(&value1, &value2, &value3, &value4, &value5, &value6, &value7, &value8);
}


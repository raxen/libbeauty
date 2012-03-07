/* A very simple function to test pointer params. */

int test38(int *value1 );

int test38(int *value1 )
{
	*value1 = 1;
	return 10;
}

int main()
{
	int value1;

	return test38(&value1);
}


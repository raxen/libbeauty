/* To test multiple functions in a single .o file. */

int test21a ( int value );
int test21b ( int value );

int test21a ( int value ) {
  return value+0x123;
}

int test21b ( int value ) {
  return value+0x100;
}


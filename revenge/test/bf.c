#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <bfd.h>
#include <errno.h>

#include "bfl.h"

int main() {
  const char *file="test.obj";
  struct rev_eng *handle;
  handle = bf_test_open_file(file);
  printf("handle=%p\n",handle);
  bf_test_close_file(handle);
  return 0;
}

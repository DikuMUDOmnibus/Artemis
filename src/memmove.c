#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
/*
 * #!@$ SunOS doesn't have memmove...hence this quick & dirty
 * version of it...
 */

void *memmove(void *s1, const void *s2, size_t n) {
  void *tmp_ptr;

  tmp_ptr = malloc(n);
  memcpy(tmp_ptr, s2, n);
  memcpy(s1, tmp_ptr, n);
  free(tmp_ptr);
  return s1;
}

#include <stdio.h>
#define write(x) printf("%d\n", x)
int main() {
  int a[1000000];
  int b;
  b = 0;
  a[123] = 123;
  printf("%d\n", b);
  printf("%d\n", a[123]);
}

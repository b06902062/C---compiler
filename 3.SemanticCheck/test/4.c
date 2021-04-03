#include <stdio.h>
#define write printf
int a, b;

void IntArith() {
  write("%d", a); write("\n");
  write("%d", b); write("\n");
  write("%d", a + b); write("\n");
  write("%d", a - b); write("\n");
  write("%d", a * b); write("\n");
  write("%d", a / b); write("\n");
  write("%d", a && b); write("\n");
  write("%d", a || b); write("\n");
  write("%d", !a); write("\n");
  write("%d", !b); write("\n");
  write("%d", -a); write("\n");
  write("%d", -b); write("\n");
}

int main() {
  a = 123;
  b = 456; 
  IntArith();
  a = 0;
  b = 1;
  IntArith();
  a = -1;
  b = 123;
  IntArith();
  a = 7777;
  b = -66;
  IntArith();
}

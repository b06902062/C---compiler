#include<stdio.h>

int a[123123];
int c;
int b[456456];
int d;
int e[789789];

void write(int a){
    printf("%d\n", a);
}

void f() {
  a[123] = 321;
}

void g() {
  c = 321321;
}

void h() {
  b[456] = 654;
}

void i() {
  d = 654654;
}

void j() {
  e[789] = 987;
}

int main() {
  write(a[123]);
  write(c);
  write(b[456]);
  write(d);
  write(e[789]);
  f();
  write(a[123]);
  write(c);
  write(b[456]);
  write(d);
  write(e[789]);
  g();
  write(a[123]);
  write(c);
  write(b[456]);
  write(d);
  write(e[789]);
  h();
  write(a[123]);
  write(c);
  write(b[456]);
  write(d);
  write(e[789]);
  i();
  write(a[123]);
  write(c);
  write(b[456]);
  write(d);
  write(e[789]);
  j();
  write(a[123]);
  write(c);
  write(b[456]);
  write(d);
  write(e[789]);
}

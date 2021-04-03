int ga[10];
float gf[32];

int MAIN () {
  int a[8];
  float f[21];

  write("enter ga[6]:\n");
  ga[6] = read();
  write("enter gf[17]:\n");
  gf[17] = fread();
  write("enter a[3]:\n");
  a[3] = read();
  write("enter f[19]:\n");
  f[19] = fread();

  write("ga[6]: ");
  write(ga[6]);
  write("\n");
  write("gf[17]: ");
  write(gf[17]);
  write("\n");
  write("a[3]: ");
  write(a[3]);
  write("\n");
  write("f[19]: ");
  write(f[19]);
  write("\n");
  return 0;
}
float delta = 1.0;


int floor(float num)
{
  int temp;
  write("num in floor: ");
  write(num);
  write("\n");
  temp = num;
  return temp;
}

int cool2(int a, int b, float c) {
  write(a);
  write(b);
  write(c);
  return 0;
}


int ceil(float num)
{
 int temp;
 float dumb;
  write("num in ceil: ");
  write(num);
  write("\n");
 if (num > 0) 
  {
    temp = num + delta;
  }
 else 
   {
     temp = num - delta;
   }
 return temp;
}


int MAIN()
{
  float num;
  float midp;

  write("Enter number :"); 
  
  num = fread(); 
  write("read num: ");
  write(num);
  write("\n");
  
  write(ceil(num));
  write("\n");
  write(floor(num));
  write("\n");
  midp = (ceil(num) + floor(num))/2.0;
  write(midp);
  write("\n");

  return 0;
}



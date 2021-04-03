#include<stdio.h>
#define write printf

int is_prime(int n) 
{
  int k, limit;
  if (n == 2) 
    return 1;
  if ((n/2 * 2)== 0) 
    return 0; 
  limit = n / 2;
 
  for ( k = 3; k <= limit; k = k + 2)
    if ((n - (n/k * k)) == 0)
      return 0;
  return 1;
}

int main()
{
  int i, num1, num2;
  
  write("enter a range, for example, 5<ENTER> 23<ENTER>:");
  num1 = 1;
  num2 = 20;
 
  for (i =num1; i < num2; i = i + 1)
    {
      write("%d", i);
      if (is_prime(i)) {
	write (" is prime");
	write ("\n");
      }
      else{
        write (" is not prime");
	write ("\n");
      }
    } 
  return 0; 
}


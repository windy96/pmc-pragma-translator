#include <stdio.h>
#include <omp.h>

#define	N 1000

int  A[N];
int  total;
int  result;

int main(int argc, char *argv[])
{
  int B[N];
  int i, j;

  for (i = 0; i < N; i++)
    A[i] = 0;

  #pragma omp parallel for
  for (i = 0; i < N; i++)
    B[i] = i;

  #pragma omp parallel for
  for (i = 0; i < N; i++)
    A[i] = A[i] + B[i];

  total = 0;
  for (i = 0; i < N; i++)
    total =+ A[i];

  #pragma omp parallel for
  for (i = 0; i < N; i++)
    B[i] = total - A[i]; 

  result = 0;
  for (i = 0; i < N; i++)
    result += B[i];

  printf("total = %d, result = %d\n", total, result);
  return 0;
}

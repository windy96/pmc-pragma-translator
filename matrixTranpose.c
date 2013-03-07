#include <stdio.h>
#define N 1024

double A[N][N];
double B[N][N];


void matrixTranspose(double *pDst, double *pSrc)
{
  int i, j;
  #pragma pmc write_dense pDst

  for (i = 0; i < N; i++) {
    #pragma pmc write_first pDst[i*N:(i+1)*N-1]
    #pragma pmc write_once pDst[i*N:(i+1)*N-1]
    for (j = 0; j < N; j++) {
      pDst[i*N+j] = pSrc[j*N+i];
    }
  }
}



int main() 
{
  double sum = 0;
  int i, j;

  matrixTranspose((double *) B, (double *) A);
  
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++) {
      sum += A[i][j];
    }
  }
  printf("sum = %lf\n", sum);

  return 0;
}

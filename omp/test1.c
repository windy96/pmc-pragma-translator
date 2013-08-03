int global1, global2;
int globalArray[100];
int *pGlobal;

int foo(int a, int b)
{
	int c;
	c = a + b;
	return c;
}

void baz(int a)
{
	int i;
	#pragma omp parallel for
	for (i = 0; i < 10; i++)
		globalArray[i]++;
}

void bar(int a)
{
	baz(a);
}



int main()
{
	int local1, local2, local3, local4;
	int *p, *q;
	double d;
	
	local1 = 3;
	global1 = global2 + 3;
	p = &local1;
	if (global2 == 1)
	  local3 = local2++;
	else
		local4 = 0;
	//*p = 3;
	pGlobal = &local1;
	//*pGlobal = 2;

	int i, j;
	#pragma omp parallel for
	for (i = 0; i < 50; i++)
		globalArray[i] = i;

	#pragma omp parallel for
	for (i = 0; i < 50; i++)
	{
		for (j = 0; j < 10; j++) {
			globalArray[i] = i;
			globalArray[i] += i;
			globalArray[i] += i;
		}
	}

	for (i = 0; i < 10; i++) {
		#pragma omp parallel
		{
		global1 += foo(local1, local2);
		int local5 = 1;
		local5 += foo(4, 1);
		}
	}

	for (i = 0; i < 20; i++) {
		global2 += 1;
	}

	local4 += foo(3, 2);
	bar(1);
}


int global1, global2;
#pragma pmc shared (globalArray)
int globalArray[100];
int *pGlobal;


#pragma pmc shared a
#pragma pmc shared b
int foo(int a, int b)
{
	int c;
	c = a + b;
	return c;
}

#pragma pmc shared a, b
#pragma pmc readonly a, b
int bar(int a, int b)
{
	int c;
	c = a + b;
	return c;
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
	  local2++;
	else
		local4 = 0;
	//*p = 3;
	pGlobal = &local1;
	//*pGlobal = 2;

	int i;
	#pragma pmc rwshared globalArray
	for (i = 0; i < 50; i++)
		globalArray[i] = i;

	#pragma pmc readonly globalArray
	for (i = 0; i < 50; i++)
		local1 += globalArray[i];

	#pragma pmc rwshared globalArray
	for (i = 0; i < 50; i++)
		globalArray[i]++;


	{
	global1 += foo(local1, local2);
	int local5 = 1;
	local5 += foo(4, 1);
	}
}


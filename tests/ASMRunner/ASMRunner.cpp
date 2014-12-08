// ASMRunner.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "malloc.h"
extern "C" void asm_main();

extern "C" void put(int x)
{
	printf("%d\n",x);
}

extern "C" void* memalloc(int nBytes)
{
	void* x = malloc(nBytes);
	//printf("Allocating %d bytes at address %d\n", nBytes,(int)x);
	return x;
}

int _tmain(int argc, _TCHAR* argv[])
{
	asm_main();
	scanf("a");
	return 0;
}




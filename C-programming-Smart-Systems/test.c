#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#include <time.h>
#include <conio.h>
#include <windows.h>


int askNumbers()
{
	int i1, i2;
	scanf_s("%d", &i1);

	while (getchar() != '\n') {};

	return i1;
}

int mains()
{
	while (1)
	{
		printf("firstNumber: %d", askNumbers());

	}
}
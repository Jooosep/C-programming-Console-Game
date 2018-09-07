#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#include <time.h>
#include <conio.h>
#include <windows.h>



int askNumbers()
{
	int result;
	int i;
	do
	{
		char number[10];
		fgets(number, 10, stdin);
		result = sscanf_s(number, "%d", &i);
	} while (result < 1);

	return i;
}

void ggotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

int A = 3;
int B = 3;
int C = 3;
int key = 0;
int round;

int mains()
{
	while (1)
	{
		if (round == 0)
		{
			printf("Hello lets go!\n");
			round++;
		}
		else if (round == 1)
		{
			gotoxy(20, 15);
			printf("1) add 1 to A\n");
			gotoxy(20, 16);
			printf("2) add 1 to B\n");
			gotoxy(20, 17);
			printf("3) add 1 to C\n");

			for (int i = 0; i < 1; i++)
			{
				int c = askNumbers();

				if (c == 1)
				{
					A += 1;
				}
				else if (c == 2)
				{
					B += 1;
				}
				else if (c == 3)
				{
					C += 1;
				}
				else
					i--;
			}
			round++;
		}
		else if (round == 2)
		{
			gotoxy(20, 15);
			printf("1) add 1 to A\n");
			gotoxy(20, 16);
			printf("2) add 1 to B\n");
			gotoxy(20, 17);
			printf("3) add 1 to C\n");

			for (int i = 0; i < 1; i++)
			{
				int c = askNumbers();

				if (c == 1)
				{
					A += 1;
				}
				else if (c == 2)
				{
					B += 1;
				}
				else if (c == 3)
				{
					C += 1;
				}
				else
					i--;
			}
			round++;
		}
		else if (round == 3)
		{
			gotoxy(20, 15);
			printf("1) add 1 to A\n");
			gotoxy(20, 16);
			printf("2) add 1 to B\n");
			gotoxy(20, 17);
			printf("3) add 1 to C\n");

			for (int i = 0; i < 1; i++)
			{
				int c = askNumbers();

				if (c == 1)
				{
					A += 1;
				}
				else if (c == 2)
				{
					B += 1;
				}
				else if (c == 3)
				{
					C += 1;
				}
				else
					i--;
			}
			round++;
		}
		else
		{
			gotoxy(20, 19);
			printf("A:%d, B:%d, C: %d", A, B, C);
			round++;
		}
		Sleep(2000);
		system("CLS");
	}
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* lightning(int* count)
{
	int xPos = 10 + *count ;
	int yPos = 2;

	for (int i = 0; i < 30; i++)
	{
		pthread_mutex_lock(&mutex);
		gotoxy(xPos, yPos);
		printf("|");
		pthread_mutex_unlock(&mutex);
		yPos++;
		Sleep(30);
	}
	Sleep(5000);
	printf("\n%d, %d\n", xPos, *count);

	pthread_exit(0);
}

mains()
{
	Sleep(2000);
	hidecursor();
	pthread_t thread[30];
	int retVal[30];
	int count[30];
	for (int i = 0; i < 30; i++)
	{
		count[i] = 0;
	}

	for (int i = 0; i < 30; i++)
	{
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&thread[i], &attr, lightning, &count[i]);
		for (int c = i; c < 30; c++)
		{
			count[c] += 5;
		}
	}
	for (int i = 0; i < 30; i++)
	{
		retVal[i] = pthread_join(thread[i], NULL);
	}
	while (1);
}
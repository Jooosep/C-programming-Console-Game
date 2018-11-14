#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <pthread.h>

#define FLUSH_STDIN(x) {if(x[strlen(x)-1]!='\n'){do fgets(Junk,16,stdin);while(Junk[strlen(Junk)-1]!='\n');}else x[strlen(x)-1]='\0';}
#define MIN_X 0
#define MIN_Y 2
#define MAX_X 237
#define MAX_Y 62
#define HEIGHT 50
#define WIDTH 237
#define WALL "|"
#define SIDE_WALL "||"
#define LIGHTNING_RANGE 100

char Junk[16]; // buffer for discarding excessive user input, 
			   // used by "FLUSH_STDIN" macro

struct bitFieldVars
{
	UINT8 agg : 1;
	UINT8 thunderVolume : 4;
	UINT8 thunderLaunched : 1;
	UINT8 move : 1;
	UINT8 direction : 1;
	UINT8 death : 1;
	UINT8 step : 4;
	UINT8 shield : 1;
	UINT8 sprint : 1;
	UINT8 sprintSpeed : 6;
	UINT8 tree : 4;

	UINT8 lightningRod : 1;
	UINT8 lightningRodUnlocked : 1;
	UINT8 aegisPerk : 1;
	UINT8 shieldThrow : 1;
	UINT8 lightningRunnerPerk : 1;
	UINT8 channelerPerk : 1;
	UINT8 gardenerPerk : 1;
	UINT8 harvesterPerk : 1;
	UINT8 timeSlowPerk : 1;
	UINT8 ancientTreePerk : 1;
	UINT8 repelShieldPerk : 1;
	UINT8 ancientTreeFruit : 1;

};

struct windowSize {
	unsigned short width;
	unsigned short height;
	unsigned short max_y;
};
struct windowSize w;

struct modes
{
	UINT8 botMode : 1; //0 = player, 1 = bot
	UINT8 gameMode : 2; //1 = normal, 0 = survival, 2 = chaotic
};
struct modes m;

struct lightningVars {
	signed short travelPoints[12];
	signed short thunderLaunchPoints[12];
	unsigned short thunderLaunches[12];
	int thunderLaunchExtraDelays[12];
	unsigned short thunderProgresses[12];
	signed short thunderPath[12][59];
	unsigned short thunderProgress;
	UINT8 thunderSpeed;
	int thunderFrequency;
};

struct shieldVars
{
	int shieldDuration;
	int shieldCooldown;
};

struct treeVars
{
    UINT8 treeMaxHealth;
    signed short treePosition[5];
    signed char treePhase[5];
    int treeCooldown;
    UINT8 treeHealth[5];
    UINT8 treeFruit[5];
};

struct heroVars {
	unsigned short currentPosition;
	unsigned short points;
	UINT8 runDelay;
	unsigned short thunderGuard;
};

struct sprintVars
{
	int sprintDownTime;
	int sprintDuration;
	int sprintCooldown;
};


struct rodVars
{
	int rodCooldown;
	int rodDuration;
	int rodPosition;
};

struct clocks
{
	clock_t pressDelay, moveDelay, thunderPause, thunderPauses[12], thunderDelay, thunderDelays[12];
	clock_t treeDownTime, treeTimer[5], ancientTreeTimer, treeDelay, treeDamageDelay[5];
	clock_t rodDowntime, statsDelay, sprintDelay, sprintTimer, throwDelay, shieldDelay, shieldPause;
};

void hidecursor()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void erase(UINT8 ch)
{
	for (char i = 0; i < ch; i++)
	{
		printf("\b \b");
	}
}

void recordPath(int i, int direction, struct lightningVars *lv)
{
	if (m.gameMode == 1)
	{
		gotoxy(lv->travelPoints[i], lv->thunderProgress);
		lv->thunderPath[i][lv->thunderProgress - 3] = lv->travelPoints[i];
		if (direction == 0)
			printf("\\\n");
		else if (direction == 1)
			printf("/\n");
		else
			printf("|\n");
	}
	else
	{
		gotoxy(lv->travelPoints[i], lv->thunderProgresses[i]);
		lv->thunderPath[i][lv->thunderProgresses[i] - 3] = lv->travelPoints[i];
		if (direction == 0)
			printf("\\\n");
		else if (direction == 1)
			printf("/\n");
		else
			printf("|\n");
	}
}

void clearThunder(int vol, unsigned short thunderPath[12][59])
{
	for (int i = 0; i < vol; i++)
	{
		for (int t = 0; t < 59; t++)
		{
			gotoxy(thunderPath[i][t] + 1, t + 3);
			printf("\b \b");
		}
	}
}

void clearSpecificThunder(int i, unsigned short thunderPath[12][59])
{
	for (int t = 0; t < 59; t++)
	{
		gotoxy(thunderPath[i][t] + 1, t + 3);
		printf("\b \b");
	}
}

void buildRod(int rodPosition)
{
	char charray[5] = { 0, 193, 181, 198, 216 };
	for (int i = 1; i < 5; i++)
	{
		gotoxy(rodPosition, w.max_y - i);
		printf("%c", charray[i]);
	}
}

void clearRod(int rodPosition)
{
	for (int i = 1; i < 5; i++)
	{
		gotoxy(rodPosition + 1, w.max_y - i);
		printf("\b \b");
	}

}

int aegisThrow( struct lightningVars *lv, struct bitFieldVars *bfv, struct heroVars *hv)
{
	int thunderGuardGained = 0;

	if (bfv->direction == 0)
	{
		gotoxy(hv->currentPosition, w.max_y - 4);
		printf("O");
		gotoxy(hv->currentPosition - 1, w.max_y - 3);
		printf("%c|", 205);
		gotoxy(hv->currentPosition - 1, w.max_y - 2);
		printf("< \\");
		
		for (int i = hv->currentPosition - 2; i > 1; i--)
		{
			gotoxy(i, w.max_y - 3);
			printf("(");
			for (int p = 0; p < hv->currentPosition - 2 - i; p++)
			{
				printf("%c", 196);
			}
			Sleep(2);
		}

	}
	else
	{
		gotoxy(hv->currentPosition, w.max_y - 4);
		printf("O");
		gotoxy(hv->currentPosition, w.max_y - 3);
		printf("|%c", 205);
		gotoxy(hv->currentPosition - 1, w.max_y - 2);
		printf("/ >");

		for (int i = hv->currentPosition + 2; i < w.width; i++)
		{
			gotoxy(hv->currentPosition + 2, w.max_y - 3);
			for (int p = hv->currentPosition + 2; p < i; p++)
			{
				printf("%c", 196);
			}
			gotoxy(i, w.max_y - 3);
			printf(")");
			Sleep(2);
		}
	}

	if (m.gameMode == 1)
	{
		if (lv->thunderProgress > w.height - 15)
		{
			for (int i = 0; i < bfv->thunderVolume; i++)
			{
				if ((bfv->direction == 0) ? ((lv->travelPoints[i] < hv->currentPosition)) : ((lv->travelPoints[i] > hv->currentPosition)))
				{
					thunderGuardGained++;
					for (int t = lv->thunderProgress; t < w.max_y; t++)
					{
						gotoxy(lv->travelPoints[i], t);
						printf("|");
						Sleep(3);
					}
				}
			}

			for (int i = 0; i < bfv->thunderVolume; i++)
			{
				if ((bfv->direction == 0) ? ((lv->travelPoints[i] < hv->currentPosition)) : ((lv->travelPoints[i] > hv->currentPosition)))
				{
					for (int t = lv->thunderProgress; t < w.max_y; t++)
					{
						gotoxy(lv->travelPoints[i] + 1, t);
						printf("\b \b");
						Sleep(3);
					}
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < bfv->thunderVolume; i++)
		{
			if (lv->thunderProgresses[i] > 50)
			{
				if ((bfv->direction == 0) ? ((lv->travelPoints[i] < hv->currentPosition)) : ((lv->travelPoints[i] > hv->currentPosition)))
				{
					thunderGuardGained++;
					for (int t = lv->thunderProgresses[i]; t < w.max_y; t++)
					{
						gotoxy(lv->travelPoints[i], t);
						printf("|");
						Sleep(3);
					}
				}
			}
		}
		for (int i = 0; i < bfv->thunderVolume; i++)
		{
			if (lv->thunderProgresses[i] > 50)
			{
				if ((bfv->direction == 0) ? ((lv->travelPoints[i] < hv->currentPosition)) : ((lv->travelPoints[i] > hv->currentPosition)))
				{
					for (int t = lv->thunderProgresses[i]; t < w.max_y; t++)
					{
						gotoxy(lv->travelPoints[i] + 1, t);
						printf("\b \b");
						Sleep(2);
					}
				}
			}
		}
	}
	if (bfv->direction == 0)
	{
		for (int i = 1; i < hv->currentPosition - 2; i++)
		{
			gotoxy(i + 1, w.max_y - 3);
			printf("\b \b");
			gotoxy(i + 1, w.max_y - 3);
			printf("(");
			Sleep(2);
		}
	}
	else
	{
		for (int i = w.width - 1; i > hv->currentPosition + 2; i--)
		{
			gotoxy(i - 2, w.max_y - 3);
			printf(")");
			gotoxy(i, w.max_y - 3);
			printf("\b \b");
			Sleep(2);
		}
	}
	return thunderGuardGained;
}
void clearTree(int pos)
{
	for (int t = 1; t < 6; t++)
	{
		gotoxy(pos + 5, w.max_y - t);
		for (int i = 0; i < 7; i++)
		{
			printf("\b \b");
		}
	}
}

void ancientTree( struct bitFieldVars *bfv , int ancientTreeTimer)
{
	int pos = w.width / 2;
	gotoxy(pos - 1, w.max_y - 1);
	printf("%c%c", 188, 200);
	gotoxy(pos - 1, w.max_y - 2);
	printf("%c%c", 222, 221);
	gotoxy(pos - 1, w.max_y - 3);
	if (bfv->harvesterPerk && clock() - ancientTreeTimer > 9999)
	{
		printf("%c%c%c", 222, 221, 235);
		gotoxy(pos - 2, w.max_y - 3);
		printf("%c",235);
		bfv->ancientTreeFruit = 1;
	}
	else
	{
		printf("%c%c", 222, 221);
	}
	gotoxy(pos - 4, w.max_y - 4);
	printf("%c%c%c%c%c%c%c%c", 178, 178, 178, 178, 178, 178, 178, 178);
	gotoxy(pos - 3, w.max_y - 5);
	printf("%c%c%c%c%c%c", 178, 178, 178, 178, 178, 178);
	gotoxy(pos - 2, w.max_y - 6);
	printf("%c%c%c%c", 178, 178, 178, 178);
	gotoxy(pos - 1, w.max_y - 7);
	printf("%c%c", 178, 178);
}

void buildTree(struct bitFieldVars *bfv, int buildPhase, int treePos, int treeNumber, struct clocks *clk, struct treeVars *tv)
{
	int treeY[5];
	for (int i = 0; i < 5; i++)
	{
		treeY[i] = w.max_y - (i + 1);
	}
	int treeX[5] = { treePos - 2, treePos, treePos, treePos - 2, treePos - 1 };

	if (buildPhase < 5)
	{
		char treeArray[5][10] = { "\\\\///" , { 186 }, {186 }, { 177, 177, 177, 177, 177, 177 }, { 177, 177, 177, 177 } };
		for (int i = 0; i < buildPhase + 1; i++)
		{
			gotoxy(treeX[i], treeY[i]);
			printf("%s", treeArray[i]);
		}

		if (bfv->harvesterPerk && clock() - clk->treeTimer[treeNumber] > 16500)
		{
			gotoxy(treePos + 1, w.max_y - 3);
			printf("%c", 235);
			tv->treeFruit[treeNumber] = 1;
		}
	}
	else
	{
		tv->treeFruit[treeNumber] = 0;
		bfv->tree--;
		tv->treePosition[treeNumber] = -1;
		tv->treePhase[treeNumber] = -2;

		for (int i = 0; i < 5; i++)
		{
			gotoxy(treeX[i] + 6, treeY[i]);
			erase(6);
		}
	}
}

void speedLine(int position, struct bitFieldVars *bfv)
{
	char mSpeedLine[4][3] = { { 196, 196 },{ 196, 196 }, { 196, 196, 196 }, { '\0' } };
	char lSpeedLine[4][4] = { { 196, 196 }, { 196, 196, 196 }, { 196, 196, 196, 196}, { '\0' } };
	int startX[3] = { position , position - 1, position - 1 };
	int startY[3] = { w.max_y - 4 , w.max_y - 3, w.max_y - 2 };
	if (bfv->direction == 1)
	{
		if (bfv->sprintSpeed == 10)
		{
			for (int i = 0; i < 3; i++)
			{
				int l = sizeof(mSpeedLine[i]) / sizeof(char);
				gotoxy(startX[i] - l, startY[i]);
				printf("%s", mSpeedLine[i]);
			}
		}
		else if (bfv->sprintSpeed == 5)
		{
			for (int i = 0; i < 3; i++)
			{
				int l = sizeof(lSpeedLine[i]) / sizeof(char);
				gotoxy(startX[i] - l, startY[i]);
				printf("%s", lSpeedLine[i]);
			}
		}
	}
	else
	{
		int startX[3] = { position , position + 1, position + 1 };
		if (bfv->sprintSpeed == 10)
		{
			for (int i = 0; i < 3; i++)
			{
				gotoxy(startX[i], startY[i]);
				printf("%s", mSpeedLine[i]);
			}
		}
		else if (bfv->sprintSpeed == 5)
		{
			for (int i = 0; i < 3; i++)
			{
				gotoxy(startX[i], startY[i]);
				printf("%s", lSpeedLine[i]);
			}
		}
	}
}

void moveHero( struct bitFieldVars *bfv, int position )
{

	if (bfv->shield == 1)
	{
		gotoxy(position, w.max_y - 5);
		printf("%c", 239);
		gotoxy(position - 1, w.max_y - 4);
		printf("\\O/");
		gotoxy(position , w.max_y - 3);
		printf("|");
		gotoxy(position - 1, w.max_y - 2);
		printf("/ \\");
	}
	else if (!bfv->move && !bfv->death)
	{
		gotoxy(position, w.max_y - 4);
		printf("O");
		gotoxy(position - 1, w.max_y - 3);
		printf("/|\\");
		gotoxy(position - 1, w.max_y - 2);
		printf("/ \\");
	}
	else 
	{
		if (bfv->death)
		{
			if (bfv->step % 2 == 0)
			{
				gotoxy(position, w.max_y - 4);
				printf("@");
				gotoxy(position - 1, w.max_y - 3);
				printf("%c%c%c", 196, 157, 196);
				gotoxy(position - 1, w.max_y - 2);
				printf("%c \\", 159);
				bfv->step++;
			}
			else
			{
				gotoxy(position, w.max_y - 4);
				printf("O");
				gotoxy(position - 1, w.max_y - 3);
				printf("/|\\");
				gotoxy(position - 1, w.max_y - 2);
				printf("/ \\");
				bfv->step++;
			}
			if (bfv->step > 10)
			{
				bfv->step = 0;
			}
		}
		else if (bfv->direction == 1)
		{
			if (bfv->sprint)
			{
				speedLine(position, bfv);
			}

			gotoxy(position, w.max_y - 4);
			printf(" O");
			gotoxy(position - 1, w.max_y - 3);
			printf("//");
			gotoxy(position - 1, w.max_y - 2);
			if (bfv->step % 10 < 5)
			{
				printf(" />");
				bfv->step++;
			}
			else
			{
				printf("%c\\ ", 217);
				bfv->step++;
			}

			if (bfv->step > 9)
			{
				bfv->step = 0;
			}

		}
		else if (bfv->direction == 0)
		{
			if (bfv->sprint)
			{
				speedLine(position, bfv);
			}
			gotoxy(position - 2, w.max_y - 4);
			printf("O");
			gotoxy(position - 1, w.max_y - 3);
			printf("\\\\");
			gotoxy(position - 1, w.max_y - 2);
			if (bfv->step % 10 < 5)
			{
				printf("|%c", 192);
				bfv->step++;
			}
			else
			{
				printf("<\\ ");
				bfv->step++;
			}
		}
	}
}

void gameBorders()
{
	gotoxy(MIN_X, MIN_Y);
	for (int i = 0; i < w.width; i++)
	{
		printf(WALL);
	}

	gotoxy(MIN_X, w.max_y);
	for (int i = 0; i < w.width; i++)
	{
		printf(WALL);
	}
}
void thunderAsync(int i, struct lightningVars *lv, struct bitFieldVars *bfv, struct heroVars *hv, struct treeVars *tv, struct rodVars *rv)
{
	int closestTree = 1000;
	int closestTreePos = -1000;
	int random[12];

	random[i] = (rand() % LIGHTNING_RANGE);
	lv->thunderLaunchPoints[i] = random[i] + hv->currentPosition - (LIGHTNING_RANGE / 2);
	if (lv->thunderLaunchPoints[i] < 0)
	{
		lv->thunderLaunchPoints[i] = abs(lv->thunderLaunchPoints[i]);
	}
	if (lv->thunderLaunchPoints[i] > w.width - 2)
	{
		lv->thunderLaunchPoints[i] = (w.width - 1) - lv->thunderLaunchPoints[i] % (w.width - 2);
	}

	if (bfv->repelShieldPerk && bfv->shield && lv->thunderProgress >(int)(0.76 * w.max_y) && abs(hv->currentPosition - lv->travelPoints[i] < 60))
	{
		if (hv->currentPosition < lv->travelPoints[i] && lv->travelPoints[i] < w.width - 2)
		{
			lv->travelPoints[i] += 1;
			recordPath(i, 0, lv);
		}
		else if (hv->currentPosition > lv->travelPoints[i] && lv->travelPoints[i] > 2)
		{
			lv->travelPoints[i] -= 1;
			recordPath(i, 1, lv);
		}
		else
		{
			if (hv->currentPosition > w.width / 2)
			{
				lv->travelPoints[i] -= 1;
				recordPath(i, 1, lv);
			}
			else
			{
				lv->travelPoints[i] += 1;
				recordPath(i, 0, lv);
			}
		}
	}
	else if (lv->thunderProgress > (int)(0.48 * w.max_y) && bfv->lightningRod && abs(rv->rodPosition - lv->travelPoints[i]) < 80)
	{

		if (rv->rodPosition > lv->travelPoints[i])
		{
			lv->travelPoints[i] += 1;
			recordPath(i, 0, lv);
		}
		else if (rv->rodPosition < lv->travelPoints[i])
		{
			lv->travelPoints[i] -= 1;
			recordPath(i, 1, lv);
		}
		else {
			recordPath(i, 2, lv);
		}

	}
	else if (lv->thunderProgress > (int)(0.76 * w.max_y))
	{
		for (int t = 0; t < 5; t++)
		{
			if (tv->treePhase[t] > 2 && tv->treePhase[t] < 12 && abs(tv->treePosition[t] - lv->travelPoints[i]) < closestTree)
			{
				closestTree = abs(tv->treePosition[t] - lv->travelPoints[i]);
				closestTreePos = tv->treePosition[t];
			}

		}
		if (bfv->ancientTreePerk)
		{
			closestTreePos = w.width / 2;
		}

		if (abs(closestTreePos - lv->travelPoints[i]) < 70)
		{
			if (closestTreePos > lv->travelPoints[i])
			{
				lv->travelPoints[i] += 1;
				recordPath(i, 0, lv);
			}
			else if (closestTreePos < lv->travelPoints[i])
			{
				lv->travelPoints[i] -= 1;
				recordPath(i, 1, lv);
			}
			else {
				recordPath(i, 2, lv);
			}
		}
		else if (abs(hv->currentPosition - lv->travelPoints[i]) < 60 && !(bfv->lightningRunnerPerk && !bfv->shield))
		{

			if (hv->currentPosition > lv->travelPoints[i])
			{
				lv->travelPoints[i] += 1;
				recordPath(i, 0, lv);
			}
			else if (hv->currentPosition < lv->travelPoints[i])
			{
				lv->travelPoints[i] -= 1;
				recordPath(i, 1, lv);
			}
			else {
				recordPath(i, 2, lv);
			}
		}
		else
		{
			if (random[i] < 34 && lv->travelPoints[i] < w.width - 2)
			{
				lv->travelPoints[i] += 1;
				recordPath(i, 0, lv);
			}
			else if (random[i] < 67 && lv->travelPoints[i] > 0)
			{
				lv->travelPoints[i] -= 1;
				recordPath(i, 1, lv);
			}
			else
			{
				recordPath(i, 2, lv);
			}
		}
	}

	else
	{
		if (lv->thunderProgress == MIN_Y + 1)
		{
			if (i == 0)
				lv->travelPoints[i] = lv->thunderLaunchPoints[i];
			else
			{
				lv->travelPoints[i] = lv->thunderLaunchPoints[i] + (i * (rand() % 200)) - (i * 100);
				if (lv->travelPoints[i] > w.width)
				{
					lv->travelPoints[i] = lv->travelPoints[i] % w.width;
				}
				else if (lv->travelPoints[i] < MIN_X)
				{
					lv->travelPoints[i] = abs(lv->travelPoints[i]) % w.width;
				}
			}
		}
		if (random[i] < 34 && lv->travelPoints[i] < w.width - 2)
		{
			lv->travelPoints[i] += 1;
			recordPath(i, 0, lv);
		}
		else if (random[i] < 67 && lv->travelPoints[i] > 0)
		{
			lv->travelPoints[i] -= 1;
			recordPath(i, 1, lv);
		}
		else
		{
			recordPath(i, 2, lv);

		}
	}
}

void thunder( struct lightningVars *lv , struct bitFieldVars *bfv, struct heroVars *hv, struct treeVars *tv, struct rodVars *rv)
{
	int closestTree = 1000;
	int closestTreePos = -1000;
	int random[12];

	for (int i = 0; i < bfv->thunderVolume; i++)
	{
		random[i] = (rand() % LIGHTNING_RANGE);
		lv->thunderLaunchPoints[i] = random[i] + hv->currentPosition - (LIGHTNING_RANGE / 2);
		if (lv->thunderLaunchPoints[i] < 0)
		{
			lv->thunderLaunchPoints[i] = abs(lv->thunderLaunchPoints[i]);
		}
		if (lv->thunderLaunchPoints[i] < 2)
		{
			lv->thunderLaunchPoints[i] = 2;
		}
		if (lv->thunderLaunchPoints[i] > w.width - 2)
		{
			lv->thunderLaunchPoints[i] = (w.width - 2) - lv->thunderLaunchPoints[i] % (w.width - 2);
		}

		if (lv->thunderProgress == MIN_Y + 1)
		{
			if (i == 0)
				lv->travelPoints[i] = lv->thunderLaunchPoints[i];
			else
			{
				lv->travelPoints[i] = lv->thunderLaunchPoints[i] + (i * (rand() % 200)) - (i * 100);
				if (lv->travelPoints[i] > w.width)
				{
					lv->travelPoints[i] = lv->travelPoints[i] % w.width;
				}
				else if (lv->travelPoints[i] < MIN_X)
				{
					lv->travelPoints[i] = abs(lv->travelPoints[i]) % w.width;
				}
			}
		}
		
	}

	for (int i = 0; i < bfv->thunderVolume; i++)
	{
		if (bfv->repelShieldPerk && bfv->shield && lv->thunderProgress > (int)(0.76 * w.max_y) && abs(hv->currentPosition - lv->travelPoints[i] < 60))
		{
			if (hv->currentPosition < lv->travelPoints[i] && lv->travelPoints[i] < w.width - 2)
			{
				lv->travelPoints[i] += 1;
				recordPath(i, 0, lv);
			}
			else if (hv->currentPosition > lv->travelPoints[i] && lv->travelPoints[i] > 2)
			{
				lv->travelPoints[i] -= 1;
				recordPath(i, 1, lv);
			}
			else
			{
				if (lv->travelPoints[i] > w.width / 2)
				{ 
					lv->travelPoints[i] -= 1;
					recordPath(i, 1, lv);
				}
				else
				{
					lv->travelPoints[i] += 1;
					recordPath(i, 0, lv);
				}
			}
		}

		else if (lv->thunderProgress > (int)(0.48 * w.max_y) && bfv->lightningRod && abs(rv->rodPosition - lv->travelPoints[i]) < 80)
		{
			if (rv->rodPosition > lv->travelPoints[i])
			{
				lv->travelPoints[i] += 1;
				recordPath(i, 0, lv);
			}
			else if (rv->rodPosition < lv->travelPoints[i] && lv->travelPoints[i] > 2)
			{
				lv->travelPoints[i] -= 1;
				recordPath(i, 1, lv);
			}
			else {
				recordPath(i, 2, lv);
			}
		}

		else if (lv->thunderProgress >(int)(0.76 * w.max_y))
		{
			for (int t = 0; t < 5; t++)
			{
				if (tv->treePhase[t] > 2 && tv->treePhase[t] < 12 && abs(tv->treePosition[t] - lv->travelPoints[i]) < closestTree)
				{
					closestTree = abs(tv->treePosition[t] - lv->travelPoints[i]);
					closestTreePos = tv->treePosition[t];
				}

			}
			if (bfv->ancientTreePerk)
			{
				closestTreePos = w.width / 2;
			}

			if (abs(closestTreePos - lv->travelPoints[i]) < 70)
			{
				if (closestTreePos > lv->travelPoints[i])
				{
					lv->travelPoints[i] += 1;
					recordPath(i, 0, lv);
				}
				else if (closestTreePos < lv->travelPoints[i] && lv->travelPoints[i] > 2)
				{
					lv->travelPoints[i] -= 1;
					recordPath(i, 1, lv);
				}
				else {
					recordPath(i, 2, lv);
				}
			}
			else if (abs(hv->currentPosition - lv->travelPoints[i]) < 60 && !(bfv->lightningRunnerPerk && !bfv->shield))
			{

				if (hv->currentPosition > lv->travelPoints[i])
				{
					lv->travelPoints[i] += 1;
					recordPath(i, 0, lv);
				}
				else if (hv->currentPosition < lv->travelPoints[i] && lv->travelPoints[i] > 2)
				{
					lv->travelPoints[i] -= 1;
					recordPath(i, 1, lv);
				}
				else {
					recordPath(i, 2, lv);
				}
			}
			else 
			{
				if (random[i] < 34 && lv->travelPoints[i] < w.width - 2)
				{
					lv->travelPoints[i] += 1;
					recordPath(i, 0, lv);
				}
				else if (random[i] < 67 && lv->travelPoints[i] > 2)
				{
					lv->travelPoints[i] -= 1;
					recordPath(i, 1, lv);
				}
				else
				{
					recordPath(i, 2, lv);
				}
			}
		}
		else
		{
			if (random[i] < 34 && lv->travelPoints[i] < w.width - 2)
			{
				lv->travelPoints[i] += 1;
				recordPath(i, 0, lv);
			}
			else if (random[i] < 67 && lv->travelPoints[i] > 2)
			{
				lv->travelPoints[i] -= 1;
				recordPath(i, 1, lv);
			}
			else
			{
				recordPath(i, 2, lv);
			}
		}
	}
}

clearHero(int currentPosition)
{
	gotoxy(currentPosition + 2, w.max_y - 1);
	for (int i = 0; i < 4; i++)
	{
		printf("\b \b");
	}
	gotoxy(currentPosition + 6, w.max_y - 2);
	for (int i = 0; i < 12; i++)
	{
		printf("\b \b");
	}
	gotoxy(currentPosition + 6, w.max_y - 3);
	for (int i = 0; i < 12; i++)
	{
		printf("\b \b");
	}
	gotoxy(currentPosition + 3, w.max_y - 4);
	for (int i = 0; i < 8; i++)
	{
		printf("\b \b");
	}
	gotoxy(currentPosition + 3, w.max_y - 5);
	for (int i = 0; i < 8; i++)
	{
		printf("\b \b");
	}
}

int askNumber()
{
	int result;
	int i;
	char number[10];
	do
	{
		fgets(number, 10, stdin);
		result = sscanf_s(number, "%d", &i);
	} while (result < 1);
	FLUSH_STDIN(number);
	return i;
}

void gameProgress(struct bitFieldVars *bfv, struct lightningVars *lv, struct shieldVars *sv, struct treeVars * tv, struct heroVars *hv, struct sprintVars * spv, struct rodVars *rv, struct clocks *clk)
{
	
	const char *choice1[4];
	const char *choice2[4];
	const char *choice3[4];
	const char *choice4[4];
	const char *choice5[4];
	const char *choice6[4];

	const char *endText[7];


	choice1[0] = "Shield cooldown -2 seconds";
	choice1[1] = "Seed cooldown -10 seconds";
	choice1[2] = "Sprint duration +10 seconds";
	choice1[3] = "";

	choice2[0] = "Shield cooldown -2 second";
	choice2[1] = "Trees become more resistant to lightning";
	choice2[2] = "Sprint duration +5 seconds";
	choice2[3] = "Trees now grow 2 fruit";

	choice3[0] = "Trees become more resistant to lightning";
	choice3[1] = "Time slows down when you sprint";
	choice3[2] = "Seed cooldown -6 seconds";
	choice3[3] = "Unlock the mighty lightning rod";

	choice4[0] = "Upgrade sprint speed and duration +5 seconds";
	choice4[1] = "Trees become more resistant to lightning";
	choice4[2] = "Lightning rod cooldown - 8 seconds OR unlock it if you haven't yet.";
	choice4[3] = "Sprint duration +5 seconds and enchance timeslow while sprinting";

	choice5[0] = "Throw your shield horizontally by pressing the left or right arrow key during shielding, snatch a thunderGuard from each nearby ray of lightning it passes - MASTER OF AEGIS";
	choice5[1] = "Permanently sprint and lightning can't home into you except when you shield - LIGHTNING RUNNER";
	choice5[2] = "Lightning rod cooldown - 7 seconds and refresh all other cooldowns when you plant the rod - CHANNELER";
	choice5[3] = "Trees bloom instantly into their final form and slow down approaching lightning - GARDENER";

	choice6[0] = "ThunderGuard +10";
	choice6[1] = "Shield now repels lightning instead and lasts for 3 seconds";
	choice6[2] = "Give up all seeds to grow an ancient tree that will never fall, it grants two fruit every 10 seconds";
	choice6[3] = "Trees bloom instantly into their final form and slow down approaching lightning - GARDENER";

	endText[0] = "||        |||||||||    ||||||||   |||||||||  |||     ||  ||||          |       |||||||||| ||      ||";
	endText[1] = "||        ||          ||          ||         ||||    ||  ||  ||       | |      ||      ||  ||    || ";
	endText[2] = "||        ||         ||           ||         || ||   ||  ||    ||    || ||     ||   |||     ||  ||  ";
	endText[3] = "||        |||||||||  ||  |||||||  |||||||||  ||  ||  ||  ||     ||  ||   ||    ||||||         ||    ";
	endText[4] = "||        ||         ||        || ||         ||   || ||  ||    ||  |||||||||   ||   ||        ||    ";
	endText[5] = "||        ||         ||       ||  ||         ||    ||||  ||   ||  ||       ||  ||    ||       ||    ";
	endText[6] = "||||||||| |||||||||  ||||||||||   |||||||||  ||     |||  |||||   ||         || ||     ||      ||    ";

	if (hv->points < 10)
	{
		lv->thunderSpeed--;
		lv->thunderFrequency = lv->thunderFrequency - 60;
	}
	else if (hv->points > 20 && hv->points < 22)
	{
		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		gotoxy(80, 31);
		printf("1)%s", choice1[0]);
		gotoxy(80, 32);
		printf("2)%s", choice1[1]);
		gotoxy(80, 33);
		printf("3)%s", choice1[2]);
		(m.gameMode == 1) ? (bfv->thunderVolume = 2) : (bfv->thunderVolume = 4);

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				sv->shieldCooldown = sv->shieldCooldown - 2000;

			}
			else if (c == 2)
			{
				tv->treeCooldown = tv->treeCooldown - 10000;

			}
			else if (c == 3)
			{
				spv->sprintDuration = spv->sprintDuration + 10000;

			}
			else
				i--;
		}
		system("CLS");

		gameBorders();
		clk->thunderPause = clock() + 2000;
		for (int i = 0; i < 7; i++)
		{
			clk->thunderPauses[i] = clock() + 2000;
		}
	}
	else if (hv->points > 30 && hv->points < 32)
	{
		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		for (int i = 0; i < 4; i++)
		{
			gotoxy(80, 31 + i);
			printf("%d)%s",i + 1, choice2[i]);
		}

		(m.gameMode == 1) ? (bfv->thunderVolume = 3) : (bfv->thunderVolume = 5);
		(m.gameMode == 1) ? (lv->thunderSpeed = 13) : (lv->thunderSpeed = 20);

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				sv->shieldCooldown = sv->shieldCooldown - 2000;

			}
			else if (c == 2)
			{
				tv->treeMaxHealth = tv->treeMaxHealth + 2;
			}
			else if (c == 3)
			{
				spv->sprintDuration = spv->sprintDuration + 4000;
			}
			else if (c == 4)
			{
				bfv->harvesterPerk = 2;
			}
			else
				i--;
		}
		system("CLS");

		gameBorders();
		clk->thunderPause = clock() + 2000;
		for (int i = 0; i < 7; i++)
		{
			clk->thunderPauses[i] = clock() + 2000;
		}
	}
	else if (hv->points > 44 && hv->points < 46)
	{
		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		for (int i = 0; i < 4; i++)
		{
			gotoxy(80, 31 + i);
			printf("%d)%s",i + 1, choice3[i]);
		}
		(m.gameMode == 1) ? (bfv->thunderVolume = 4) : (bfv->thunderVolume = 6);
		(m.gameMode == 1) ? (lv->thunderSpeed = 10) : (lv->thunderSpeed = 17);

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				tv->treeMaxHealth = tv->treeMaxHealth * 2;
			}
			else if (c == 2)
			{
				bfv->timeSlowPerk = 1;
			}
			else if (c == 3)
			{
				tv->treeCooldown = tv->treeCooldown - 6000;
			}
			else if (c == 4)
			{
				bfv->lightningRodUnlocked = 1;
			}
			else
				i--;
		}
		system("CLS");

		gameBorders();
		clk->thunderPause = clock() + 2000;
		for (int i = 0; i < 7; i++)
		{
			clk->thunderPauses[i] = clock() + 2000;
		}
	}
	else if (hv->points > 59 && hv->points < 61)
	{
		(m.gameMode == 1) ? (bfv->thunderVolume = 4) : (bfv->thunderVolume = 7);
		(m.gameMode == 1) ? (lv->thunderSpeed = 7) : (lv->thunderSpeed = 14);

		gotoxy(80, 30);
		for (int i = 0; i < 4; i++)
		{
			gotoxy(80, 31 + i);
			printf("%d)%s", i + 1, choice4[i]);
		}

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				bfv->sprintSpeed -= 5;
				spv->sprintDuration += 5000;
			}
			else if (c == 2)
			{
				tv->treeMaxHealth = tv->treeMaxHealth + 2;
			}
			else if (c == 3)
			{
				if (bfv->lightningRodUnlocked)
					rv->rodCooldown -= 8000;
				else
					bfv->lightningRodUnlocked = 1;
			}
			else if (c == 4)
			{
				spv->sprintDuration += 5000;
				bfv->timeSlowPerk += 1;
			}
			else
				i--;
		}
		system("CLS");

		gameBorders();
		clk->thunderPause = clock() + 2000;
		for (int i = 0; i < 7; i++)
		{
			clk->thunderPauses[i] = clock() + 2000;
		}
	}
	else if (hv->points > 79 && hv->points < 81)
	{
		(m.gameMode == 1) ? (bfv->thunderVolume = 4) : (bfv->thunderVolume = 8);
		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		for (int i = 0; i < 4; i++)
		{
			gotoxy(80, 31 + i);
			printf("%d)%s", i + 1, choice5[i]);
		}

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				sv->shieldDuration = sv->shieldDuration * 2;
				bfv->aegisPerk = 1;
			}
			else if (c == 2)
			{
				spv->sprintCooldown = 0;
				bfv->lightningRunnerPerk = 1;
				hv->runDelay = bfv->sprintSpeed;
			}
			else if (c == 3)
			{
				rv->rodCooldown -= 7000;
				bfv->channelerPerk = 1;
			}
			else if (c == 4)
			{
				bfv->gardenerPerk = 1;
				tv->treeCooldown -= 4000;
			}
			else
				i--;
		}
		(m.gameMode == 1) ? (lv->thunderSpeed = 5) : (lv->thunderSpeed = 10);
		lv->thunderFrequency = lv->thunderFrequency - 300;
		system("CLS");

		gameBorders();
		clk->thunderPause = clock() + 2000;
		for (int i = 0; i < 7; i++)
		{
			clk->thunderPauses[i] = clock() + 2000;
		}
	}
	else if (hv->points > 99 && hv->points < 101)
	{
		lv->thunderFrequency = lv->thunderFrequency - 100;
		(m.gameMode == 1) ? (bfv->thunderVolume = 5) : (bfv->thunderVolume = 9);
	}
	else if (hv->points > 119 && hv->points < 121)
	{
		lv->thunderFrequency -= 100;
	}
	else if (hv->points > 139 && hv->points < 141)
	{
		(m.gameMode == 1) ? (bfv->thunderVolume = 6) : (bfv->thunderVolume = 10);
		lv->thunderFrequency -= 100;
	}
	else if (hv->points > 149 && hv->points < 151)
	{
		gotoxy(80, 30);
		printf("Prepare for hell:");

		for (int i = 0; i < 4; i++)
		{
			gotoxy(80, 31 + i);
			printf("%d)%s", i + 1, choice6[i]);
		}


		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				hv->thunderGuard += 10;
			}
			else if (c == 2)
			{
				bfv->repelShieldPerk = 1;
				sv->shieldDuration = 3000;
			}
			else if (c == 3)
			{
				bfv->ancientTreePerk = 1;
				clk->ancientTreeTimer = clock();
			}
			else if (c == 4)
			{
				bfv->gardenerPerk = 1;
				tv->treeCooldown -= 4000;
			}
			else
				i--;
		}
		(m.gameMode == 1) ? (bfv->thunderVolume = 6) : (bfv->thunderVolume = 11);
		(m.gameMode == 1) ? (lv->thunderSpeed = 4) : (lv->thunderSpeed = 8);
		lv->thunderFrequency = lv->thunderFrequency - 200;
		system("CLS");

		gameBorders();
		PlaySound("finalRound.wav", NULL, SND_ASYNC);
		clk->thunderPause = clock() + 3000;
		for (int i = 0; i < 7; i++)
		{
			clk->thunderPauses[i] = clock() + 3000;
		}

	}
	else if (hv->points > 249 && m.gameMode)
	{
		system("CLS");
		PlaySound("BGM.wav", NULL, SND_ASYNC);
		Sleep(400);
		for (int i = 0; i < 7; i++)
		{
			gotoxy(50, 20 + i);
			printf("%s", endText[i]);
			Sleep(400);
		}
		Sleep(4000);

		gotoxy(40, 35);
		printf("YOU MAY LIVE ETERNALLY!");
		gotoxy(40, 36);
		printf("PRESS ENTER TO GO AGAIN!");
		int c;
		while ((c = getchar()) != '\n' && c != EOF);
	}
	else if (hv->points > 199 && hv->points < 201 && !m.gameMode)
	{
		lv->thunderSpeed = 3;
	}
}

initialize(struct bitFieldVars *bfv, struct lightningVars *lv, struct shieldVars *sv, struct treeVars * tv, struct heroVars *hv, struct sprintVars * spv, struct rodVars *rv, struct clocks *clk)
{


	bfv->agg = 0;
	(m.gameMode == 1) ? (bfv->thunderVolume = 1) : (bfv->thunderVolume = 3);
	(!m.gameMode) ? (bfv->thunderVolume = 5) : (1);
	bfv->thunderLaunched = 0;
	bfv->move = 0;
	bfv->direction = 0;
	bfv->death = 0;
	bfv->step = 0;
	bfv->shield = 0;
	bfv->sprint = 0;
	(!m.gameMode) ? (bfv->sprintSpeed = 5) : (bfv->sprintSpeed = 10);
	bfv->tree = 0;

	bfv->lightningRod = 0;
	bfv->lightningRodUnlocked = 0;
	bfv->aegisPerk = 0;
	bfv->shieldThrow = 0;
	(!m.gameMode) ? (bfv->lightningRunnerPerk = 1) : (bfv->lightningRunnerPerk = 0);
	bfv->channelerPerk = 0;
	bfv->gardenerPerk = 0;
	bfv->harvesterPerk = 0;
	bfv->timeSlowPerk = 0;
	bfv->ancientTreePerk = 0;
	bfv->repelShieldPerk = 0;
	bfv->ancientTreeFruit = 0;

	for (char i = 0; i < 12; i++)
	{
		lv->thunderLaunchExtraDelays[i] = 0;
		lv->thunderProgresses[i] = MIN_Y + 1;
		lv->travelPoints[i] = MIN_Y + 1;
	}
	lv->thunderProgress = MIN_Y + 1;
	(m.gameMode == 1) ? (lv->thunderSpeed = 25) : (lv->thunderSpeed = 35);
	(!m.gameMode) ? (lv->thunderSpeed = 5) : (1);
	(m.gameMode == 1) ? (lv->thunderFrequency = 1600) : (lv->thunderFrequency = 2000);
	(!m.gameMode) ? (lv->thunderFrequency = 600) : (1);

	clk->pressDelay = clock(); 
	clk->moveDelay = clock();
	clk->thunderPause = clock();
	clk->thunderDelay = clock();
	clk->treeDownTime = clock();
	clk->ancientTreeTimer = clock();
	clk->treeDelay = clock();
	clk->rodDowntime = clock();
	clk->statsDelay = clock();
	clk->sprintDelay = clock();
	clk->sprintTimer = clock();
	clk->throwDelay = clock();
	clk->shieldDelay = clock();
	clk->shieldPause = clock();

	for (char i = 0; i < 5; i++)
	{
		clk->treeDamageDelay[i] = clock();
		clk->treeTimer[i] = clock();
	}
	for (char i = 0; i < 12; i++)
	{
		clk->thunderPauses[i] = clock();
		clk->thunderDelays[i] = clock();
	}

	sv->shieldDuration = 300;
	sv->shieldCooldown = 8000;

	tv->treeMaxHealth = 2;
	tv->treeCooldown = 20000;
	for (char i = 0; i < 5; i++)
	{
		tv->treeHealth[i] = 2;
		tv->treeFruit[i] = 0;
		tv->treePhase[i] = -2;
		tv->treePosition[i] = -1;
	}

	hv->currentPosition = 100;
	hv->points = 0;
	hv->runDelay = 20;
	hv->thunderGuard = 0;
	
	spv->sprintDuration = 5000;
	spv->sprintCooldown = 20000;


	rv->rodCooldown = 30000;
	rv->rodDuration = 15000;
	rv->rodPosition = -1;


	PlaySound("thunderClap.wav", NULL, SND_ASYNC);
	system("CLS");

	gameBorders();
    moveHero(bfv, hv->currentPosition);
}

void chosenModeBorders(int mode)
{
	int xMin = 45;
	int xMax = 185;
	int borderLength = 140;
	if (mode == 1)
	{
		gotoxy(xMax, 48);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		gotoxy(xMax, 52);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin + 1, 49 + i);
			printf("\b \b");
			gotoxy(xMax, 49 + i);
			printf("\b \b");
		}

		gotoxy(xMax, 53);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		gotoxy(xMax, 57);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin + 1, 54 + i);
			printf("\b \b");
			gotoxy(xMax, 54 + i);
			printf("\b \b");
		}
		gotoxy(xMin, 43);
		for (int i = 0; i < borderLength; i++)
		{
			printf("%s", WALL);
		}
		gotoxy(xMin, 47);
		for (int i = 0; i < borderLength; i++)
		{
			printf("%s", WALL);
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin, 44 + i);
			printf("%s", WALL);
			gotoxy(xMax - 1, 44 + i);
			printf("%s", WALL);
		}
	}
	else if(mode == 0)
	{
		gotoxy(xMax, 43);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		gotoxy(xMax, 47);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin + 1, 44 + i);
			printf("\b \b");
			gotoxy(xMax, 44 + i);
			printf("\b \b");
		}

		gotoxy(xMax, 53);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		gotoxy(xMax, 57);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin + 1, 54 + i);
			printf("\b \b");
			gotoxy(xMax, 54 + i);
			printf("\b \b");
		}

		gotoxy(xMin, 48);
		for (int i = 0; i < borderLength; i++)
		{
			printf("%s", WALL);
		}
		gotoxy(xMin, 52);
		for (int i = 0; i < borderLength; i++)
		{
			printf("%s", WALL);
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin, 49 + i);
			printf("%s", WALL);
			gotoxy(xMax - 1, 49 + i);
			printf("%s", WALL);
		}
	}
	else if (mode == 2)
	{
		gotoxy(xMax, 43);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		gotoxy(xMax, 47);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin + 1, 44 + i);
			printf("\b \b");
			gotoxy(xMax, 44 + i);
			printf("\b \b");
		}

		gotoxy(xMax, 48);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		gotoxy(xMax, 52);
		for (int i = 0; i < borderLength; i++)
		{
			printf("\b \b");
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(46, 49 + i);
			printf("\b \b");
			gotoxy(xMax, 49 + i);
			printf("\b \b");
		}

		gotoxy(xMin, 53);
		for (int i = 0; i < borderLength; i++)
		{
			printf("%s", WALL);
		}
		gotoxy(xMin, 57);
		for (int i = 0; i < borderLength; i++)
		{
			printf("%s", WALL);
		}
		for (int i = 0; i < 3; i++)
		{
			gotoxy(xMin, 54 + i);
			printf("%s", WALL);
			gotoxy(xMax - 1, 54 + i);
			printf("%s", WALL);
		}
	}
}

void intro()
{
	const char *titleText[7];
	const char *controls[7];

	titleText[0] = "||          ||  |||||||         ||        ||     ||||||||  ||      || ||||||||||| |||     ||  ||  |||     ||     |||||||| ";
	titleText[1] = " ||        ||  ||               ||        ||   ||          ||      ||     ||      ||||    ||  ||  ||||    ||   ||         ";
	titleText[2] = "  ||      ||   ||               ||        ||  ||           ||      ||     ||      || ||   ||  ||  || ||   ||  ||          ";
	titleText[3] = "   ||    ||     |||||||         ||        ||  ||  |||||||  ||||||||||     ||      ||  ||  ||  ||  ||  ||  ||  ||  ||||||| ";
	titleText[4] = "    ||  ||            ||        ||        ||  ||        || ||      ||     ||      ||   || ||  ||  ||   || ||  ||        ||";
	titleText[5] = "     ||||             ||        ||        ||  ||       ||  ||      ||     ||      ||    ||||  ||  ||    ||||  ||       || ";
	titleText[6] = "      ||        ||||||| ()      ||||||||| ||   |||||||||   ||      ||     ||      ||     |||  ||  ||     |||   |||||||||  ";

	controls[0] = "CONTROLS:";
	controls[1] = "You can move left and right with arrow keys.";
	controls[2] = "The down arrow key plants a magical seed that grows into a tree.";
	controls[3] = "The up arrow key raises the lightning guard, that absorbs lightning strikes and grants you one thunder-guard, that can protect you from future strikes.";
	controls[4] = "The s key initiates a sprint that will last a certain duration.";
	controls[5] = "The r key plants the mighty lightning rod if you have it in your backpack.";


	int customView = 0;
	printf("Press any key to set custom window size!\n\n");
	while (clock() < 3000)
	{
		while (_kbhit())
		{
			customView = 1;
            printf("Warning! Authentic game experience no longer guaranteed\n\n");
            _getch();
            break;
		}
        if (customView)
        {
            printf("What is your wanted game width in columns?\n");
            scanf_s("%hu", &w.width);
            printf("What is your wanted game height in rows?\n");
            scanf_s("%hu", &w.height);
            w.max_y = w.height - 4;
            break;
        }
	}

	printf("\nAdjust your console window size now!\n");
	Sleep(4000);
	system("CLS");
	PlaySound("BG.wav", NULL, SND_ASYNC);

	for (int i = 0; i < 7; i++)
	{
		gotoxy(50, 20 + i);
		printf("%s", titleText[i]);
	}
	for (int i = 0; i < 6; i++)
	{
		gotoxy(50, 35 + i);
		printf("%s", controls[i]);
	}

	gotoxy(50, 45);
	hidecursor();
	printf("NORMAL MODE - Lightning gets progressively more dangerous and you can upgrade your abilities, survive long enough to be victorious");
	gotoxy(50, 50);
	printf("NEVERENDING MODE - Dodge six rays of lightning with blinding speed for as long as you can, it gets harder, but never ends");
	gotoxy(50, 55);
	printf("CHAOS MODE - A more chaotic version of the classic game mode");
	m.gameMode = 1;
	chosenModeBorders(1);
	while (_getch() != '\r')
	{
		char c = 0;

		while (_kbhit())
		{
			c = _getch();
		}
		switch (c)
		{
		case 72:
			if (m.gameMode == 0)
			{
				m.gameMode = 1;
				chosenModeBorders(1);
				Sleep(50);
			}
			else if (m.gameMode == 2)
			{
				m.gameMode = 0;
				chosenModeBorders(0);
				Sleep(50);
			}
			break;
		case 80:
			if (m.gameMode == 1)
			{
				m.gameMode = 0;
				chosenModeBorders(0);
				Sleep(50);
			}

			else if (m.gameMode == 0)
			{
				m.gameMode = 2;
				chosenModeBorders(2);
				Sleep(50);
			}
			break;
		}
		if (c == 10)
		{
			break;
		}
	}

}

main()
{

	m.botMode = 0; //0 = player, 1 = bot
	m.gameMode = 1; //1 = normal, 0 = survival

	w.width = MAX_X;
	w.height = MAX_Y + 4;
	w.max_y = MAX_Y;

	char key = 0;
	char key2 = 0;
	char keyArray[2] = { 0, 0 };
	char c = 0;

	struct bitFieldVars *bfv = malloc(sizeof(struct bitFieldVars));
	struct lightningVars *lv = malloc(sizeof(struct lightningVars));
	struct shieldVars *sv = malloc(sizeof(struct shieldVars));
	struct treeVars *tv = malloc(sizeof(struct treeVars));
	struct heroVars *hv = malloc(sizeof(struct heroVars));
	struct sprintVars *spv = malloc(sizeof(struct sprintVars));
	struct rodVars *rv = malloc(sizeof(struct rodVars));
	struct clocks *clk = malloc(sizeof(struct clocks));

	

	srand((unsigned int)time(NULL));   // should only be called once
	intro();
	initialize(bfv, lv, sv, tv, hv, spv, rv, clk);


	moveHero(bfv, hv->currentPosition);
	while (1)
	{


		if (m.botMode)
		{
			int closestThunder = 1000;
			int closestThunderPos = w.width / 2;
			int a;

			for (int i = 0; i < bfv->thunderVolume; i++)
			{
				for (int j = i + 1; j < bfv->thunderVolume; j++)
				{
					if (lv->travelPoints[j] < lv->travelPoints[i])
					{
						a = lv->travelPoints[i];
						lv->travelPoints[i] = lv->travelPoints[j];
						lv->travelPoints[j] = a;
					}
				}
			}

			int limits[8];
			limits[0] = 0;
			for (int i = 1; i <= bfv->thunderVolume; i++)
			{
				limits[i] = lv->travelPoints[i - 1];
			}
			limits[bfv->thunderVolume + 1] = 237;

			int position;
			int gaps[7];
			for (int i = 0; i < bfv->thunderVolume + 1; i++)
			{
				gaps[i] = limits[i + 1] - limits[i];
				if (limits[i + 1] > hv->currentPosition && hv->currentPosition > limits[i])
					position = i;
			}

			int gapBigEnough[7];
			for (int i = 0; i < bfv->thunderVolume + 1; i++)
			{
				if (gaps[i] > 22 && lv->thunderProgress < 25)
					gapBigEnough[i] = 1;
				else if (gaps[i] > 18 && lv->thunderProgress < 40)
					gapBigEnough[i] = 1;
				else if (gaps[i] > 14)
					gapBigEnough[i] = 1;
				else
					gapBigEnough[i] = 0;
			}
			int closestBigEnoughGap = position;
			int distanceToClosestGap = 1000;

			for (int i = 0; i < position; i++)
			{
				if (hv->currentPosition - limits[i + 1] < distanceToClosestGap)
				{
					if (gapBigEnough[i])
					{
						distanceToClosestGap = hv->currentPosition - limits[i + 1];
						closestBigEnoughGap = i;
					}
				}
			}
			for (int i = bfv->thunderVolume; i > position; i--)
			{
				if (hv->currentPosition - limits[i] < distanceToClosestGap)
				{
					if (gapBigEnough[i])
					{
						distanceToClosestGap = hv->currentPosition - limits[i];
						closestBigEnoughGap = i;
					}
				}
			}


			for (int i = 0; i < bfv->thunderVolume; i++)
			{
				if (abs(hv->currentPosition - lv->travelPoints[i]) < closestThunder)
				{
					closestThunder = abs(hv->currentPosition - lv->travelPoints[i]);
					closestThunderPos = lv->travelPoints[i];
				}

			}

			if (closestThunder < 5 && lv->thunderProgress > 54 && clock() - clk->shieldPause > sv->shieldCooldown)
			{
				key = 72;
			}
			else if (clock() - clk->shieldPause > sv->shieldCooldown || clock() - clk->shieldPause < 300)
			{
				if (hv->currentPosition > closestThunderPos)
					key = 75;
				else
					key = 77;
			}
			else if (!bfv->thunderLaunched)
			{
				if (w.width / 2 > hv->currentPosition)
				{
					key = 77;
				}
				else
				{
					key = 75;
				}
			}
			else if (clock() - spv->sprintDownTime > spv->sprintCooldown && !bfv->lightningRunnerPerk)
			{
				key = 's';
			}
			else if (clock() - clk->treeDownTime > tv->treeCooldown)
			{
				key = 80;
			}
			else if (gaps[position] > 14)
			{
				if (hv->currentPosition > (limits[position + 1] + limits[position]) / 2)
					key = 75;
				else
					key = 77;
			}
			else if (distanceToClosestGap < 1000)
			{
				if (closestBigEnoughGap < position)
					key = 75;
				else
					key = 77;
			}
		}
		else
		{
			while (_kbhit())
			{
				key = _getch();

				if (bfv->agg == 0)
				{
					keyArray[0] = key;
				}
				else
				{
					keyArray[1] = key;
				}

				if (keyArray[0] == 0)
					key2 = keyArray[1];
				else
					key2 = keyArray[0];

				if (bfv->agg == 1)
					bfv->agg = 0;
				else
					bfv->agg = 1;
			}
		}

		switch (key)
		{
		case 77://right arrow key
			bfv->move = 1;
			key = 0;
			bfv->direction = 1;
			clk->pressDelay = clock();
			if (bfv->aegisPerk && bfv->shield && clock() - clk->throwDelay > 3000)
			{
				bfv->shieldThrow = 1;
				clk->throwDelay = clock();
			}
			break;

		case 75://left arrow key
			bfv->move = 1;
			bfv->direction = 0;
			key = 0;
			clk->pressDelay = clock();
			if (bfv->aegisPerk && bfv->shield && clock() - clk->throwDelay > 3000)
			{
				bfv->shieldThrow = 1;
				clk->throwDelay = clock();
			}
			break;

		case 72://up arrow key
			if (clock() - clk->shieldPause > sv->shieldCooldown)
			{
				bfv->move = 0;
				key = 0;
				bfv->shield = 1;
				clk->shieldDelay = clock();
				clk->shieldPause = clock();
				clearHero(hv->currentPosition);
				moveHero(bfv, hv->currentPosition);
			}
			break;

		case 80://down arrow key

			if (clock() - clk->treeDownTime > tv->treeCooldown)
			{
				key = 0;
				for (int i = 0; i < 5; i++)
				{
					if (tv->treePosition[i] == -1)
					{
						tv->treePosition[i] = hv->currentPosition;
						if (bfv->gardenerPerk)
							tv->treePhase[i] = 4;
						else
							tv->treePhase[i] = -1;
						tv->treeHealth[i] = tv->treeMaxHealth;
						bfv->tree++;
						clk->treeTimer[i] = clock();
						clk->treeDelay = clock();
						clk->treeDownTime = clock();
						i = 6;
					}
				}
			}
			break;

		}
		switch (key2)
		{
		case 's':
			key2 = 0;
			if (bfv->lightningRunnerPerk)
			{
				bfv->move = 0;
			}
			if (clock() - spv->sprintDownTime > spv->sprintCooldown && !bfv->lightningRunnerPerk)
			{
				if (bfv->sprintSpeed == 5)
				{
					PlaySound("sprintSound.wav", NULL, SND_ASYNC);
				}
				bfv->sprint = 1;
				clk->sprintTimer = clock();
				hv->runDelay = bfv->sprintSpeed;
				spv->sprintDownTime = clock();
			}
			break;

		case 'r':
			key2 = 0;
			if (bfv->lightningRodUnlocked && clock() - clk->rodDowntime > rv->rodCooldown)
			{
				bfv->lightningRod = 1;
				clk->rodDowntime = clock();
				rv->rodPosition = hv->currentPosition;
				buildRod(rv->rodPosition);
				if (bfv->channelerPerk)
				{
					hv->thunderGuard++;
					clk->treeDownTime = 20000;
					clk->shieldPause = 20000;
					spv->sprintDownTime = 20000;
				}
			}
			break;
		}

		if (bfv->sprint && clock() - clk->sprintTimer > spv->sprintDuration && !bfv->lightningRunnerPerk)
		{
			bfv->sprint = 0;
			hv->runDelay = 20;
		}

		if (bfv->lightningRod && clock() - clk->rodDowntime > rv->rodDuration)
		{
			bfv->lightningRod = 0;
			clearRod(rv->rodPosition);
			rv->rodPosition = -1;
		}
		else if (bfv->lightningRod)
		{
			buildRod(rv->rodPosition);
		}

		for (int i = 0; i < 5; i++)
		{
			if (tv->treeFruit[i] == 1)
			{
				if (abs(tv->treePosition[i] - hv->currentPosition) < 3)
				{
					hv->thunderGuard += bfv->harvesterPerk;
					tv->treeFruit[i] = 0;
					clk->treeTimer[i] = clock() - 6500;
				}
			}
		}

		if (bfv->move && clock() - clk->pressDelay > 250)
		{
			bfv->move = 0;
			clearHero(hv->currentPosition);
			moveHero(bfv, hv->currentPosition);
		}

		if (bfv->move && clock() - clk->moveDelay > hv->runDelay)
		{
			if (bfv->direction == 0)
			{
				if (hv->currentPosition > 2)
					hv->currentPosition--;
				clk->moveDelay = clock();
			}
			else if (bfv->direction == 1)
			{
				if (hv->currentPosition < w.width - 3)
					hv->currentPosition++;
				clk->moveDelay = clock();
			}
			clearHero(hv->currentPosition);
			moveHero(bfv, hv->currentPosition);
		}
		 
		hidecursor();
		if (clock() - clk->statsDelay > 100)
		{
			gotoxy(138, w.max_y + 1);
			printf("\b \b \b \b");
			gotoxy(98, w.max_y + 2);
			printf("\b \b \b \b");
			gotoxy(57, w.max_y + 1);
			printf("\b \b \b \b");
			gotoxy(136, w.max_y + 2);
			printf("\b \b \b \b");
			gotoxy(137, w.max_y + 3);
			printf("\b \b \b \b");

			if (bfv->lightningRodUnlocked)
			{
				gotoxy(80, w.max_y + 2);
				printf(clock() - clk->rodDowntime > rv->rodCooldown ? "LIGHTNING-ROD CD: 0" : "LIGHTNING-ROD CD: %d", ((rv->rodCooldown / 1000) - ((clock() - clk->rodDowntime) / 1000)));
			}

			gotoxy(40, w.max_y + 2);
			printf("SCORE: %d", hv->points);
			gotoxy(40, w.max_y + 1);
			printf("ThunderGuards: %d", hv->thunderGuard);

			if (!bfv->lightningRunnerPerk)
			{
				gotoxy(125, w.max_y + 1);
				printf(clock() - spv->sprintDownTime > spv->sprintCooldown ? "SPRINT CD: 0" : "SPRINT CD: %d", ((spv->sprintCooldown / 1000) - ((clock() - spv->sprintDownTime) / 1000)));
			}

			if (!bfv->ancientTreePerk)
			{
				gotoxy(125, w.max_y + 2);
				printf(clock() - clk->treeDownTime > tv->treeCooldown ? "SEED CD: 0" : "SEED CD: %d", ((tv->treeCooldown / 1000) - ((clock() - clk->treeDownTime) / 1000)));
			}

			gotoxy(125, w.max_y + 3);
			clock_t cd = clock() - clk->shieldPause;
			printf(cd > sv->shieldCooldown ? "SHIELD CD: 0" : "SHIELD CD: %d", ((sv->shieldCooldown / 1000) - (cd / 1000)));


			clk->statsDelay = clock();

		}

		if (bfv->shield && clock() - clk->shieldDelay > sv->shieldDuration)
		{
			bfv->shield = 0;
			clearHero(hv->currentPosition);
			moveHero(bfv, hv->currentPosition);
			clk->shieldPause = clock();
		}
		if (bfv->ancientTreePerk)
		{
			ancientTree(bfv, clk->ancientTreeTimer);
			if (abs(hv->currentPosition - (w.width / 2)) < 4)
			{
				if (bfv->ancientTreeFruit > 0)
				{
					hv->thunderGuard += 2;
					clk->ancientTreeTimer = clock();
					bfv->ancientTreeFruit = 0;
				}
			}
		}
		else if (bfv->tree > 0 && clock() - clk->treeDelay > 30)
		{
			for (int i = 0; i < 5; i++)
			{
				if (tv->treePosition[i] != -1)
				{
					if ((clock() - clk->treeTimer[i]) / 1500 > tv->treePhase[i] && (tv->treePhase[i] < 4))
					{
						tv->treePhase[i] += 1;
					}
					if (tv->treePhase[i] < 4)
					{
						buildTree(bfv, tv->treePhase[i], tv->treePosition[i], i, clk, tv);
					}
					else if (tv->treePhase[i] == 4)
					{
						buildTree(bfv, 4, tv->treePosition[i], i, clk, tv);
					}
					else
					{
						buildTree(bfv, 5, tv->treePosition[i], i, clk, tv);
					}
				}
			}
		}
		
		if (bfv->shieldThrow)
		{
			clearHero(hv->currentPosition);
			hv->thunderGuard += aegisThrow(lv, bfv, hv);
			bfv->shieldThrow = 0;
			clearHero(hv->currentPosition);
			moveHero(bfv, hv->currentPosition);
			clk->shieldPause = clock();
			clk->shieldDelay = clock() + 450;
			clk->throwDelay = clock();
		}

		if (m.gameMode == 2)
		{
			for (int i = 0; i < bfv->thunderVolume; i++)
			{

				if (!lv->thunderLaunches[i] && clock() - clk->thunderPauses[i] > lv->thunderFrequency + lv->thunderLaunchExtraDelays[i])
				{
					lv->thunderLaunches[i] = 1;
					clk->thunderDelays[i] = clock();
					lv->thunderLaunchExtraDelays[i] = rand() % 1000;
				}

				if (lv->thunderLaunches[i])
				{
					if (bfv->gardenerPerk && bfv->tree > 0 && lv->thunderProgresses[i] > 47 && lv->thunderProgresses[i] < w.max_y)
					{
						if (lv->thunderProgresses[i] < w.max_y && clock() - clk->thunderDelays[i] > lv->thunderSpeed + 10)
						{
							clk->thunderDelays[i] = clock();
							thunderAsync(i, lv, bfv, hv, tv, rv);
							lv->thunderProgresses[i]++;
						}
					}
					else if (lv->thunderProgresses[i] < w.max_y && clock() - clk->thunderDelays[i] > lv->thunderSpeed)
					{
						clk->thunderDelays[i] = clock();
						thunderAsync(i, lv, bfv, hv, tv, rv);
						lv->thunderProgresses[i]++;
					}

					if (lv->thunderProgresses[i] > w.max_y - 4)
					{

							for (int t = 0; t < 5; t++)
							{
								if (tv->treePosition[t] != -1)
								{
									if (tv->treePhase[t] > 3 && tv->treePhase[t] < 12 && (lv->travelPoints[i] < tv->treePosition[t] + 7 && lv->travelPoints[i] > tv->treePosition[t] - 7) && clock() - clk->treeDamageDelay[t] > 100)
									{
										if (tv->treeHealth[t] == 1)
										{
											PlaySound("explosion.wav", NULL, SND_ASYNC);
											tv->treePhase[t] = 5;
											clearTree(tv->treePosition[t]);
											moveHero(bfv, hv->currentPosition);
										}
										else
										{
											PlaySound("strike3.wav", NULL, SND_ASYNC);
											tv->treeHealth[t] -= 1;
											clk->treeDamageDelay[t] = clock();
										}
									}
								}
							}
							if (lv->travelPoints[i] < hv->currentPosition + 6 && lv->travelPoints[i] > hv->currentPosition - 6)
							{
								if (bfv->shield)
								{
									PlaySound("absorb.wav", NULL, SND_ASYNC);
									clearSpecificThunder(i, lv->thunderPath);
									lv->thunderLaunches[i] = 0;
									lv->thunderProgresses[i] = MIN_Y + 1;
									clk->thunderPauses[i] = clock();
									hv->thunderGuard++;
									if (i == 0)
									{
										hv->points++;
										gameProgress(bfv, lv, sv, tv, hv, spv, rv, clk);
									}
									i = 10;
								}
								else if (hv->thunderGuard > 0)
								{
									clearSpecificThunder(i, lv->thunderPath);
									PlaySound("strike3.wav", NULL, SND_ASYNC);
									hv->thunderGuard--;
									lv->thunderLaunches[i] = 0;
									lv->thunderProgresses[i] = MIN_Y + 1;
									clk->thunderPauses[i] = clock();

								}
								else
								{
									clearHero(hv->currentPosition);
									bfv->death = 1;
									moveHero(bfv, hv->currentPosition);
									if (hv->points < 10)
										PlaySound("strike3.wav", NULL, SND_ASYNC);
									else if (hv->points < 20)
										PlaySound("strike2.wav", NULL, SND_ASYNC);
									else if (hv->points > 19)
										PlaySound("strike1.wav", NULL, SND_ASYNC);
									for (int i = 0; i < 20; i++)
									{
										moveHero(bfv, hv->currentPosition);
										Sleep(30);
										clearHero(hv->currentPosition);
										Sleep(10);
										gameBorders();
									}
									gotoxy(40, 25);
									printf("YOU AVOIDED %d STRIKES BEFORE FRYING.", hv->points);
									gotoxy(40, 26);
									printf("PRESS ENTER TO TRY AGAIN!");
									while ((c = getchar()) != '\n' && c != EOF);
									initialize(bfv, lv, sv, tv, hv, spv, rv, clk);

								}
							}
						
						if (lv->thunderProgresses[i] >= w.max_y)
						{
							clearSpecificThunder(i, lv->thunderPath);
							lv->thunderLaunches[i] = 0;
							lv->thunderProgresses[i] = MIN_Y + 1;
							clk->thunderPauses[i] = clock();
							if (i == 0)
							{
								hv->points++;
								gameProgress(bfv, lv, sv, tv, hv, spv, rv, clk);
							}
						}
					}
				}
			}
		}
		else
		{
			if (!bfv->thunderLaunched && clock() - clk->thunderPause > lv->thunderFrequency)
			{
				bfv->thunderLaunched = 1;
				clk->thunderDelay = clock();
			}
			if (bfv->thunderLaunched)
			{
				if (bfv->timeSlowPerk > 0 && bfv->sprint)
				{
					if (lv->thunderProgress < w.max_y && clock() - clk->thunderDelay > lv->thunderSpeed + bfv->timeSlowPerk * 3)
					{
						clk->thunderDelay = clock();
						thunder(lv, bfv, hv, tv, rv);
						lv->thunderProgress++;
					}
				}
				else if (bfv->gardenerPerk && bfv->tree > 0 && lv->thunderProgress > 47 && lv->thunderProgress < w.max_y)
				{
					if (lv->thunderProgress < w.max_y && clock() - clk->thunderDelay > lv->thunderSpeed + 10)
					{
						clk->thunderDelay = clock();
						thunder(lv, bfv, hv, tv, rv);
						lv->thunderProgress++;
					}
				}
				else if (lv->thunderProgress < w.max_y && clock() - clk->thunderDelay > lv->thunderSpeed)
				{
					clk->thunderDelay = clock();
					thunder(lv, bfv, hv, tv, rv);
					lv->thunderProgress++;
				}

				if (lv->thunderProgress > w.max_y - 4)
				{
					for (int i = 0; i < bfv->thunderVolume; i++)
					{
						for (int t = 0; t < 5; t++)
						{
							if (tv->treePosition[t] != -1)
							{
								if (tv->treePhase[t] > 3 && tv->treePhase[t] < 12 && (lv->travelPoints[i] < tv->treePosition[t] + 7 && lv->travelPoints[i] > tv->treePosition[t] - 7) && clock() - clk->treeDamageDelay[t] > 100)
								{
									if (tv->treeHealth[t] == 1)
									{
										PlaySound("explosion.wav", NULL, SND_ASYNC);
										tv->treePhase[t] = 5;
										clearTree(tv->treePosition[t]);
										moveHero(bfv, hv->currentPosition);
									}
									else
									{
										PlaySound("strike3.wav", NULL, SND_ASYNC);
										tv->treeHealth[t] -= 1;
										clk->treeDamageDelay[t] = clock();
									}
								}
							}
						}
						if (lv->travelPoints[i] < hv->currentPosition + 6 && lv->travelPoints[i] > hv->currentPosition - 6)
						{
							if (bfv->shield)
							{
								PlaySound("absorb.wav", NULL, SND_ASYNC);
								clearThunder(bfv->thunderVolume, lv->thunderPath);
								bfv->thunderLaunched = 0;
								lv->thunderProgress = MIN_Y + 1;
								clk->thunderPause = clock();
								hv->thunderGuard++;
								hv->points++;
								i = 10;
								gameProgress(bfv, lv, sv, tv, hv, spv, rv, clk);
							}
							else if (hv->thunderGuard > 0)
							{
								clearThunder(bfv->thunderVolume, lv->thunderPath);
								PlaySound("strike3.wav", NULL, SND_ASYNC);
								hv->thunderGuard--;
								bfv->thunderLaunched = 0;
								lv->thunderProgress = MIN_Y + 1;
								clk->thunderPause = clock();
								i = 10;
							}
							else
							{
								clearHero(hv->currentPosition);
								bfv->death = 1;
								moveHero(bfv, hv->currentPosition);
								if (hv->points < 10)
									PlaySound("strike3.wav", NULL, SND_ASYNC);
								else if (hv->points < 20)
									PlaySound("strike2.wav", NULL, SND_ASYNC);
								else if (hv->points > 19)
									PlaySound("strike1.wav", NULL, SND_ASYNC);
								for (int i = 0; i < 20; i++)
								{
									moveHero(bfv, hv->currentPosition);
									Sleep(30);
									clearHero(hv->currentPosition);
									Sleep(10);
									gameBorders();
								}
								gotoxy(40, 25);
								printf("YOU AVOIDED %d STRIKES BEFORE FRYING.", hv->points);
								gotoxy(40, 26);
								printf("PRESS ENTER TO TRY AGAIN!");
								while ((c = getchar()) != '\n' && c != EOF);
								initialize(bfv, lv, sv, tv, hv, spv, rv, clk);
							}
						}
					}
					if (lv->thunderProgress >= w.max_y)
					{
						clearThunder(bfv->thunderVolume, lv->thunderPath);
						bfv->thunderLaunched = 0;
						lv->thunderProgress = MIN_Y + 1;
						clk->thunderPause = clock();
						hv->points++;
						gameProgress(bfv, lv, sv, tv, hv, spv, rv, clk);
					}
				}
			}
		}
	}
	return 1;
}


#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>


#define MIN_X 0
#define MIN_Y 2
#define MAX_X 237
#define MAX_Y 62
#define HEIGHT 50
#define WIDTH 237
#define WALL "|"
#define SIDE_WALL "||"
#define LIGHTNING_RANGE 100



int r[3];
int c;
int keyArray[2];
int agg;
int cd;
int travelPoints[3];
int travelPoint2;
int thunderLaunchPoints[3];
int thunderLaunched;
int thunderProgress;
int thunderGuard;
int thunderSpeed;
int thunderFrequency;
int thunderVolume;
int key;
int currentPosition;
int move;
int direction;
int death;
int step;
int shield;
int shieldDuration;
int shieldCooldown;
int tree;
int treeMaxHealth;
int treePosition[5];
int treeDuration;
int treePhase[5];
int treeCooldown;
int treeDurationUpgrade;
int points;
int sprintDownTime;
int sprintDuration;
int sprintCooldown;
int sprint;
int sprintSpeed;
int sprintTimer;
int runDelay;
int lightningRodUnlocked;
int rodCooldown;
int lightningRod;
int rodDuration;
int rodPosition;
int treeDecay;
int treeHealth[5];
clock_t start, delay, thunderPause, thunderDelay, shieldDelay, shieldPause;
clock_t treeDownTime, treeTimer[5], treeDelay, soundDelay, sprintDelay;
clock_t rodDowntime, statsDelay, treeDamageDelay[5];


const char *choice1[4];
const char *choice2[4];
const char *choice3[4];
const char *choice4[4];

const char *EndText[7];


void hidecursor()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void ggotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void buildRod()
{
	gotoxy(rodPosition, MAX_Y - 1);
	printf("%c", 193);
	gotoxy(rodPosition, MAX_Y - 2);
	printf("%c", 181);
	gotoxy(rodPosition, MAX_Y - 3);
	printf("%c", 198);
	gotoxy(rodPosition, MAX_Y - 4);
	printf("%c", 216);
}

void clearRod()
{
	gotoxy(rodPosition + 1, MAX_Y - 1);
	printf("\b \b");
	gotoxy(rodPosition + 1, MAX_Y - 2);
	printf("\b \b");
	gotoxy(rodPosition + 1, MAX_Y - 3);
	printf("\b \b");
	gotoxy(rodPosition + 1, MAX_Y - 4);
	printf("\b \b");

}

void buildTree(int buildPhase, int treePos, int treeNumber)
{
	switch (buildPhase)
	{
	case 0:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		break;
	case 1:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		gotoxy(treePos, MAX_Y - 2);
		printf("%c",186);
		break;
	case 2:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		gotoxy(treePos, MAX_Y - 2);
		printf("%c", 186);
		gotoxy(treePos, MAX_Y - 3);
		printf("%c",186);
		break;
	case 3:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		gotoxy(treePos, MAX_Y - 2);
		printf("%c", 186);
		gotoxy(treePos, MAX_Y - 3);
		printf("%c", 186);
		gotoxy(treePos - 2, MAX_Y - 4);
		printf("%c%c%c%c%c%c", 177,177,177, 177, 177, 177);
		break;
	case 4:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		gotoxy(treePos, MAX_Y - 2);
		printf("%c", 186);
		gotoxy(treePos, MAX_Y - 3);
		printf("%c", 186);
		gotoxy(treePos - 2, MAX_Y - 4);
		printf("%c%c%c%c%c%c", 177,177,177, 177, 177, 177);
		gotoxy(treePos - 1, MAX_Y - 5);
		printf("%c%c%c%c", 177,177, 177, 177);
		break;

	case 11:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		gotoxy(treePos, MAX_Y - 2);
		printf("%c", 186);
		gotoxy(treePos, MAX_Y - 3);
		printf("%c", 186);
		gotoxy(treePos - 2, MAX_Y - 4);
		printf("%c%c%c%c%c%c", 177,177,177, 177, 177, 177);

		gotoxy(treePos, MAX_Y - 5);
		for (int i = 0; i < 5; i++)
		{
			printf("\b \b");
		}
		break;
	case 12:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		gotoxy(treePos, MAX_Y - 2);
		printf("%c", 186);
		gotoxy(treePos, MAX_Y - 3);
		printf("%c", 186);

		gotoxy(treePos, MAX_Y - 4);
		for (int i = 0; i < 5; i++)
		{
			printf("\b \b");
		}
		break;
	case 13:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\///");
		gotoxy(treePos, MAX_Y - 2);
		printf("%c", 186);

		gotoxy(treePos - 2, MAX_Y - 3);
		for (int i = 0; i < 5; i++)
		{
			printf("\b \b");
		}
		break;
	case 14:
		gotoxy(treePos - 2, MAX_Y - 1);
		printf("\\\\//");
		gotoxy(treePos - 1, MAX_Y - 2);
		for (int i = 0; i < 5; i++)
		{
			printf("\b \b");
		}
		break;
	case 15:

		gotoxy(treePos - 1, MAX_Y - 1);
		for (int i = 0; i < 5; i++)
		{
			printf("\b \b");
		}
		tree--;

		treePosition[treeNumber] = -1;
		treePhase[treeNumber] = -2;
		break;
	}
}

void moveHero(int position, int move, int direction)
{
	if (shield)
	{
		gotoxy(position, MAX_Y - 5);
		printf("%c", 239);
		gotoxy(position - 1, MAX_Y - 4);
		printf("\O/");
		gotoxy(position , MAX_Y - 3);
		printf("|");
		gotoxy(position - 1, MAX_Y - 2);
		printf("/ \\");
	}
	else if (!move && !death)
	{
		gotoxy(position, MAX_Y - 4);
		printf("O");
		gotoxy(position - 1, MAX_Y - 3);
		printf("/|\\");
		gotoxy(position - 1, MAX_Y - 2);
		printf("/ \\");
	}
	else 
	{
		if (death)
		{
			if (step % 2 == 0)
			{
				gotoxy(position, MAX_Y - 4);
				printf("@");
				gotoxy(position - 1, MAX_Y - 3);
				printf("%c%c%c", 196, 157, 196);
				gotoxy(position - 1, MAX_Y - 2);
				printf("%c \\", 159);
				step++;
			}
			else
			{
				gotoxy(position, MAX_Y - 4);
				printf("O");
				gotoxy(position - 1, MAX_Y - 3);
				printf("/|\\");
				gotoxy(position - 1, MAX_Y - 2);
				printf("/ \\");
				step++;
			}
		}
		else if (direction == 1)
		{
			gotoxy(position, MAX_Y - 4);
			printf(" O");
			gotoxy(position - 1, MAX_Y - 3);
			printf("//");
			gotoxy(position - 1, MAX_Y - 2);
			if (step % 10 < 5)
			{
				printf(" />");
				step++;
			}
			else
			{
				printf("%c\ ", 217);
				step++;
			}
		}
		else if (direction < 3)
		{
			gotoxy(position - 2, MAX_Y - 4);
			printf("O");
			gotoxy(position - 1, MAX_Y - 3);
			printf("\\\\");
			gotoxy(position - 1, MAX_Y - 2);
			if (step % 10 < 5)
			{
				printf("|%c", 192);
				step++;
			}
			else
			{
				printf("<\\ ");
				step++;
			}
		}
	}
}

void gameBorders()
{
	gotoxy(MIN_X, MIN_Y);
	for (int i = 0; i < MAX_X; i++)
	{
		printf(WALL);
	}

	gotoxy(MIN_X, MAX_Y);
	for (int i = 0; i < MAX_X; i++)
	{
		printf(WALL);
	}
}

void thunder(int progress)
{
	int closestTree = 1000;
	int closestTreePos = -1000;

	for (int i = 0; i < thunderVolume; i++)
	{
		r[i] = (rand() % LIGHTNING_RANGE);
		thunderLaunchPoints[i] = r[i] + currentPosition - (LIGHTNING_RANGE / 2);
	}
	for (int i = 0; i < thunderVolume; i++)
	{
		if (progress > 30 && lightningRod && abs(rodPosition - travelPoints[i]) < 130)
		{
			if (rodPosition > -1 && treePhase > 3)
			{
				if (rodPosition > travelPoints[i])
				{
					travelPoints[i] += 1;
					gotoxy(travelPoints[i], progress);
					printf("\\\n");
				}
				else if (rodPosition < travelPoints[i])
				{
					travelPoints[i] -= 1;
					gotoxy(travelPoints[i], progress);
					printf("/\n");
				}
				else {
					gotoxy(travelPoints[i], progress);
					printf("|\n");
				}
			}
		}
		else if (progress > 47)
		{
			for (int t = 0; t < 5; t++)
			{
				if (treePhase[t] > 2 && treePhase[t] < 12 && abs(treePosition[t] - travelPoints[i]) < closestTree)
				{
					closestTree = abs(treePosition[t] - travelPoints[i]);
					closestTreePos = treePosition[t];
				}

			}
			if (abs(closestTreePos - travelPoints[i]) < 70)
			{
				if (closestTreePos > travelPoints[i])
				{
					travelPoints[i] += 1;
					gotoxy(travelPoints[i], progress);
					printf("\\\n");
				}
				else if (closestTreePos < travelPoints[i])
				{
					travelPoints[i] -= 1;
					gotoxy(travelPoints[i], progress);
					printf("/\n");
				}
				else {
					gotoxy(travelPoints[i], progress);
					printf("|\n");
				}
			}
			else if (abs(currentPosition - travelPoints[i]) < 60) 
			{

				if (currentPosition > travelPoints[i])
				{
					travelPoints[i] += 1;
					gotoxy(travelPoints[i], progress);
					printf("\\\n");
				}
				else if (currentPosition < travelPoints[i])
				{
					travelPoints[i] -= 1;
					gotoxy(travelPoints[i], progress);
					printf("/\n");
				}
				else {
					gotoxy(travelPoints[i], progress);
					printf("|\n");
				}
			}
			else 
			{
				if (r[i] < 34)
				{
					travelPoints[i] -= 1;
					gotoxy(travelPoints[i], progress);
					printf("/\n");
				}
				else if (r[i] < 67)
				{
					gotoxy(travelPoints[i], progress);
					printf("|\n");
				}
				else
				{
					travelPoints[i] += 1;
					gotoxy(travelPoints[i], progress);
					printf("\\\n");
				}
			}
		}

		else
		{
			if (progress == MIN_Y)
			{
				travelPoints[i] = thunderLaunchPoints[i] + (i * (rand() % 200)) - (i * 100);
			}
			if (r[i] < 34)
			{
				travelPoints[i] -= 1;
				gotoxy(travelPoints[i], progress);
				printf("/\n");
			}
			else if (r[i] < 67)
			{
				gotoxy(travelPoints[i], progress);
				printf("|\n");
			}
			else
			{
				travelPoints[i] += 1;
				gotoxy(travelPoints[i], progress);
				printf("\\\n");
			}
		}
	}
}

clearHero(int currentPosition)
{
	gotoxy(currentPosition + 3, MAX_Y - 2);
	for (int i = 0; i < 6; i++)
	{
		printf("\b \b");
	}
	gotoxy(currentPosition + 3, MAX_Y - 3);
	for (int i = 0; i < 5; i++)
	{
		printf("\b \b");
	}
	gotoxy(currentPosition + 3, MAX_Y - 4);
	for (int i = 0; i < 6; i++)
	{
		printf("\b \b");
	}
	gotoxy(currentPosition + 3, MAX_Y - 5);
	for (int i = 0; i < 6; i++)
	{
		printf("\b \b");
	}
}

int askNumber()
{
	int i1;
	scanf_s("%d", &i1);
	while (getchar() != '\n') {};
	return i1;
}

int gameProgress()
{
	if (thunderSpeed > 16)
	{
		thunderSpeed--;
		thunderFrequency = thunderFrequency - 110;
	}
	else if (thunderVolume == 1)
	{
		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		gotoxy(80, 31);
		printf("1)%s", choice1[0]);
		gotoxy(80, 32);
		printf("2)%s", choice1[1]);
		gotoxy(80, 33);
		printf("3)%s", choice1[2]);
		thunderVolume = 2;

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				shieldCooldown = shieldCooldown - 2000;
			}
			else if (c == 2)
			{
				treeCooldown = treeCooldown - 10000;
			}
			else if (c == 3)
			{
				sprintDuration = sprintDuration + 4000;
			}
			else
				i--;
		}
	}
	else if (points > 50 && thunderVolume == 2)
	{
		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		gotoxy(80, 31);
		printf("1)%s", choice2[0]);
		gotoxy(80, 32);
		printf("2)%s", choice2[1]);
		gotoxy(80, 33);
		printf("3)%s", choice2[2]);
		gotoxy(80, 34);
		thunderVolume = 3;
		thunderSpeed = 13;

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				shieldCooldown = shieldCooldown - 2000;
			}
			else if (c == 2)
			{
				sprintSpeed -= 5;
			}
			else if (c == 3)
			{
				sprintDuration = sprintDuration + 3000;
			}
			else
				i--;
		}

	}
	else if (points > 100 && thunderVolume == 3)
	{
		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		gotoxy(80, 31);
		printf("1)%s", choice3[0]);
		gotoxy(80, 32);
		printf("2)%s", choice3[1]);
		gotoxy(80, 33);
		printf("3)%s", choice3[2]);
		gotoxy(80, 34);
		printf("4)%s", choice3[3]);
		thunderVolume = 4;
		thunderSpeed = 10;

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				treeMaxHealth = treeMaxHealth * 3;
			}
			else if (c == 2)
			{
				shieldDuration = shieldDuration * 3;
			}
			else if (c == 3)
			{
				treeCooldown = treeCooldown - 6000;
			}
			else if (c == 4)
			{
				lightningRodUnlocked = 1;
			}
			else
				i--;
		}
	}
	else if (points > 150 && thunderVolume == 4 && thunderSpeed > 7)
	{
		thunderSpeed = 7;

		gotoxy(80, 30);
		printf("Choose wisely, things will only get worse from here");
		gotoxy(80, 31);
		printf("1)%s", choice4[0]);
		gotoxy(80, 32);
		printf("2)%s", choice4[1]);
		gotoxy(80, 33);
		printf("3)%s", choice4[2]);

		for (int i = 0; i < 1; i++)
		{
			int c = askNumber();

			if (c == 1)
			{
				sprintSpeed -= 5;
			}
			else if (c == 2)
			{
				treeMaxHealth = treeMaxHealth * 3;
			}
			else if (c == 3)
			{
				rodCooldown = rodCooldown - 15000;
			}
			else
				i--;
		}

	}
	else if (thunderSpeed > 5 && points > 200)
	{
		thunderSpeed = 5;
		thunderFrequency = thunderFrequency - 300;
	}
	else if (thunderSpeed > 3 && points > 200)
	{
		thunderSpeed = 3;
		thunderFrequency = thunderFrequency - 300;
	}
	else if (points > 249)
	{
		system("CLS");
		PlaySound("c:\\BGM.wav", NULL, SND_ASYNC);
		Sleep(400);
		gotoxy(50, 20);
		printf("%s", EndText[0]);
		Sleep(400);
		gotoxy(50, 21);
		printf("%s", EndText[1]);
		Sleep(400);
		gotoxy(50, 22);
		printf("%s", EndText[2]);
		Sleep(400);
		gotoxy(50, 23);
		printf("%s", EndText[3]);
		Sleep(400);
		gotoxy(50, 24);
		printf("%s", EndText[4]);
		Sleep(400);
		gotoxy(50, 25);
		printf("%s", EndText[5]);
		Sleep(400);
		gotoxy(50, 26);
		printf("%s", EndText[6]);
		Sleep(4000);

		gotoxy(40, 35);
		printf("LIVE ETERNALLY!", points);
		gotoxy(40, 36);
		printf("PRESS ENTER TO GO AGAIN!");
		while ((c = getchar()) != '\n' && c != EOF);
		initialize();
	}
}

initialize()
{
	PlaySound("c:\\thunderClap.wav", NULL, SND_ASYNC);
	system("CLS");
	thunderPause = clock();
	thunderDelay = clock();
	shieldDelay = clock();
	soundDelay = clock();
	thunderLaunched = 0;
	thunderProgress = MIN_Y;
	currentPosition = 100;
	thunderSpeed = 30;
	thunderFrequency = 2600;
	thunderVolume = 1;
	thunderGuard = 0;


	choice1[0] = "Shield cooldown -2 seconds";
	choice1[1] = "Seed cooldown -10 seconds";
	choice1[2] = "Sprint duration +4 seconds";
	choice1[3] = "";

	choice2[0] = "Shield cooldown -2 second";
	choice2[1] = "Upgrade sprint speed";
	choice2[2] = "Sprint duration +3 seconds";
	choice2[3] = "";

	choice3[0] = "Trees become more resistant to lightning";
	choice3[1] = "Triple shield uptime";
	choice3[2] = "Seed cooldown -6 seconds";
	choice3[3] = "Unlock the mighty lightning rod";

	choice4[0] = "Upgrade sprint speed";
	choice4[1] = "Trees become even more resistant to lightning";
	choice4[2] = "Lightning rod cooldown - 15 seconds";
	choice4[3] = "";

	EndText[0] = "||        |||||||||    ||||||||   |||||||||  |||     ||  ||||          |       |||||||||| ||      ||";
	EndText[1] = "||        ||          ||          ||         ||||    ||  ||  ||       | |      ||      ||  ||    || ";
	EndText[2] = "||        ||         ||           ||         || ||   ||  ||    ||    || ||     ||   |||     ||  ||  ";
	EndText[3] = "||        |||||||||  ||  |||||||  |||||||||  ||  ||  ||  ||     ||  ||   ||    ||||||         ||    ";
	EndText[4] = "||        ||         ||         | ||         ||   || ||  ||    ||  |||||||||   ||   ||        ||    ";
	EndText[5] = "||        ||         ||        |  ||         ||    ||||  ||   ||  ||       ||  ||    ||       ||    ";
	EndText[6] = "||||||||| |||||||||  ||||||||||   |||||||||  ||     |||  |||||   ||         || ||     ||      ||    ";


	sprint = 0;
	sprintDelay = clock();
	sprintSpeed = 15;
	runDelay = 20;
	start = 0;
	delay = clock();
	points = 0;

	statsDelay = 100;
	key = 0;
	shield = 0;
	tree = 0;
	treeDecay = 1;
	lightningRod = 0;
	rodDuration = 15000;
	shieldDuration = 300;
	shieldCooldown = 8000;
	treeCooldown = 20000;
	treeMaxHealth = 2;
	treeDurationUpgrade = 0;
	sprintDuration = 5000;
	sprintCooldown = 20000;
	lightningRodUnlocked = 0;
	rodCooldown = 30000;

	for (int i = 0; i < 5; i++)
	{
		treePosition[i] = -1;
		treePhase[i] = -2;
		treeHealth[i] = 2;
		treeDamageDelay[i] = clock();
	}

	rodPosition = -1;
	move = 0;
	death = 0;
	step = 1;
	agg = 0;
	direction = 2; // 0 = left 1 = right
	gameBorders();
	moveHero(currentPosition, move, direction);
}

main()
{

	srand(time(NULL));   // should only be called once
	initialize();

	while (1)
	{
		while (_kbhit())
		{
			key = _getch();
			if (agg % 2 == 0)
			{
				keyArray[0] = key;
			}
			else
			{
				keyArray[1] = key;
			}

			if (agg == 0)
				agg = 1;
			else
				agg = 0;
		}

		switch (key)
		{
			case 77:
				move = 1;
				key = 0;
				direction = 1;
				start = clock();
				break;

			case 75:
				move = 1;
				direction = 0;
				key = 0;
				start = clock();
				break;

			case 72:
				if (clock() - shieldPause > shieldCooldown)
				{
					move = 0;
					key = 0;
					direction = 2;
					shield = 1;
					shieldDelay = clock();
					shieldPause = clock();
					clearHero(currentPosition);
					moveHero(currentPosition, move, direction);
				}
				break;

			case 80:

				if (clock() - treeDownTime > treeCooldown)
				{
					key = 0;
					for (int i = 0; i < 5; i++)
					{
						if (treePosition[i] == -1)
						{
							treePosition[i] = currentPosition;
							treePhase[i] = -1;
							treeHealth[i] = treeMaxHealth;
							tree++;
							treeTimer[i] = clock();
							treeDelay = clock();
							treeDownTime = clock();
							i = 6;
						}
					}
				}
				break;

		}
		
		switch (keyArray[0])
		{
		case 115:
			if (clock() - sprintDownTime > sprintCooldown)
			{
				sprint = 1;
				sprintTimer = clock();
				runDelay = sprintSpeed;
				sprintDownTime = clock();
			}
			break;

		case 114:
			if (lightningRodUnlocked && clock() - rodDowntime > rodCooldown)
			{
				lightningRod = 1;
				rodDowntime = clock();
				rodPosition = currentPosition;
				buildRod();
			}
			break;
		}

		switch (keyArray[1])
		{
		case 115:
			if (clock() - sprintDownTime > sprintCooldown)
			{
				sprint = 1;
				sprintTimer = clock();
				runDelay = sprintSpeed;
				sprintDownTime = clock();
			}
			break;

		case 114:
			if (lightningRodUnlocked && clock() - rodDowntime > rodCooldown)
			{
				lightningRod = 1;
				rodDowntime = clock();
				rodPosition = currentPosition;
				buildRod();
			}
			break;
		}
		

		if (sprint && clock() - sprintTimer > sprintDuration)
		{
			sprint = 0;
			runDelay = 20;
		}

		if (lightningRod && clock() - rodDowntime > rodDuration)
		{
			lightningRod = 0;
			clearRod();
			rodPosition = -1;
		}
		else if (lightningRod)
		{
			buildRod();
		}

		if (move && clock() - start > 250)
		{
			move = 0;
			clearHero(currentPosition);
			moveHero(currentPosition, move, direction);
		}

		if (move && clock() - delay > runDelay)
		{
			if (direction == 0)
			{
				currentPosition--;
				delay = clock();
			}
			else if (direction == 1)
			{
				currentPosition++;
				delay = clock();
			}
			clearHero(currentPosition);
			moveHero(currentPosition, move, direction); 
		}
		 
		hidecursor();
		if (clock() - statsDelay > 100)
		{
			gotoxy(138, MAX_Y + 1);
			printf("\b \b \b \b");
			gotoxy(98, MAX_Y + 2);
			printf("\b \b \b \b");
			gotoxy(58, MAX_Y + 1);
			printf("\b \b \b \b");
			gotoxy(136, MAX_Y + 2);
			printf("\b \b \b \b");
			gotoxy(137, MAX_Y + 3);
			printf("\b \b \b \b");

			if (lightningRodUnlocked)
			{
				gotoxy(80, MAX_Y + 2);
				printf(clock() - rodDowntime > rodCooldown ? "LIGHTNING-ROD CD: 0" : "LIGHTNING-ROD CD: %d", ((rodCooldown / 1000) - ((clock() - rodDowntime) / 1000)));
			}

			gotoxy(40, MAX_Y + 2);
			printf("SCORE: %d", points);
			gotoxy(40, MAX_Y + 1);
			printf("ThunderGuards: %d", thunderGuard);

			gotoxy(125, MAX_Y + 1);
			printf(clock() - sprintDownTime > sprintCooldown ? "SPRINT CD: 0" : "SPRINT CD: %d", ((sprintCooldown / 1000) - ((clock() - sprintDownTime) / 1000)));

			gotoxy(125, MAX_Y + 2);
			printf(clock() - treeDownTime > treeCooldown ? "SEED CD: 0" : "SEED CD: %d", ((treeCooldown / 1000) - ((clock() - treeDownTime) / 1000)));

			gotoxy(125, MAX_Y + 3);
			cd = clock() - shieldPause;
			printf(cd > shieldCooldown ? "SHIELD CD: 0" : "SHIELD CD: %d", ((shieldCooldown / 1000) - (cd / 1000)));


			statsDelay = clock();

		}
		

		if (!thunderLaunched && clock() - thunderPause > thunderFrequency)
		{
			thunderLaunched = 1;
			thunderDelay = clock();
		}

		if (shield && clock() - shieldDelay > shieldDuration)
		{
			shield = 0;
			clearHero(currentPosition);
			moveHero(currentPosition, move, direction);
			shieldPause = clock();
		}

		if (tree > 0 && clock() - treeDelay > 30)
		{
			for (int i = 0; i < 5; i++)
			{
				if (treePosition[i] != -1)
				{
					if ((clock() - treeTimer[i]) / 1000 > treePhase[i] && (treePhase[i] < 4 || treePhase[i] > 13))
					{
						treePhase[i] += 1;
					}
					if (treePhase[i] > 13 || treePhase[i] < 4)
					{
						buildTree(treePhase[i], treePosition[i], i);
					}
					else
					{
						buildTree(4, treePosition[i], i);
					}
				}
			}
		}
		
		if (thunderLaunched)
		{
			if (thunderProgress < MAX_Y && clock() - thunderDelay > thunderSpeed)
			{
				thunderDelay = clock();
				thunder(thunderProgress);
				thunderProgress++;
			}

			if (thunderProgress > MAX_Y - 4)
			{ 
				for (int i = 0; i < thunderVolume; i++)
				{
					for (int t = 0; t < 5; t++)
					{
						if (treePosition[t] != -1)
						{
							if (treePhase[t] > 3 && treePhase[t] < 12 && (travelPoints[i] < treePosition[t] + 7 && travelPoints[i] > treePosition[t] - 7) && clock() - treeDamageDelay[t] > 100)
							{
								if (treeHealth[t] == 1)
								{
									PlaySound("c:\\explosion.wav", NULL, SND_ASYNC);
									treePhase[t] = 14;
									moveHero(currentPosition, move, direction);
								}
								else
								{
									PlaySound("c:\\strike3.wav", NULL, SND_ASYNC);
									treeHealth[t] -= 1;
									treeDamageDelay[t] = clock();
								}
							}
						}
					}
					if (travelPoints[i] < currentPosition + 6 && travelPoints[i] > currentPosition - 6)
					{
						if (shield)
						{
							PlaySound("c:\\absorb.wav", NULL, SND_ASYNC);
							system("CLS");
							moveHero(currentPosition, move, direction);
							gameBorders();
							thunderLaunched = 0;
							thunderProgress = MIN_Y;
							thunderPause = clock();
							thunderGuard += 1;
							points++;
							i = 10;
							gameProgress();
						}
						else if (thunderGuard > 0)
						{
							PlaySound("c:\\strike3.wav", NULL, SND_ASYNC);
							thunderGuard--;
							thunderLaunched = 0;
							thunderProgress = MIN_Y;
							thunderPause = clock();
							system("CLS");
							moveHero(currentPosition, move, direction);
							gameBorders();
							i = 10;
						}
						else
						{
							clearHero(currentPosition);
							death = 1;
							moveHero(currentPosition, move, direction);
							if (points < 10)
								PlaySound("c:\\strike3.wav", NULL, SND_ASYNC);
							else if (points < 20)
								PlaySound("c:\\strike2.wav", NULL, SND_ASYNC);
							else if (points > 19)
								PlaySound("c:\\strike1.wav", NULL, SND_ASYNC);
							for (int i = 0; i < 20; i++)
							{
								moveHero(currentPosition, move, direction);
								Sleep(30);
								clearHero(currentPosition);
								Sleep(10);
								gameBorders();
							}
							gotoxy(40, 25);
							printf("YOU AVOIDED %d STRIKES BEFORE FRYING.", points);
							gotoxy(40, 26);
							printf("PRESS ENTER TO TRY AGAIN!");
							while ((c = getchar()) != '\n' && c != EOF);
							initialize();
						}
					}
				}
				if (thunderProgress >= MAX_Y)
				{
					system("CLS");
					moveHero(currentPosition, move, direction);
					gameBorders();
					thunderLaunched = 0;
					thunderProgress = MIN_Y;
					thunderPause = clock();
					points++;
					gameProgress();
				}
			}
		}
		
	}
	return 1;
}
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <string> 
#include <math.h>


using namespace std;
/*
liczby przypisane znakom w tabeli:
0 - scatter
1 - wisnia
2 - cytryna
3 - pomarancza
4 - sliwka
5 - winogrono
6 - arbuz
7 - siedem
*/

//short** draw();

struct gameData {

	short layouts[20][5] = { //układy wygranych
		{0,0,0,0,0},
		{0,1,2,1,0},
		{0,0,1,0,0},
		{0,1,2,2,2},
		{0,1,2,1,1},
		{0,0,1,1,1},
		{0,0,0,1,2},
		{0,1,0,1,0},
		{1,1,1,1,1},
		{1,1,0,1,1},
		{1,0,1,0,1},
		{1,2,1,2,1},
		{2,2,2,2,2},
		{2,1,0,1,2},
		{2,2,1,0,0},
		{2,1,0,2,2},
		{2,1,0,0,0},
		{2,1,2,1,2},
		{2,2,1,1,1},
		{2,2,2,1,0}
	};

	float payouts[8][5] = { //wartości wygranych
		{0,0,2,10,50},
		{0,0.25,1,2.50,10},
		{0,0,1,2.50,10},
		{0,0,1,2.50,10},
		{0,0,1,2.50,10},
		{0,0,2.50,10,25},
		{0,0,2.50,10,25},
		{0,0,5,50,250}
	};
};

struct reelsStruct //struktura maszyny (jej output oraz układ znaków oraz)
{
	short reels[5][20] = {
		{ 4,3,1,7,5,4,6,2,0,3,7,5,2,3,6,5,3,2,4,7 }, //tasmy
		{ 0,6,5,4,7,6,3,1,4,2,5,7,5,6,4,2,7,3,1,3 },
		{ 1,6,0,1,3,4,6,2,5,7,3,1,6,2,5,1,4,1,0,2 },
		{ 2,5,0,6,7,4,1,5,3,1,3,6,4,1,6,4,3,1,2,3 },
		{ 1,3,2,4,6,0,7,4,5,7,1,6,5,4,1,7,2,3,1,3 }
	};
	
	short output[3][5] = {
		{0,0,0,0,0},
		{0,0,0,0,0},
		{0,0,0,0,0}
	};

};

struct gameStatistics //struktura przechowywująca statystyki oraz nazwy symboli
{
	string symbols[8] = {
		"Scatter     ",
		"Wiśnia      ",
		"Cytryna     ",
		"Pomarańcza  ",
		"Śliwka      ",
		"Winogrono   ",
		"Arbuz       ",
		"Siódemka    "
	};

	float currentPayoutCounter = 0;

	float currentSaldo = 100000;

	float startingSaldo = 100000;

	int showStatsRange = 100;

	float tempGain = 0;

	int tempHitCounter = 0;

	float gain = 0;

	float HR = 0;

	int hitCounter = 0;

	int drawsCounter = 0;

	float RTP = 0;

	float gameCost = 1;

	float maxGain = 0;

	int FigLayoutHit[8][4] = { //ilość trafień w daną sekwencję
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0}
	};
};

struct statisticsRange //dane przechowywane w liście
{
	float gain = 0;

	int hitCounter = 0;

	int rangeTo = 0;

	statisticsRange* next;

	statisticsRange();
};

statisticsRange::statisticsRange()
{
	next = 0;
}

struct list //lista stworzona na potrzeby wyświetlania danych losowanych w zakresach
{
	statisticsRange* first;
	void addStatRange(float gain, int hitCounter, int rangeFrom);
	void printData(int range, float allGain, int allHits);
	list();
};

list::list()
{
	first = 0;
}

void checkLayout(gameData*, reelsStruct*, gameStatistics*);
void checkScatters(gameData*, reelsStruct*, gameStatistics*);
void draw(reelsStruct*);
void printStatistics(gameStatistics*);
void saveToFile(ofstream&, string);
void printStatisticsLayoutHits(gameStatistics*, gameData*);
void printStatsRange(gameStatistics*, list*);
void printSpaces(int, int);
void printSeparators(int);

int main(int argc, char* argv[])
{
	srand(time(NULL));
	setlocale(LC_CTYPE, "Polish");

	gameData game;
	reelsStruct machine;
	gameStatistics stats;
	list* statsList = new list;
	int gamesCount = 0; //ilość gier do przelosowania
	
	string fileName;
	ofstream gameFile;
	bool isArgSaveToFile = false, isArgGames = false;

	for (int i = 0; i < argc; ++i)
	{
		if (argv[i][0] == '-' && i + 1 <= argc)
		{
			if (argv[i] == "-gamesCount"s) //w przypadku nie podania tego argumentu program zapyta o ilość gier
			{
				gamesCount = (int)argv[i + 1];
				isArgGames = true;
			}
			if (argv[i] == "-startCredit"s) //podstawowe salto w przypadku nie podania argumentu to 100000
			{
				stats.startingSaldo = (float)(int)argv[i + 1]; //do celów liczenia statystyk
				stats.currentSaldo = (float)(int)argv[i + 1];
			}
			if (argv[i] == "-creditOutFile"s) //jeśli ten argument nie zostanie podany to plik nie zostanie zapisany
			{
				fileName = (string)argv[i + 1];
				isArgSaveToFile = true;
			}
		}
	}

	if (!isArgGames)
	{
		cout << "Podaj liczbe losowan: ";
		cin >> gamesCount;
	}

	stats.showStatsRange = gamesCount / 10; //ustawione w ten sposób by w skróconym raporcie wyświetlkiło się tylko 10 pozycji
	if (isArgSaveToFile)
	{
		gameFile.open(fileName, ios::in | ios::app);
	}

	for (int i = 0; i < gamesCount; i++)
	{
		if (stats.currentSaldo < stats.gameCost)
		{
			break; //jeśli saldo jest niewystarczające gra nie będzie trwała dalej
		}

		stats.currentPayoutCounter = 0; //ustawianie danych startowych dla każdego nowego losowania
		stats.currentSaldo -= stats.gameCost;
		stats.drawsCounter++;
		
		draw(&machine); //losowanie układu
		
		checkScatters(&game, &machine, &stats);
		checkLayout(&game, &machine, &stats); //sprawdzanie możliwych wygranych

		stats.gain += stats.currentPayoutCounter; 
		stats.currentSaldo += stats.currentPayoutCounter; //aktualizacja salda

		if (stats.currentPayoutCounter > stats.maxGain) //sprawdzanie największej wygranej w losowaniach
		{
			stats.maxGain = stats.currentPayoutCounter;
		}

		if ((i + 1) % stats.showStatsRange == 0) //jeśli zostało wylosowane kolejne 10% zadeklarowanych gier
		{
			statsList->addStatRange(stats.tempGain, stats.tempHitCounter, i); //dadajemy wygrane w tych gier oraz ich ilość do listy
			stats.tempGain = 0;
			stats.tempHitCounter = 0;
		}
		else
		{
			stats.tempGain += stats.currentPayoutCounter; //jeśli nie to zwiększamy tymczasową wygraną
		}

		stats.HR = (float)stats.hitCounter / (float)stats.drawsCounter * 100;
		stats.RTP = stats.gain / (stats.drawsCounter * stats.gameCost) * 100;
		if (isArgSaveToFile)
		{
			saveToFile(gameFile, to_string(stats.currentSaldo));
		}
	}

	if (gameFile.is_open())
	{
		gameFile.close();
	}
	
	printStatistics(&stats); //wyświetlenie statystyk

	printStatisticsLayoutHits(&stats, &game); //wyświetlenie głównych statystyk
	printStatsRange(&stats, statsList); //wyświetlenie skróconego raportu

	delete(statsList); //usunięcie listy
}

void checkLayout(gameData* game, reelsStruct* machine, gameStatistics* gameStats)
{
	for (int i = 0; i < 20; i++) //iterowanie po ukladach
	{
		short symbolType = 0;
		int symbolCounter = 0;
		symbolType = machine->output[game->layouts[i][0]][0]; //przypisanie do zmiennej typu pierwszego znaku w ukladzie
		for (int j = 0; j < 5; j++) //iterowanie po kolejnych znakach w ukladzie na tasmach
		{
			if (symbolType == 0 && j == 0) //jeśli symbol to scatter i jestesmy na pierwszej tasmie wychodzimy z pentli
			{
				break;
			}
			if (j > 0 && machine->output[game->layouts[i][j]][j] != symbolType) //jesli nastepny symbol w ulozeniu nie jest taki sam jak pierwszy wychodzimy z pentli
			{
				break;
			}
			else //w przeciwnym razie inkrementujemy liczbe takich samych symboli
			{
				symbolCounter++;
			}
		}
		if (symbolType != 0) //jesli po sprawdzeniu ilosci symboli oraz upewnieniu sie ze nie trafilismy na scatter na pierwszym bebnie idziemy do przyznania wygranej
		{
			if (game->payouts[symbolType][symbolCounter - 1] > 0) //jesli wygrana z ilosci symboli jest wieksza od 0
			{
				gameStats->currentPayoutCounter += game->payouts[symbolType][symbolCounter - 1] * gameStats->gameCost; //przypisanie wygranej jesli istnieje
				gameStats->hitCounter++;
				gameStats->tempHitCounter++;
				gameStats->FigLayoutHit[symbolType][symbolCounter - 2]++; // - 2 poniewaz na pozycji 0 jest przechowywana wartość trafień 2 symboli
			}
		}
	}
}

void checkScatters(gameData* game, reelsStruct* machine, gameStatistics* gameStats)
{
	int scattersCounter = 0;
	for (int i = 0; i < 3; i++) //liczenie ilosci scatterow na wylosowanych danych
	{
		for (int j = 0; j < 5; j++)
		{
			if (machine->output[i][j] == 0)
			{
				scattersCounter++;
			}
		}
	}
	if (scattersCounter > 5)
	{
		scattersCounter = 5;
	}
	if (scattersCounter > 2 && game->payouts[0][scattersCounter - 1] > 0)
	{
		gameStats->currentPayoutCounter += game->payouts[0][scattersCounter - 1] * gameStats->gameCost; //przypisanie wygranej jesli istnieje
		gameStats->hitCounter++;
		gameStats->tempHitCounter++;
		gameStats->FigLayoutHit[0][scattersCounter - 2]++;
	}
}

void draw(reelsStruct* machine)
{
	short position;

	for (int i = 0; i < 5; i++) //wylosowanie pozycji taśm
	{
		position = rand() % 20;
		for (int j = 0; j < 3; j++)
		{
			if (position == 19)
			{
				position = -1; //jeśli pozycja przekracza zakres taśmy wracamy na początek
			}
			position++;
			machine->output[j][i] = machine->reels[i][position];
		}
	}
}

void printStatsRange(gameStatistics* gameStats, list* statsList)
{
	cout << "Rozkład wygranych podany w zakresach:" << endl;
	printSeparators(91);
	cout << "|     Przedział     | Ilość wygranych | Ilość wygranych % | Uzyskany przychód | Przychód % |" << endl;
	printSeparators(91);

	statsList->printData(gameStats->showStatsRange, gameStats->gain, gameStats->hitCounter); //wyświetlenie danych z raportu
}

void printStatistics(gameStatistics* gameStats)
{
	cout << "Początkowe saldo: " << gameStats->startingSaldo << endl;
	cout << "Liczba gier: " << gameStats->drawsCounter << endl;
	cout << "Stawka gry: " << gameStats->gameCost << endl;
	cout << "Suma pobranych stawek: " << (int)(gameStats->drawsCounter * gameStats->gameCost) << endl;
	cout << "Suma wygranych: " << gameStats->gain << endl;
	cout << "Najwyższa wygrana z jednego losowania: " << gameStats->maxGain << endl;
	cout << "Saldo: " << gameStats->currentSaldo << endl;
	cout << "Hit Ratio: " << gameStats->HR << endl;
	cout << "RTP: " << gameStats->RTP << endl;
}

void saveToFile(ofstream& gameFile, string text)
{
	text[text.length() - 7] = ','; //zamiana na potrzeby zrobienia wykresu w programie excell
	gameFile << text << endl;
}

void printSpaces(int charCount, int charNumber) //formatowanie tabelki
{
	for (int i = 0; i < charNumber - charCount; i++)
	{
		cout << " ";
	}
	cout << "|";
}

void printSeparators(int value)
{
	for (int j = 0; j <= value; j++) //wyświetlenie separatorów w konsoli
	{
		cout << "-";
	}
	cout << endl;
}

void printStatisticsLayoutHits(gameStatistics* gameStats, gameData* game)
{
	string temp;
	cout << "Częstotliwość występowania układów:" << endl;
	printSeparators(102);
	cout << "|     Znak      | Mnożnik | Licz. traf. | Licz. traf. % | Cał. war. | Cał. war. % |  1/(w)  |  1/(g)  |" << endl;
	printSeparators(102);

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (gameStats->FigLayoutHit[i][j] > 0)
			{
				cout << "|";
				cout << gameStats->symbols[i];
				cout << " x" << j + 2 << "|";

				temp = to_string(game->payouts[i][j + 1]);
				temp = temp.substr(0, temp.length() - 4); //usówamy niepotrzebne 0 po przecinku z liczby
				cout << temp;
				printSpaces(temp.length(), 9);

				temp = to_string(gameStats->FigLayoutHit[i][j]);
				cout << temp;
				printSpaces(temp.length(), 13);

				temp = to_string(round((float)gameStats->FigLayoutHit[i][j] / (float)gameStats->hitCounter * 10000) / 100); //zaokrąglamy do 2 miejsc po ,
				temp = temp.substr(0, temp.length() - 4); //usówamy niepotrzebne 0 po przecinku z liczby
				temp += "%";
				cout << temp;
				printSpaces(temp.length(), 15);

				temp = to_string(game->payouts[i][j + 1] * gameStats->FigLayoutHit[i][j]);
				temp = temp.substr(0, temp.length() - 4); //usówamy niepotrzebne 0 po przecinku z liczby
				cout << temp;
				printSpaces(temp.length(), 11);

				temp = to_string(round((game->payouts[i][j + 1] * gameStats->FigLayoutHit[i][j]) / gameStats->gain * 10000) / 100);
				temp = temp.substr(0, temp.length() - 4); //usówamy niepotrzebne 0 po przecinku z liczby
				temp += "%";
				cout << temp;
				printSpaces(temp.length(), 13);

				temp = to_string(gameStats->hitCounter / gameStats->FigLayoutHit[i][j]);
				cout << temp;
				printSpaces(temp.length(), 9);

				temp = to_string(gameStats->drawsCounter / gameStats->FigLayoutHit[i][j]);
				cout << temp;
				printSpaces(temp.length(), 9);

				cout << endl;
			}
		}
		printSeparators(102);
	}
}

void list::addStatRange(float gain, int hitCounter, int rangeFrom) //przypisanie wartości pozycji w liście
{
	statisticsRange* statsRange = new statisticsRange;

	statsRange->gain = gain;
	statsRange->hitCounter = hitCounter;
	statsRange->rangeTo = rangeFrom;

	if (first == 0)
	{
		first = statsRange;
	}
	else
	{
		statisticsRange* temp = first;

		while (temp->next)
		{
			temp = temp->next;
		}

		temp->next = statsRange;
		statsRange->next = 0;
	}
}

void list::printData(int range, float allGain, int allHits)
{
	statisticsRange* temp = first;
	string tempString;

	while (temp) //wyświetlenie danych z danej pozycji z listy oraz kalkulacja wartości %
	{
		tempString = "| " + to_string(temp->rangeTo - (range - 2)) + " - " + to_string(temp->rangeTo + 1);
		cout << tempString;
		printSpaces(tempString.length(), 20);


		tempString = to_string(temp->hitCounter);
		cout << tempString;
		printSpaces(tempString.length(), 17);

		tempString = to_string(round((float)temp->hitCounter / (float)allHits * 10000) / 100);
		tempString = tempString.substr(0, tempString.length() - 4);
		cout << tempString;
		printSpaces(tempString.length(), 19);

		tempString = to_string(temp->gain);
		tempString = tempString.substr(0, tempString.length() - 4);
		cout << tempString;
		printSpaces(tempString.length(), 19);

		tempString = to_string(round((float)temp->gain / (float)allGain * 10000) / 100);
		tempString = tempString.substr(0, tempString.length() - 4);
		cout << tempString;
		printSpaces(tempString.length(), 12);
		cout << endl;
		printSeparators(91);
		temp = temp->next;
	}
}
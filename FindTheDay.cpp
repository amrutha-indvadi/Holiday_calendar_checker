#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRICATE
#define _CRT_NONSTDC_NO_DEPRICATE

#include<stdio.h>
#include<windows.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

typedef struct
{
	int Date;
	long long Time;
	double Price;
	double IV;
	double HighPriceBand;
}Data;

typedef struct
{
	unsigned long long FileSize;
	int IncStepsSize;
	int RowSize;
	int RowsSpace;
	int MaxToken;
	int MinToken;
	int AutoId;
	int RowCount;
	int NextVacantId;
	void* pVoid;
	size_t   NoofLines;
	size_t   NoofValidLines;
}FileHeader;

typedef struct
{
	char Path[MAX_PATH + 1];
	char Delimiter;
	char LineTerminate;
	FileHeader FileInfo;
}MetaFile;

typedef struct
{
	char* sPointer;
	char* ePointer;
	long   Length;
}MetaToken;

typedef struct
{
	long      Line;
	long      TokenCount;
	MetaToken* TokenInfo;
}MetaData;

typedef struct
{
	void* pHeap;
	int PacketSize;
	size_t Step;
	size_t Size;
	size_t Count;
}Memory;

int Day;
int Month;
int Year;

size_t GetMem(Memory* pMem, size_t ReqPackets)
{
	if (pMem->Count + ReqPackets > pMem->Size)
	{
		size_t exSize;

		if (pMem->Count + ReqPackets >= pMem->Size)
			exSize = pMem->Count + ReqPackets + pMem->Step;
		else
			exSize = pMem->Size + pMem->Step;

		void* tHeap = realloc(pMem->pHeap, pMem->PacketSize * exSize);

		if (tHeap)
		{
			memset((byte*)tHeap + pMem->Size, 0, pMem->PacketSize * ((exSize - pMem->Size)));
			pMem->pHeap = tHeap;
			pMem->Size = exSize;
		}
		else
		{
			printf("Welcome to hell !! You have spent more money on ram bugger !!\n");
		}
	}
	pMem->Count++;
	return pMem->Count - 1;
}

MetaData* ReadMetaFile(void* MapData, LARGE_INTEGER FileSize, char delimiter, char lineTerminator, size_t* Line)
{
	Memory   sLine;
	Memory   sToken;

	memset(&sLine, 0, sizeof(Memory));
	sLine.Step = 100000;
	sLine.PacketSize = sizeof(MetaData);

	memset(&sToken, 0, sizeof(Memory));
	sToken.Step = 100000;
	sToken.PacketSize = sizeof(MetaToken);

	size_t Count = 0;
	size_t Chars = 0;

	MetaData* pMeta = NULL;
	MetaToken* pToken = NULL;

	size_t gLineCount = 0;
	size_t gTokenCount = 0;

	Chars = FileSize.QuadPart / sizeof(char);

	for (size_t i = 0; i < Chars; i++)
	{
		if (*((char*)MapData + i) == lineTerminator)
		{
			gTokenCount++;
			gLineCount++;
		}
		else if (*((char*)MapData + i) == delimiter)
		{
			gTokenCount++;
		}
	}

	if (*((char*)MapData + Chars - 1) != lineTerminator)
		gLineCount++;
	if (*((char*)MapData + Chars - 1) != delimiter)
		gTokenCount++;

	GetMem(&sLine, gLineCount);
	pMeta = (MetaData*)sLine.pHeap;

	GetMem(&sToken, gTokenCount);
	pToken = (MetaToken*)sToken.pHeap;

	(pMeta)->TokenInfo = pToken;
	(pMeta)->TokenInfo->sPointer = (char*)MapData;

	size_t lLineCount = 0;
	size_t lTokenCount = 0;

	for (size_t i = 0; i < Chars; i++)
	{
		if (i == Chars - 1 || *((char*)MapData + i) == lineTerminator)
		{
			(pMeta + lLineCount)->TokenCount++;
			lTokenCount++;

			if (i < Chars - 1)
			{
				(pMeta + lLineCount + 1)->TokenInfo = (MetaToken*)pToken + lTokenCount;
				((pMeta + lLineCount + 1)->TokenInfo + (pMeta + lLineCount + 1)->TokenCount)->sPointer = (char*)MapData + i + 1;
			}

			if ((pMeta + lLineCount)->TokenCount > 0)
			{
				if (i > 0)
				{
					((pMeta + lLineCount)->TokenInfo + (pMeta + lLineCount)->TokenCount - 1)->ePointer = (char*)MapData + i - 1;
				}
			}
			lLineCount++;
		}
		else if (*((char*)MapData + i) == delimiter)
		{
			(pMeta + lLineCount)->TokenCount++;
			lTokenCount++;

			if (i < Chars - 1)
			{
				((pMeta + lLineCount)->TokenInfo + (pMeta + lLineCount)->TokenCount)->sPointer = (char*)MapData + i + 1;
			}
			if ((pMeta + lLineCount)->TokenCount > 0)
			{
				if (i > 0)
				{
					((pMeta + lLineCount)->TokenInfo + (pMeta + lLineCount)->TokenCount - 1)->ePointer = (char*)MapData + i - 1;

					if (((pMeta + lLineCount)->TokenInfo + (pMeta + lLineCount)->TokenCount - 1)->sPointer)
					{
						((pMeta + lLineCount)->TokenInfo + (pMeta + lLineCount)->TokenCount - 1)->Length = ((pMeta + lLineCount)->TokenInfo + (pMeta + lLineCount)->TokenCount - 1)->ePointer - ((pMeta + lLineCount)->TokenInfo + (pMeta + lLineCount)->TokenCount - 1)->sPointer;
					}
				}
			}
		}
	}
	*Line = gLineCount;
	return pMeta;
}

void __inline CsvReader(MetaFile* pArg)
{
	pArg->FileInfo.NoofLines = 0;
	MetaData* Meta = NULL;
	HANDLE hTestw = CreateFileA(pArg->Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LARGE_INTEGER FileSize;
	GetFileSizeEx(hTestw, &FileSize);
	HANDLE hMapFile = CreateFileMapping(hTestw, NULL, PAGE_READONLY, FileSize.HighPart, FileSize.LowPart, 0);

	if (hMapFile)
	{
		int Err = GetLastError();
		void* MapData = (int*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);

		if (MapData)
		{
			Meta = ReadMetaFile(MapData, FileSize, pArg->Delimiter, pArg->LineTerminate, &pArg->FileInfo.NoofLines);

			if (Meta)
			{
				pArg->FileInfo.pVoid = calloc(pArg->FileInfo.NoofLines, sizeof(Data));
				Data* pData = (Data*)pArg->FileInfo.pVoid;

				if (pArg->FileInfo.pVoid)
				{
					for (size_t j = 0; j < pArg->FileInfo.NoofLines; j++)
					{
						char* StartPoint = NULL;
						char* EndPoint = NULL;
						long  length = 0;
						char String[1000] = "";

						char year[4], month[2], day[2];
						int year1, month1, day1;

						//column 1 - Date
						StartPoint = ((Meta + j)->TokenInfo + 0)->sPointer;
						EndPoint = ((Meta + j)->TokenInfo + 0)->ePointer;
						length = EndPoint - StartPoint + 1;

						memcpy(day, StartPoint, 2);
						memcpy(month, StartPoint + 3, 2);
						memcpy(year, StartPoint + 6, 4);

						day1 = atoi(day);
						month1 = atoi(month);
						year1 = atoi(year);

						if (Day == day1 && Month == month1 && Year == year1)
						{
							//column 2 - Description
							StartPoint = ((Meta + j)->TokenInfo + 1)->sPointer;
							EndPoint = ((Meta + j)->TokenInfo + 1)->ePointer;
							length = EndPoint - StartPoint + 1;

							memcpy(String, StartPoint, length);
							printf("Holiday: %s\n", String);
							break;
						}
						pArg->FileInfo.NoofValidLines++;

						printf("%d%d%d \t%s \n ", year1,month1,day1, String);
					}
				}
				free(Meta->TokenInfo);
				free(Meta);
			}
			UnmapViewOfFile(MapData);
		}
		CloseHandle(hMapFile);
	}
	CloseHandle(hTestw);
	return;
}

int main()
{
    const char* WeekDay[]= { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

	printf("Enter the date (DD MM YYYY):\n");
	scanf("%d %d %d", &Day, &Month, &Year);

	struct tm DateInfo { 0 };
    DateInfo.tm_mday = Day;  // day of the month - [1, 31]
    DateInfo.tm_mon = Month - 1;   // months since January - [0, 11]
    DateInfo.tm_year = Year - 1900;  // years since 1900

    // Day
    if (mktime(&DateInfo) == (time_t)-1) 
        printf("Error calculating the day of the week.\n");
    else 
        printf("\nDay: %s\n", WeekDay[DateInfo.tm_wday]);

    // Weekend
    if (DateInfo.tm_wday == 0 || DateInfo.tm_wday == 6)
        printf("It is a weekend\n");

    // Holiday
	MetaFile sMetaFile;
	memset(&sMetaFile, 0, sizeof(MetaFile));
	strcpy(sMetaFile.Path, "HolidayList.csv");
	sMetaFile.Delimiter = ',';
	sMetaFile.LineTerminate = '\n';
	CsvReader(&sMetaFile);

	return 0;
}

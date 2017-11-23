/**
*Buzz Chess Engine
*main.c
*
*Initializes the engine
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#if defined(_WIN32) || defined(_WIN64)
	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#include "eval.h"
#include "thread.h"
#include "search.h"
#include "defs.h"
#include "bitinstructions.h"
#include "movegen.h"
#include "board.h"
#include "xboard.h"
#include "consolecolors.h"
#include "hash.h"
#include "book.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "log.h"
#include "rand64.h"
#include "timemanager.h"
#include "bench.h"
#include "resultanalysis.h"

extern Mutex printLock; //from xboard.h
extern Mutex logLock;   //from log.h


void printArrU64(U64* arr, char* name)
{
	int i;
	print("const U64 %s[64]=\n",name);
	print("{\n");
	for(i=0;i<64;i++)
	{
		if(!(i&3)) print("\t");
		print("C64(0x");
		printHex(arr[i]);
		print(")");
		if(i!=63) print(",");
		if(i && !((i+1)&3)) print("\n");
		else print(" ");
	}
	print("};\n");
}

void makeKFillArrs()
{
	U64 KBFill[64];
	U64 KRFill[64];
	U64 KNFill[64];
	U64 KQFill[64];

	int i;
	for(i=0;i<64;i++)
	{
		KBFill[i] = Bfill(Kmoves(i),0);
		KRFill[i] = Rfill(Kmoves(i),0);
		KNFill[i] = Nfill(Kmoves(i));
		KQFill[i] = Qfill(Kmoves(i),0);
		if(KQFill[i]!=(KRFill[i]|KBFill[i]))
			print("Error\n");
	}
	printArrU64(KQFill,"KQFill");
	//printBB(KBFillNoOcc[E3]);
	//printBB(KRFill[E3]);
	//printBB(KNFill[E3]);
}

int main(int argc, char *argv[])
{
	//Initialize the engine
	seed(); //seed MT RNG
	generateZobrist();
	initMoveGen();
	initSearch();

	InitMutex(&printLock);
	InitMutex(&logLock);
	InitMutex(&rootEntryLock);

	srand((unsigned int)time(NULL));

	//parse command-line parameters
	if(argc>=2)
	{
		if(!strcmp(argv[1],"--h") || !strcmp(argv[1],"--help")
			||!strcmp(argv[1],"-h") || !strcmp(argv[1],"-help")
			|| !strcmp(argv[1],"/h") || !strcmp(argv[1],"/help")
			|| !strcmp(argv[1],"h") || !strcmp(argv[1],"help"))
		{
			printf
			(
				/**********************************************************************************/
				"\nBook-Making Options\n"
				"-------------------\n"
				"Buzz makebook <name> <plys> <games> <razor %%> <pgnFile1> <pgnFile2> ... <pgnFileN>\n"
				"     <name>      - name/location of the book file\n"
				"     <plys>      - how many plys to use in the book\n"
				"     <games>     - add only positions with atleast <games> games\n"
				"     <razor %%>   - positions that score below razor%% will get pruned\n"
				"     <pgnFileX>  - the locations of the pgn files to use\n"
				"\n"
				"Example: Buzz makebook grand.book 30 10 40 GM2001.pgn Euwe.pgn Massive.pgn\n"
				"Example: Buzz makebook " stringer("buzz books/grand.book") " 60 20 35.5 pgns/Massive.pgn\n"
				/**********************************************************************************/
				"\nRegular Options\n"
				"---------------\n"
				"Both - and / will be accepted as option indicators\n"
				"-bk <file>, -book <file>\n"
				"     Gives the location of the book file to use.\n"
				"     Example: -book small.book\n"
				"     Example: -book " stringer("mybooks/book with space.book") "\n"
				"     Default: default.book\n"
				"-h, -help\n"
				"     Show the command-line help.  This will only work by itself.\n"
				"-hash <megabytes>\n"
				"     Set the amout of memory to use for the hash table in megabytes.\n"
				"     Example: -hash 64\n"
				"     Default: 64 MB\n"
				"-icstalk <kibitz|whisper|adaptive|notalk>\n"
				"     Determine the manner in which analysis is given on an ics.\n"
				"     Parameter kibitz  : kibitz always\n"
				"     Parameter whisper : whisper always\n"
				"     Parameter adaptive: kibitz to computer, whisper to human\n"
				"     Parameter notalk  : no analysis information given\n"
				"     Example: -icstalk adaptive\n"
				"     Default: adaptive\n"
				"-l, -log\n"
				"     Buzz will write to a log named MMDDYY_HHMMSS_log.html.\n"
				"     MMDDYY_HHMMSS represents the time of creation where\n"
				"     MM=month, DD=day, YY=year, HH=hour, MM=minute, and SS=second.\n"
				#ifdef ANDRES_EVAL
				"-degbb <bitbase path> (Dirty only option)\n"
				"-dcache <bitbase cache size> (Dirty only option)\n"
				#endif
			);
			return EXIT_SUCCESS;
		}
		else if(!strcmp(argv[1],"makebook"))
		{
			makebook(argc, argv);
			return EXIT_SUCCESS;
		}
		else if(*argv[1]=='-' || *argv[1]=='/') //run normally with command-line parameters
		{
			int i;
			unsigned int hashTableSize=DEFAULT_HASH_TABLE_SIZE;
			argc--;
			for(i=1; argc; argc--, i++)
			{
				char* option=argv[i];
				if(*option!='-' && *option!='/')
				{
					printError("misplaced command-line option", argv[i]);
					continue;
				}
				option++;
				if(!strcmp(option,"book") || !strcmp(option,"bk"))
				{
					argc--; i++;
					if(!argc)
					{
						printError("missing argument",argv[i-1]);
						break;
					}
					strcpy(bookPath,argv[i]);
				}
				else if(!strcmp(option,"hash"))
				{
					unsigned int mb;
					argc--; i++;
					if(!argc)
					{
						printError("missing argument",argv[i-1]);
						break;
					}
					if(sscanf(argv[i],"%u",&mb)!=1 || !mb)
					{
						argc++; i--;
						printError("bad argument",argv[i]);
						continue;
					}
					hashTableSize=mb*1024*1024;
				}
				else if(!strcmp(option,"icstalk"))
				{
					argc--; i++;
					if(!argc)
					{
						printError("missing argument",argv[i-1]);
						break;
					}
					
					if(!strcmp(argv[i],"kibitz"))
						icsInfoTalk = ICS_INFOTALK_KIBITZ;
					else if(!strcmp(argv[i],"whisper"))
						icsInfoTalk = ICS_INFOTALK_WHISPER;
					else if(!strcmp(argv[i],"adaptive"))
						icsInfoTalk = ICS_INFOTALK_ADAPTIVE;
					else if(!strcmp(argv[i],"notalk"))
						icsInfoTalk = ICS_INFOTALK_NOTALK;
					else
					{
						argc++; i--;
						printError("bad argument",argv[i]);
						continue;
					}
				}
				else if(!strcmp(option,"log") || !strcmp(option,"l"))
					startLogHTML();

				//ANDRES COMMAND LINE
				#ifdef ANDRES_EVAL
				else if(!strcmp(option,"degbb"))
				{
					argc--; i++;
					if(!argc)
					{
						printError("missing argument",argv[i-1]);
						break;
					}
					strcpy(degbb,argv[i]);
				}
				else if(!strcmp(option,"dcache"))
				{
					argc--; i++;
					if(!argc)
					{
						printError("missing argument",argv[i-1]);
						break;
					}
					if(sscanf(argv[i],"%d",&dcache)!=1)
					{
						argc++; i--;
						printError("bad argument",argv[i]);
						continue;
					}
				}
				#endif

				else
					printError("unknown switch",argv[i]);
			}
			initHashTable(hashTableSize);
		}
	}
	else //no parameters passed, do default operations
		initHashTable(DEFAULT_HASH_TABLE_SIZE);

	//Is the program opened in console?
	{
		#if defined(_WIN32) || defined(_WIN64)
			DWORD dwConsoleMode/*, dwAvailable*/;
			HANDLE hInput;
			hInput = GetStdHandle(STD_INPUT_HANDLE);
			if(!GetConsoleMode(hInput,&dwConsoleMode))
     			openedInConsole=/*PeekNamedPipe(hInput,0,0,0,&dwAvailable,0)==0*/false;
			else
				openedInConsole=true;
		#else //assume Linux/UNIX
			openedInConsole=isatty(fileno(stdin));
		#endif
	}
	
	if(openedInConsole)
		printf
		(
			PROGRAM_VERSION_STRING "\n"
			#ifndef ANDRES_EVAL
				"Copyright (C) " COPYRIGHT_YEAR_STRING " Pradyumna Kannan\n\n"

				"Buzz is provided " stringer("as-is") " without any express or implied warranty. In no event\n"
				"will I (Pradyumna Kannan) be held liable for any damages arising from the use\n"
				"of Buzz.\n"
			#endif
		);


	#ifdef ANDRES_EVAL
		init_AndresEval();
	#endif

	xboard();
	stopLogHTML();

	DeleteMutex(rootEntryLock);
	DeleteMutex(printLock);
	DeleteMutex(logLock);
	return EXIT_SUCCESS;
}

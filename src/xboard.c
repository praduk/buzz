/**
*Buzz Chess Engine
*xboard.c
*
*Manages the Winboard/xboard protocol.
*
*Copyright (C) 2007 Pradu Kannan.
**/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "thread.h"
#include "search.h"
#include "defs.h"
#include "movegen.h"
#include "moveordering.h"
#include "timemanager.h"
#include "xboard.h"
#include "bitinstructions.h"
#include "eval.h"
#include "hash.h"
#include "consolecolors.h"
#include "rand64.h"
#include "book.h"
#include "log.h"
#include "recog.h"
#include "bench.h"
#include "resultanalysis.h"

board gamePosition[MAX_GAME_MOVES];
unsigned int moveNumber;
unsigned int trueMoveNumber;
unsigned int compSide;
bool openedInConsole;
bool post;
bool ponder;
bool analyze;
bool opponentIsComputer;
char opponentName[MAX_CHARS_IN_NAMES];
char ics[MAX_CHARS_IN_NAMES]="-";
unsigned int myRating;
unsigned int opponentRating;
unsigned int sd;
U64 ponderHashKey;
move ponderMove;
int icsInfoTalk = ICS_INFOTALK_DEFAULT;

Mutex actionLock; //Locks to certain actions that cannot be done together
                  //Like the process of starting the search and setting
                  //the time manager
Mutex printLock;  //Locks whenever one is trying to print to stdout

//replacement for print - can be used to catch all output to a logfile ect..
void print(const char *format , ...)
{
	va_list ap;
	LockM(printLock);
	//stdout
	va_start(ap,format);
	vprintf(format,ap);
	va_end(ap);
	
	//log-file
	va_start(ap,format);
	vprintLogHTML(0,format,ap);
	va_end(ap);
	ReleaseM(printLock);
}

//private function to reset some game data
void resetGameSpecificData()
{
	myRating=0;
	opponentRating=0;
	*opponentName='\0';
	opponentIsComputer=false;
}

//returns true on illegal move
bool makeGameMove(move m)
{
	board pos2;
	
	
	pos2=gamePosition[moveNumber];
	assert(moveNumber<(MAX_GAME_MOVES-1));
	
	//Legality checking
	if(!pos2.AllPieces)
	{
		coordString moveString;
		moveToCoords(gamePosition+moveNumber,m,moveString);
		CC_Red();
		print("Illegal Move: %s\n",moveString);
		CC_Normal();
		return true;
	}
	{
		moveList ml;
		unsigned int i;
		genMoves(&pos2,&ml);
		if(m==NULLMOVE)
		{
			if(!inCheck(pos2,pos2.side))
				goto moveIsOk;
			else
			{
				CC_Red();
				print("Illegal Move: Null\n");
				CC_Normal();
				return true;
			}
		}
		for(i=0;i<ml.moveCount;i++)
			if(ml.moves[i].m==m)
				goto moveIsOk;
		{
			coordString moveString;
			moveToCoords(gamePosition+moveNumber,m,moveString);
			CC_Red();
			print("Illegal Move: %s\n",moveString);
			CC_Normal();
			return true;
		}
	}

moveIsOk:


	makemove(&pos2,m);
	
	gamePosition[++moveNumber]=pos2;
	trueMoveNumber++;

	printLogHTML(HTMLLOG_COLOR_BLUE,"Played move %u\n",m);
	logfen(gamePosition+moveNumber);

	return false;
}

//What to do to stop 2 search threads from starting??
void makeComputerGameMove(move m)
{
	LockM(actionLock);
	
	if(gamePosition[moveNumber].side!=compSide)
	{
		
		goto pondering_section;
	}
	
	if(!openedInConsole)
	{
		coordString cs;
		
		moveToCoords(gamePosition+moveNumber,m,cs);
		
		print("move %s\n",cs);
	}
	else
	{
		sanString ss;
		moveToSAN(gamePosition+moveNumber,m,ss);
		print("move %s\n",ss);
	}

	if(*ics!='-' && icsInfoTalk!=ICS_INFOTALK_NOTALK)
	{
		hashEntry he;
		if(probeHash(gamePosition[moveNumber].hashkey,&he))
		{
			int depth=he.depth;
			int score=he.score;
			time_ms time=timeUsed();
			if(depth>=0)
			{
				if(icsInfoTalk==ICS_INFOTALK_WHISPER || 
					(icsInfoTalk==ICS_INFOTALK_ADAPTIVE && !opponentIsComputer))
					print("tellothers ");
				else
					print("tellall ");
				print("Depth=%d Score=%d Time(s)=%.0f Nodes=",depth,score,(float)time/1000);
				printU64(nodes);
				print(" KNPS=%.0f PV=",(float)nodes/(float)time);
				
				printPV(gamePosition[moveNumber]);
				
				print("\n");
			}
		}
	}

	makeGameMove(m);
	if(claimResult())
	{
		ReleaseM(actionLock);
		return;
	}
pondering_section:
	if(ponder && compSide==gamePosition[moveNumber].xside) //pondering
	{
		hashEntry probe;
		if(probeHash(gamePosition[moveNumber].hashkey,&probe))
		{
			ponderMove=extractMove(probe.data);
			if(ponderMove!=NULLMOVE);
			{
				bwd* arg=(bwd*)malloc(sizeof(bwd));
				arg->b=gamePosition[moveNumber];
				if(sd) arg->d=sd; else arg->d=MAX_SEARCH_DEPTH;
				makemove(&(arg->b),ponderMove);
				//if posiiton is found in book, don't ponder
				if(getBookMove(&arg->b,NULL)!=NULLMOVE)
				{
					free(arg);
					ReleaseM(actionLock);
					return;
				}
				ponderHashKey=(arg->b).hashkey;
				gamePosition[moveNumber+1]=arg->b;
				moveNumberDrawDetection=moveNumber+1;
				setEndTime();
				infiniteTime();
				searchThread=BeginThread(searchStart,(void*)arg);
			}
		}
	}
	ReleaseM(actionLock);
}

void printError(char* reason, char* line)
{
	CC_Red();
	print("Error (%s): %s\n",reason,line);
	CC_Normal();
}

void xboard()
{
	char line[2048];

	//use unbuffered input and output
	setbuf(stdout,NULL);
	setbuf(stdin,NULL);
	setvbuf(stdout,NULL,_IONBF,0);
	setvbuf(stdin,NULL,_IONBF,0); 
	//ignore signals SIGINT and SIGTERM
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	*gamePosition=startPos;

	moveNumber=0;
	trueMoveNumber=2;
	compSide=BLACK;
	post=true;
	analyze=false;

	print("\n"); //Winboard protocol requires a newline after initialization


	InitMutex(&actionLock);
	while(fgets(line,2048,stdin))
	{
		char command[2048];
		move m;
		
		if(*line=='\0' || *line=='\n')
			continue;
		//remove newline
		line[strlen(line)]='\0';
		printLogHTML(HTMLLOG_COLOR_RED,"%u > %s\n",getms(),line);
		sscanf(line,"%s",command);

		/********** State changing xboard commands **********/
		LockM(actionLock);
		//Is it a move?

		
		m=stringToMove(gamePosition+moveNumber,line);
		
		
		if(m!=NULLMOVE)
			makeGameMove(m);
		else if(!strcmp(command,"null") || !strcmp(command,"Null") || !strcmp(command,"NULL"))
			makeGameMove(NULLMOVE);
		else if(!strcmp(command,"usermove"))
		{
			char moveString[2048];
			
			sscanf(line,"usermove %s",moveString);
			m=stringToMove(gamePosition+moveNumber,moveString);
			if(m!=NULLMOVE)
				makeGameMove(m);
			else
			{
				CC_Red();
				print("Illegal move: %s\n",moveString);
				logfen(gamePosition+moveNumber);
				
				CC_Normal();
				ReleaseM(actionLock);
				
				continue;
			}
			
		}
		else if(!strcmp(line,"null") || !strcmp(line,"usermove null")) //debugging not verified
			makeGameMove(NULLMOVE);
		else if(!strcmp(command,"new"))
		{
			
			*gamePosition=startPos;
			moveNumber=0;
			sd=0;
			trueMoveNumber=2;
			compSide=BLACK;
			resetGameSpecificData();
			
		}
		else if(!strcmp(command,"undo"))
		{
			if(moveNumber)
			{
				moveNumber--;
				trueMoveNumber--;
			}
			else
				printError("command not legal now",command);
		}
		else if(!strcmp(command,"remove"))
		{
			if(moveNumber>=2)
			{
				moveNumber-=2;
				trueMoveNumber-=2;
			}
			else
				printError("command not legal now",command);
		}
		else if(!strcmp(command,"white"))
		{
			compSide=BLACK;
			gamePosition[moveNumber].side=WHITE;
			gamePosition[moveNumber].xside=BLACK;
		}
		else if(!strcmp(command,"black"))
		{
			compSide=WHITE;
			gamePosition[moveNumber].side=BLACK;
			gamePosition[moveNumber].xside=WHITE;
		}
		else if(!strcmp(command,"playother"))
		{
			compSide=gamePosition[moveNumber].xside;
		}
		else if(!strcmp(command,"go"))
		{
			compSide=gamePosition[moveNumber].side;
		}
		else if(!strcmp(command,"?"))
		{
			ReleaseM(actionLock);
			stopSearch();
			LockM(actionLock);
			ponderHashKey=INVALID_HASH_KEY;
		}
		else if(!strcmp(command,"edit"))
		{
			resetGameSpecificData();
			CC_Red();
			edit();
			CC_Normal();
		}
		else if(!strcmp(command,"setboard"))
		{
			resetGameSpecificData();
			CC_Red();
			setboard(line+9);
			//gamePosition->hashkey=1;
			CC_Normal();
		}
		else if(!strcmp(command,"analyze"))
		{
			post=true;
			analyze=true;
			compSide=NOSIDE;
		}
		else if(!strcmp(command,"exit"))
		{
			analyze=false;
			ReleaseM(actionLock);
			stopSearch();
			LockM(actionLock);
		}
		/*else if(!strcmp(command,"dtstest"))
		{
			int d;
			int i;
			initParallelSearch();
			
			sscanf(line,"dtstest %d",&d);
			initParallelSearch();
			resetNodeCount();
			nodes = 0;
			setStartTime();
			rootNode.pos = gamePosition[moveNumber];
			
			rootNode.dfr   = 0;
			rootNode.alpha = -INF;
			rootNode.beta = INF;
			rootNode.m = NULLMOVE;
			rootNode.nodeType = OPEN_NODE;
			genMoves(&rootNode.pos,&rootNode.ml);
			resetStack(Stacks+1);
			SearchThread[1]=BeginThread(SearchThreadStart,(void*)1);
			for(i=1;i<=d;i++)
			{
				rootNode.depth = i;
				orderMoves(&rootNode.pos,&rootNode.ml,0);
				resetNode(&rootNode);
				resetSplitData(&rootNode);
				resetStack(Stacks);
				rootNode.workLeft = rootNode.ml.moveCount;
				Stacks->assignedNode = &rootNode;
				Stacks->haswork = true;
				*SearchThread=BeginThread(SearchThreadStart,0);
				Join(*SearchThread);
			}
			Stacks[1].terminate;
			Join(SearchThread[1]);
			deleteParallelSearch();
		}*/
		else
		{
			ReleaseM(actionLock);
			goto nonsc;
		}


		//START: common to all state-changing commands
		{
			board pos=gamePosition[moveNumber];
			moveList ml;
			genMoves(&pos,&ml);

			//result claiming
			if(claimResult())
			{
				ReleaseM(actionLock);
				continue;
			}
			if(analyze) //analyze mode
			{
				bwd* arg=(bwd*)malloc(sizeof(bwd));
				arg->d=MAX_SEARCH_DEPTH;
				arg->b=gamePosition[moveNumber];
				moveNumberDrawDetection=moveNumber;
				ReleaseM(actionLock);
				stopSearch();
				LockM(actionLock);
				infiniteTime();
				searchThread=BeginThread(searchStart,(void*)arg);
			}
			else if(compSide==gamePosition[moveNumber].side) //myturn
			{
				//book move
				if(ml.moveCount>=2)
				{
					move m=getBookMove(gamePosition+moveNumber,NULL);
					if(m!=NULLMOVE)
					{
						ReleaseM(actionLock);
						stopSearch();
						LockM(actionLock);
						ponderHashKey=INVALID_HASH_KEY;
						printLogHTML(HTMLLOG_COLOR_GREEN,"Making book move\n",pos.hashkey);
						makeComputerGameMove(m); //Immediate Lock/Unlock of action
						ReleaseM(actionLock);
						continue;
					}
				}
				//check if searching and that you pondered the right move
				if(ponder && searchRunning && ponderHashKey==gamePosition[moveNumber].hashkey)
					finiteTime();
				else //Start search thread
				{
					bwd* arg=(bwd*)malloc(sizeof(bwd));
					ReleaseM(actionLock);
					stopSearch();
					LockM(actionLock);
					ponderHashKey=INVALID_HASH_KEY;
					{
						arg->b=gamePosition[moveNumber];
						if(sd) arg->d=sd; else arg->d=MAX_SEARCH_DEPTH;
						moveNumberDrawDetection=moveNumber;
						setEndTime();
						searchThread=BeginThread(searchStart,(void*)arg);
					}
				}
			}
			
		}
		ReleaseM(actionLock);
		continue;
		//END: common to all state-changing commands




//non-state changing commands
nonsc:

		/********** Buzz specific commands **********/

		if(!strcmp(command,"help"))
			print
			(
				"\n"
				"bench          - runs a benchmark\n"
				"clock          - prints the time since the epoch in milliseconds\n"
				"d              - prints the current board position\n"
				"divide <depth> - does a perft for all legal moves individually\n"
				"eb <W> <D> <L> - ELO rating and error bars given a result\n"
				"eval           - prints the eval for the current position\n"
				"fen            - prints the current board position in fen\n"
				"hash <memory>  - allocate memory to the hash table (extensions: bytes kb mb gb)\n"
				"                 example: hash 523 kb\n"
				"hashkey        - prints the hexadecimal hashkey of the current board position\n"
				"hashinfo       - prints information on the current position from the hashtable\n"
				"info           - prints some info on the current executable\n"
				"legalmoves     - prints the legal moves for the current board position\n"
				"perft <depth>  - starts a perft thread for the current position\n"
				"quies/qsearch  - prints the qsearch for the current position\n"
				"see <move>     - prints the static exchange evaluvation of a move\n"
				"\n"
			);
		else if(!strcmp(command,"info"))
		{
			print
			(
				"\n"
				"Program Version: " PROGRAM_VERSION_STRING "\n"
				"Build Date: " __DATE__ "\n"
				"Build Time: " __TIME__ "\n"

				#ifdef __INTEL_COMPILER
					"Built with Intel C/C++ %d.%.2d\n"
					#ifdef __LP64__
						"64-bit build\n"
					#else
						"32-bit build\n"
					#endif
				#elif defined(_MSC_VER)
					"Built with Microsoft C/C++ Compiler %d.%.2d\n"
					#ifdef _WIN64
						"64-bit build\n"
					#else
						#if defined(_WIN32)
							"32-bit build\n"
						#elif defined(_WIN64)
							"64-bit Build\n"
						#endif
					#endif
				#elif defined(__GNUC__)
					"Built with GNU C/C++ Compiler " __VERSION__ "\n"
					#ifdef __LP64__
						"64-bit build\n"
					#else
						"32-bit build\n"
					#endif
				#endif

				#ifdef __TURBOC__
					"Built by Borland Turbo C " __TURBOC__ "\n"
				#endif

				#ifdef __BORLANDC__
					"Built by Borland C/C++ " __BORLANDC__ "\n"
				#endif

				"\n"
				
				#ifdef __INTEL_COMPILER
					,__INTEL_COMPILER/100,__INTEL_COMPILER%100
				#elif defined(_MSC_VER)
					,_MSC_VER/100,_MSC_VER%100
				#endif
			);
		}
		else if(!strcmp(command,"bench"))
			bench();
		else if(!strcmp(command,"clock"))
			print("%u ms\n",getms());
		else if(!strcmp(command,"d"))
			printBoard(gamePosition+moveNumber);
		else if(!strcmp(command,"divide"))
		{
			int depth;
			sscanf(line,"divide %d",&depth);
			if(depth<=1)
			{
				unsigned int i;
				moveList ml;
				genMoves(gamePosition+moveNumber,&ml);
				for(i=0;i<ml.moveCount;i++)
				{
					coordString moveStr;
					moveToCoords(gamePosition+moveNumber,ml.moves[i].m,moveStr);
					print("%s\n",moveStr);
				}
			}
			else
				divide(gamePosition[moveNumber],depth);
		}
		else if(!strcmp(command,"eval"))
			print("eval %d\n",eval(gamePosition+moveNumber,-INF,INF));
		else if(!strcmp(command,"eb")) //short for error bars
		{
			int numread;
			unsigned int W,D,L;
			numread=sscanf(line,"eb %u %u %u",&W,&D,&L);
			if(numread<3) printError("not enough parameters",line);
			else
			{
				double ELOmin, ELOnorm, ELOmax;
				printf("80%% confidence ");
				errorBars(W,D,L,1.28155,&ELOmin,&ELOnorm,&ELOmax);
				ELOmin=winToELO(ELOmin); ELOnorm=winToELO(ELOnorm); ELOmax=winToELO(ELOmax);
				printf("%4.0f <= %4.0f <= %4.0f\n",ELOmin, ELOnorm, ELOmax);
				printf("90%% confidence ");
				errorBars(W,D,L,1.64485,&ELOmin,&ELOnorm,&ELOmax);
				ELOmin=winToELO(ELOmin); ELOnorm=winToELO(ELOnorm); ELOmax=winToELO(ELOmax);
				printf("%4.0f <= %4.0f <= %4.0f\n",ELOmin, ELOnorm, ELOmax);
				printf("95%% confidence ");
				errorBars(W,D,L,1.95996,&ELOmin,&ELOnorm,&ELOmax);
				ELOmin=winToELO(ELOmin); ELOnorm=winToELO(ELOnorm); ELOmax=winToELO(ELOmax);
				printf("%4.0f <= %4.0f <= %4.0f\n",ELOmin, ELOnorm, ELOmax);
				printf("98%% confidence ");
				errorBars(W,D,L,2.32635,&ELOmin,&ELOnorm,&ELOmax);
				ELOmin=winToELO(ELOmin); ELOnorm=winToELO(ELOnorm); ELOmax=winToELO(ELOmax);
				printf("%4.0f <= %4.0f <= %4.0f\n",ELOmin, ELOnorm, ELOmax);
				printf("99%% confidence ");
				errorBars(W,D,L,2.57583,&ELOmin,&ELOnorm,&ELOmax);
				ELOmin=winToELO(ELOmin); ELOnorm=winToELO(ELOnorm); ELOmax=winToELO(ELOmax);
				printf("%4.0f <= %4.0f <= %4.0f\n",ELOmin, ELOnorm, ELOmax);
			}
		}
		else if(!strcmp(command,"epd"))
			testEPD(line+4);

		else if(!strcmp(command,"fen"))
			fen(gamePosition+moveNumber);
			
		else if(!strcmp(command,"hashkey"))
		{
			print("hashkey ");
			printHex(gamePosition[moveNumber].hashkey);
			print("\n");
		}
		else if(!strcmp(command,"hash"))
		{
			int numread;
			unsigned int bytes;
			char type[2048];
			numread=sscanf(line,"hash %u %s",&bytes,type);
			if(numread<1)
				printError("missing parameter",line);
			if(!strcmp(type,"bytes")); //do nothing for bytes
			if(*type=='k' || *type=='K') //kilobytes
				bytes*=1024;
			else if(*type=='g' || *type=='G') //gigabytes
				bytes*=1024*1024*1024;
			else //megabytes
				bytes*=1024*1024;

			if(bytes<64)
				printError("please allocate atleast 64 bytes",line);
			else
			{
				initHashTable(bytes);
				if(hashTable==NULL)
					printError("could not allocate memory",line);
			}
		}
		else if(!strcmp(command,"hashinfo") || !strcmp(command,"hashentry"))
		{
			hashEntry he;
			if(probeHash(gamePosition[moveNumber].hashkey,&he))
			{
				sanString s;
				moveToSAN(gamePosition+moveNumber,extractMove(he.data),s);
				print
				(
					"Depth   %d\n"
					"Score   %d\n"
					"Bound   %s\n"
					"Move    %s\n\n",
					he.depth,
					he.score,
					extractFlag(he.data)==HASHFLAG_EXACT?"Exact":extractFlag(he.data)==HASHFLAG_UPPER_BOUND?"Upper":"Lower",
					s
				);
			}
			else print("No entry found.\n");
		}
		else if(!strcmp(command,"legalmoves"))
		{
			unsigned int i;
			moveList ml;
			
			genMoves(gamePosition+moveNumber,&ml);
			orderMoves(gamePosition+moveNumber,&ml,0);
			for(i=0;i<ml.moveCount;i++)
			{
				sanString moveStr;
				coordString moveStr2;
				moveToSAN(gamePosition+moveNumber,ml.moves[i].m,moveStr);
				moveToCoords(gamePosition+moveNumber,ml.moves[i].m,moveStr2);
				print("%2d) %8s %5s ",i+1,moveStr,moveStr2);
				if(ml.moves[i].score==MOVEORDERING_HASHMOVE)
					print("(HASHMOVE)\n");
				else if(ml.moves[i].score>MOVEORDERING_CAPTURE)
					print("(SEE %d)\n",ml.moves[i].score-MOVEORDERING_CAPTURE);
				else if(ml.moves[i].score<-MOVEORDERING_CAPTURE)
					print("(SEE %d)\n",ml.moves[i].score+MOVEORDERING_CAPTURE);
				else if(ml.moves[i].score==MOVEORDERING_CHECK)
					print("(CHECK)\n");
				else if(ml.moves[i].score==MOVEORDERING_PASSER)
					print("(PASSER)\n");
				else if(ml.moves[i].score==MOVEORDERING_EQUALCAP)
					print("(CAPTURE)\n");
				else
					print("(%d)\n",ml.moves[i].score);
			}
			
		}
		else if(!strcmp(command,"perft"))
		{
			bwd* args=malloc(sizeof(bwd));
			sscanf(line,"perft %d",&(args->d));
			
			args->b=gamePosition[moveNumber];
			
			if(args->d<=1)
			{
				moveList ml;
				genMoves(&args->b,&ml);
				print("%u legal moves\n",ml.moveCount);
			}
			else
				BeginDThread(perftStart,(void*)(args));
		}
		else if(!strcmp(command,"qmoves"))
		{
			unsigned int i;
			moveList ml;
			
			genQMoves(gamePosition+moveNumber,&ml);
			orderQMoves(gamePosition+moveNumber,&ml);
			for(i=0;i<ml.moveCount;i++)
			{
				sanString moveStr;
				coordString moveStr2;
				moveToSAN(gamePosition+moveNumber,ml.moves[i].m,moveStr);
				moveToCoords(gamePosition+moveNumber,ml.moves[i].m,moveStr2);
				print("%2d) %8s %5s ",i+1,moveStr,moveStr2);
				if(ml.moves[i].score==MOVEORDERING_HASHMOVE)
					print("(HASHMOVE)\n");
				else if(ml.moves[i].score>MOVEORDERING_CAPTURE)
					print("(SEE %d)\n",ml.moves[i].score-MOVEORDERING_CAPTURE);
				else if(ml.moves[i].score<-MOVEORDERING_CAPTURE)
					print("(SEE %d)\n",ml.moves[i].score+MOVEORDERING_CAPTURE);
				else if(ml.moves[i].score==MOVEORDERING_CHECK)
					print("(CHECK)\n");
				else if(ml.moves[i].score==MOVEORDERING_PASSER)
					print("(PASSER)\n");
				else if(ml.moves[i].score==MOVEORDERING_EQUALCAP)
					print("(CAPTURE)\n");
				else
					print("(%d)\n",ml.moves[i].score);
			}
			
		}
		else if(!strcmp(command,"qcmoves"))
		{
			unsigned int i;
			moveList ml;
			
			genQCMoves(gamePosition+moveNumber,&ml);
			orderQMoves(gamePosition+moveNumber,&ml);
			for(i=0;i<ml.moveCount;i++)
			{
				sanString moveStr;
				coordString moveStr2;
				moveToSAN(gamePosition+moveNumber,ml.moves[i].m,moveStr);
				moveToCoords(gamePosition+moveNumber,ml.moves[i].m,moveStr2);
				print("%2d) %8s %5s ",i+1,moveStr,moveStr2);
				if(ml.moves[i].score==MOVEORDERING_HASHMOVE)
					print("(HASHMOVE)\n");
				else if(ml.moves[i].score>MOVEORDERING_CAPTURE)
					print("(SEE %d)\n",ml.moves[i].score-MOVEORDERING_CAPTURE);
				else if(ml.moves[i].score<-MOVEORDERING_CAPTURE)
					print("(SEE %d)\n",ml.moves[i].score+MOVEORDERING_CAPTURE);
				else if(ml.moves[i].score==MOVEORDERING_CHECK)
					print("(CHECK)\n");
				else if(ml.moves[i].score==MOVEORDERING_PASSER)
					print("(PASSER)\n");
				else if(ml.moves[i].score==MOVEORDERING_EQUALCAP)
					print("(CAPTURE)\n");
				else
					print("(%d)\n",ml.moves[i].score);
			}
			
		}
		else if(!strcmp(command,"quies") || !strcmp(command,"qsearch"))
		{
			board copy;
			if(searchRunning)
				print("Quies cannot be done when a search is already running\n");
			else
			{
				copy = gamePosition[moveNumber];
				searchRunning=true;
				infiniteTime();
				print("qsearch = %d\n", qsearchRoot(&copy,-INF,INF,1));
				finiteTime();
				searchRunning=false;
			}
		}
		else if(!strcmp(command,"see"))
		{
			char moveString[2048];
			
			if(sscanf(line,"see %s",moveString)==1)
			{
				move m=stringToMove(gamePosition+moveNumber,moveString);
				if(m==NULLMOVE)
					printError("Illegal move",line);
				else
					print("see = %d\n",SEE(gamePosition+moveNumber,m));
			}
			else
				printError("Not enough parameters",line);
			
		}
		else if(!strcmp(command,"symcolor"))
		{
			U64 temp;
			unsigned char temp2;
			if(gamePosition[moveNumber].Pieces[P])
				print("Pawns on board, cannot do symmetry on color\n");
			else
			{
				//switch side to play
				gamePosition[moveNumber].side=!gamePosition[moveNumber].side;
				gamePosition[moveNumber].xside=!gamePosition[moveNumber].xside;
				//switch pieces
				temp=gamePosition[moveNumber].PiecesSide[BLACK];
				gamePosition[moveNumber].PiecesSide[BLACK]=gamePosition[moveNumber].PiecesSide[WHITE];
				gamePosition[moveNumber].PiecesSide[WHITE]=temp;
				//switch king positions
				temp2=gamePosition[moveNumber].KingPos[BLACK];
				gamePosition[moveNumber].KingPos[BLACK]=gamePosition[moveNumber].KingPos[WHITE];
				gamePosition[moveNumber].KingPos[WHITE]=temp2;
				//switch castling
				temp2=gamePosition[moveNumber].castling;
				gamePosition[moveNumber].castling=((temp2&BCASTLE)>>2) | ((temp2&WCASTLE)<<2); 
			}
		}

		/********* "Easter Eggs" **********/
		else if(!strcmp(command,"Yellow") || !strcmp(command,"yellow"))
			print("Jackets\n");
		else if(!strcmp(command,"Jackets") || !strcmp(command,"jackets"))
			print("Yellow\n");

		/********** Xboard commands **********/
		else if(!strcmp(command,"."));
		else if(!strcmp(command,"xboard")) print("\n");
		else if(!strcmp(command,"random"));
		else if(!strcmp(command,"accepted"));
		else if(!strcmp(command,"rejected"));
		else if(!strcmp(command,"post"))
			post=true;
		else if(!strcmp(command,"nopost"))
			post=false;
		else if(!strcmp(command,"hard"))
			ponder=true;
		else if(!strcmp(command,"easy"))
		{
			
			if(compSide!=gamePosition[moveNumber].side)
				stopSearch();
			
			ponder=false;
		}
		else if(!strcmp(command,"force"))
			compSide=NOSIDE;
		else if(!strcmp(command,"ping"))
		{
			int ping;
			sscanf(line,"%*s %d",&ping);
			print("pong %d\n",ping);
		}
		else if(!strcmp(command,"time"))
		{
			sscanf(line,"%*s %d",&myTime);
			myTime*=10;
		}
		else if(!strcmp(command,"otim"))
		{
			sscanf(line,"%*s %d",&opponentTime);
			opponentTime*=10;
		}
		else if(!strcmp(command,"sd"))
		{
			sscanf(line,"%*s %d",&sd);
			st=0;
			mps=0;
			inc=0;
		}
		else if(!strcmp(command,"st"))
		{
			int retval=sscanf(line,"%*s %d",&st);
			if(retval==EOF || retval!=1)
				printError("incorrect number of parameters",line);
			sd=0;
			mps=0;
			inc=0;
		}
		else if(!strcmp(command,"level"))
		{
			char levelTime[2048];
			bool hasSeconds=false;
			int i;
			st=0;
			sd=0;
			i=sscanf(line,"level %d %s %d",&mps,levelTime,&inc);
			if(i==EOF || i!=3)
				printError("incorrect number of parameters",line);
			for(i=0;i<2048 || levelTime[i]=='\0';i++)
				if(levelTime[i]==':')
				{
					hasSeconds=true;
					break;
				}
			if(hasSeconds)
			{
				unsigned int minutes;
				unsigned int seconds;
				sscanf(levelTime,"%u:%u",&minutes,&seconds);
				myTime=1000*(minutes*60+seconds);
			}
			else
			{
				unsigned int minutes;
				sscanf(levelTime,"%u",&minutes);
				myTime=60000*minutes;
			}
			opponentTime=myTime;
			nextTC=myTime;
		}
		else if(!strcmp(command,"computer"))
			opponentIsComputer=true;
		else if(!strcmp(command,"rating"))
			sscanf(line,"%*s %d %d",&myRating,&opponentRating);
		else if(!strcmp(command,"name"))
			sscanf(line,"%*s %[^\n]",opponentName);
		else if(!strcmp(command,"ics"))
			sscanf(line,"%*s %[^\n]",ics);
		else if(!strcmp(command,"quit"))
			break;
		else if(!strcmp(command,"protover"))
			print
			(
				"feature done=0\n"
				"feature ping=1\n"
				"feature playother=1\n"
				"feature san=0\n" //Buzz can handle both SAN and coordinate notation
				"feature usermove=1\n"
				"feature time=1\n"
				"feature draw=1\n"
				"feature sigint=0\n"
				"feature sigterm=0\n"
				"feature reuse=1\n"
				"feature analyze=1\n"
				"feature myname=\"" PROGRAM_VERSION_STRING "\"\n"
				"feature variants=" stringer("normal") "\n"
				"feature colors=0\n"
				"feature ics=1\n"
				"feature name=1\n"
				"feature pause=0\n"
				"feature done=1\n"
			);
		else if(!strcmp(command,"bk"))
		{
			char info[2048];
			getBookMove(gamePosition+moveNumber,info);
			print(" %s\n\n",info);
		}
		else if(!strcmp(command,"hint"))
		{
			
			if(gamePosition[moveNumber].AllPieces!=U64EMPTY)
			{
				move m=getBookMove(gamePosition+moveNumber,NULL);
				
				if(m!=NULLMOVE)
				{
					coordString str;
					moveToCoords(gamePosition+moveNumber,m,str);
					print("Hint: %s\n",str);
				}
				else
				{
					hashEntry probe;
					if(probeHash(gamePosition[moveNumber].hashkey,&probe))
					{
						m=extractMove(probe.data);
						if(m!=NULLMOVE)
						{
							coordString str;
							moveToCoords(gamePosition+moveNumber,m,str);
							print("Hint: %s\n",str);
						}
					}
				}
			}
			
		}
		else if(!strcmp(command,"draw"));
			//print("Yet to be implemented\n");
		else if(!strcmp(command,"result"))
			stopSearch();
		else
			printError("unknown command",line);
		fflush(stdout);
		
	}
	DeleteMutex(actionLock);
}

//setboard using Forsyth-Edwards Notation
void setboard(char* string)
{
	//rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
	char piecePlacement[2048];
	char activeColor[2048];
	char castlingAvailability[2048];
	char enPassantSquare[2048];
	char halfMoveClock[2048];
	char fullMoveNumber[2048];
	int numread;
	unsigned char i,k;
	signed char j;

	assert(string!=NULL);
	numread=sscanf(string,"%s %s %s %s %s %s",piecePlacement,activeColor,castlingAvailability,enPassantSquare,halfMoveClock,fullMoveNumber);
	
	moveNumber=trueMoveNumber=0;
	*gamePosition=nullPos;
	if(numread<=0)
	{
		print("tellusererror Illegal position: No piece placement data\n");
		return;
	}
	if(numread<=1)
	{
		print("tellusererror Illegal position: No activeColor data\n");
		return;
	}
	
	//parse piece placement data
	assert(piecePlacement!=NULL);
	for(i=0,j=56,k=0;((unsigned int)i)<strlen(piecePlacement) && j>=0;i++)
	{
		switch(piecePlacement[i])
		{
		case 'P':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=P;
			gamePosition->Pieces[P]^=toBit(j+k);
			gamePosition->PiecesSide[WHITE]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'p':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=P;
			gamePosition->Pieces[P]^=toBit(j+k);
			gamePosition->PiecesSide[BLACK]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'N':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=N;
			gamePosition->Pieces[N]^=toBit(j+k);
			gamePosition->PiecesSide[WHITE]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'n':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=N;
			gamePosition->Pieces[N]^=toBit(j+k);
			gamePosition->PiecesSide[BLACK]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'B':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=B;
			gamePosition->Pieces[B]^=toBit(j+k);
			gamePosition->PiecesSide[WHITE]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'b':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=B;
			gamePosition->Pieces[B]^=toBit(j+k);
			gamePosition->PiecesSide[BLACK]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'R':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=R;
			gamePosition->Pieces[R]^=toBit(j+k);
			gamePosition->PiecesSide[WHITE]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'r':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=R;
			gamePosition->Pieces[R]^=toBit(j+k);
			gamePosition->PiecesSide[BLACK]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'Q':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=Q;
			gamePosition->Pieces[Q]^=toBit(j+k);
			gamePosition->PiecesSide[WHITE]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'q':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=Q;
			gamePosition->Pieces[Q]^=toBit(j+k);
			gamePosition->PiecesSide[BLACK]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			k++;
			break;
		case 'K':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=K;
			gamePosition->Pieces[K]^=toBit(j+k);
			gamePosition->PiecesSide[WHITE]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			gamePosition->KingPos[WHITE]=(j+k);
			k++;
			break;
		case 'k':
			if(k>=8)
			{
				print("tellusererror Illegal position: "
					"The inputs for rank %d exceeds 8 squares\n",ROW(j)+1);
				*gamePosition=nullPos;
				return;
			}
			gamePosition->PieceTypes[j+k]=K;
			gamePosition->Pieces[K]^=toBit(j+k);
			gamePosition->PiecesSide[BLACK]^=toBit(j+k);
			gamePosition->AllPieces^=toBit(j+k);
			gamePosition->KingPos[BLACK]=j+k;
			k++;
			break;
		case '\\':
		case '/':
			k=0;
			j-=8;
			break;
		case '1': k+=1; break;
		case '2': k+=2; break;
		case '3': k+=3; break;
		case '4': k+=4; break;
		case '5': k+=5; break;
		case '6': k+=6; break;
		case '7': k+=7; break;
		case '8': k+=8; break;
		default:
			print("tellusererror Illegal position: "
				"Unrecognized charecter %c in rank %d of the piece placement data\n",piecePlacement[i],ROW(j)+1);
			*gamePosition=nullPos;
			return;
		}
	}

	//parse active color
	assert(activeColor!=NULL);
	switch(*activeColor)
	{
	case 'b':
	case 'B':
	case 'd':
	case 'D':
		gamePosition->side=BLACK;
		gamePosition->xside=WHITE;
		break;
	default:
		gamePosition->side=WHITE;
		gamePosition->xside=BLACK;
		break;
	}

	gamePosition->castling=0;
	if(numread<=2)
		goto testLegality;

	//castling availability
	for(i=0;((unsigned int)(i))<strlen(castlingAvailability);i++)
	{
		switch(castlingAvailability[i])
		{
		case 'K':
			if(piecesWHITE(*gamePosition,R)&toBit(H1) && gamePosition->KingPos[WHITE]==E1)
				gamePosition->castling|=WK;
			break;
		case 'Q':
			if(piecesWHITE(*gamePosition,R)&toBit(A1) && gamePosition->KingPos[WHITE]==E1)
				gamePosition->castling|=WQ;
			break;
		case 'k':
			if(piecesBLACK(*gamePosition,R)&toBit(H8) && gamePosition->KingPos[BLACK]==E8)
				gamePosition->castling|=BK;
			break;
		case 'q':
			if(piecesBLACK(*gamePosition,R)&toBit(A8) && gamePosition->KingPos[BLACK]==E8)
				gamePosition->castling|=BQ;
			break;
		}
	}

	gamePosition->EP=0;
	if(numread<=3)
		goto testLegality;
	
	//enPassantSquare
	if(*enPassantSquare!='-')
	{
		int tempSq=coordToSquare(enPassantSquare);
		if(tempSq>=0)
			gamePosition->EP=(unsigned char)tempSq;
	}

	gamePosition->fifty=0;
	if(numread<=4)
		goto testLegality;

	//halfmove clock
	if(*halfMoveClock!='-')
	{
		int tempClock;
		if(sscanf(halfMoveClock,"%d",&tempClock)==1)
			gamePosition->fifty=(unsigned char)tempClock;
	}

	trueMoveNumber=2;
	if(numread<=5)
		goto testLegality;

	if(*fullMoveNumber!='-')
	{
		unsigned int tempFMN;
		if(sscanf(fullMoveNumber,"%u",&tempFMN)==1)
			trueMoveNumber=2*tempFMN+gamePosition->side;
	}

testLegality:
	//If there are incorrect number of kings for each side - the position is illegal
	if(!piecesWHITE(*gamePosition,K))
	{
		print("tellusererror Illegal position: "
				"No kings for white\n");
		*gamePosition=nullPos;
		return;
	}
	if(!piecesBLACK(*gamePosition,K))
	{
		print("tellusererror Illegal position: "
				"No kings for black\n");
		*gamePosition=nullPos;
		return;
	}
	if(piecesWHITE(*gamePosition,K)&(piecesWHITE(*gamePosition,K)-1))
	{
		print("tellusererror Illegal position: "
				"Too many kings for white\n");
		*gamePosition=nullPos;
		return;
	}
	if(piecesBLACK(*gamePosition,K)&(piecesBLACK(*gamePosition,K)-1))
	{
		print("tellusererror Illegal position: "
				"Too many kings for black\n");
		*gamePosition=nullPos;
		return;
	}

	//If the opponent side is in check - the position is illegal
	if(inCheck(*gamePosition,gamePosition->xside))
	{
		print("tellusererror Illegal position: "
				"The side not on move is in check\n");
		*gamePosition=nullPos;
		return;
	}

	//If there are pawns on the 8th or 1st rank - the position is illegal
	if(gamePosition->Pieces[P]&C64(0x00000000000000FF))
	{
		print("tellusererror Illegal position: "
				"There are pawns on the first rank\n");
		*gamePosition=nullPos;
		return;
	}
	if(gamePosition->Pieces[P]&C64(0xFF00000000000000))
	{
		print("tellusererror Illegal position: "
				"There are pawns on the eighth rank\n");
		*gamePosition=nullPos;
		return;
	}
	gamePosition->hashkey=rand64();
	gamePosition->recogsig=genrsig(*gamePosition);
}

//similar to setboard for older interfaces
//use setboard instead of edit when possible
void edit()
{
	bool color=WHITE;
	char line[2048];

	*gamePosition=gamePosition[moveNumber];
	moveNumber=0;

	while(fgets(line,2048,stdin))
	{
		line[strlen(line)]='\0';
		if(*line=='\n' || *line=='\0') continue;
		switch(*line)
		{
		case '#':
			{
				bool tempSide=gamePosition->side;
				*gamePosition=nullPos;
				gamePosition->side=tempSide;
				gamePosition->xside=!tempSide;
				trueMoveNumber=2;
				break;
			}
		case 'c':
			color=!color;
			break;
		case '.':
			goto finish;
		case 'x':
		case 'X':
			{
				int square=coordToSquare(line+1);
				if(square<0) break;
				gamePosition->AllPieces&=~toBit(square);
				gamePosition->PiecesSide[WHITE]&=~toBit(square);
				gamePosition->PiecesSide[BLACK]&=~toBit(square);
				gamePosition->Pieces[P]&=~toBit(square);
				gamePosition->Pieces[N]&=~toBit(square);
				gamePosition->Pieces[B]&=~toBit(square);
				gamePosition->Pieces[R]&=~toBit(square);
				gamePosition->Pieces[Q]&=~toBit(square);
				gamePosition->Pieces[K]&=~toBit(square);
				gamePosition->PieceTypes[square]=E;
				break;
			}
		case 'p': case 'P':
		case 'n': case 'N':
		case 'b': case 'B':
		case 'r': case 'R':
		case 'q': case 'Q':
			{
				unsigned char piece=charToPiece(*line);
				int square=coordToSquare(line+1);
				if(square<0)
				{
					printError("bad coordinate",line);
					break;
				}
				gamePosition->AllPieces^=toBit(square);
				gamePosition->PiecesSide[color]^=toBit(square);
				gamePosition->Pieces[piece]^=toBit(square);
				gamePosition->PieceTypes[square]=piece;
				break;
			}
		case 'k':
		case 'K':
			{
				int square=coordToSquare(line+1);
				if(square<0)
				{
					printError("bad coordinate",line);
					break;
				}
				gamePosition->AllPieces^=toBit(square);
				gamePosition->PiecesSide[color]^=toBit(square);
				gamePosition->Pieces[K]^=toBit(square);
				gamePosition->PieceTypes[square]=K;
				gamePosition->KingPos[color]=(unsigned char)(square);
				break;
			}
		default:
			printError("unknown command",line);
		}
	}
finish:
	gamePosition->EP=0;
	gamePosition->fifty=0;
	gamePosition->hashkey=U64EMPTY;
	gamePosition->castling=0;
	if(piecesWHITE(*gamePosition,R)&toBit(H1) && gamePosition->KingPos[WHITE]==E1)
		gamePosition->castling|=WK;
	if(piecesWHITE(*gamePosition,R)&toBit(A1) && gamePosition->KingPos[WHITE]==E1)
		gamePosition->castling|=WQ;
	if(piecesBLACK(*gamePosition,R)&toBit(H8) && gamePosition->KingPos[BLACK]==E8)
		gamePosition->castling|=BK;
	if(piecesBLACK(*gamePosition,R)&toBit(A8) && gamePosition->KingPos[BLACK]==E8)
		gamePosition->castling|=BQ;

	//If there are incorrect number of kings for each side - the position is illegal
	if(!piecesWHITE(*gamePosition,K))
	{
		print("tellusererror Illegal position: "
				"No kings for white\n");
		*gamePosition=nullPos;
		return;
	}
	if(!piecesBLACK(*gamePosition,K))
	{
		print("tellusererror Illegal position: "
				"No kings for black\n");
		*gamePosition=nullPos;
		return;
	}
	if(piecesWHITE(*gamePosition,K)&(piecesWHITE(*gamePosition,K)-1))
	{
		print("tellusererror Illegal position: "
				"Too many kings for white\n");
		*gamePosition=nullPos;
		return;
	}
	if(piecesBLACK(*gamePosition,K)&(piecesBLACK(*gamePosition,K)-1))
	{
		print("tellusererror Illegal position: "
				"Too many kings for black\n");
		*gamePosition=nullPos;
		return;
	}

	//If the opponent side is in check - the position is illegal
	if(inCheck(*gamePosition,gamePosition->xside))
	{
		print("tellusererror Illegal position: "
				"The side not on move is in check\n");
		*gamePosition=nullPos;
		return;
	}

	//If there are pawns on the 8th or 1st rank - the position is illegal
	if(gamePosition->Pieces[P]&C64(0x00000000000000FF))
	{
		print("tellusererror Illegal position: "
				"There are pawns on the first rank\n");
		*gamePosition=nullPos;
		return;
	}
	if(gamePosition->Pieces[P]&C64(0xFF00000000000000))
	{
		print("tellusererror Illegal position: "
				"There are pawns on the eighth rank\n");
		*gamePosition=nullPos;
		return;
	}
	gamePosition->hashkey=rand64();
	gamePosition->recogsig=genrsig(*gamePosition);
}


/*extracts the PV from the hash table and prints it*/
void printPV(board pos)
{
	U64 drawTable[MAX_SEARCH_DEPTH+1];
	hashEntry probe;
	unsigned int i=0;
	U64 charectersPrinted=0;

	if(pos.AllPieces==U64EMPTY) return;
	while(probeHash(pos.hashkey,&probe))
	{
		move m=extractMove(probe.data);
		
		#ifndef DEBUG_PV_PROBES
			if(m==NULLMOVE)
				return;
		#endif

		//Print the move
		if(i)
		{
			if(charectersPrinted+1>=MAX_CHARS_IN_PV) return;
			charectersPrinted++;
			print(" ");
		}
		{
			sanString str;
			moveToSAN(&pos,m,str);
			if(charectersPrinted+strlen(str)>=MAX_CHARS_IN_PV) return;
			charectersPrinted+=strlen(str);
			print("%s",str);
			#ifdef DEBUG_PV_PROBES
			{
				char ABE[3]={'=','>','<'};
				if(probe.score>=MATE_MIN)
					print("[%d%c+M%d]",probe.depth,ABE[extractFlag(probe.data)],MATE-probe.score);
				else if(probe.score<=-MATE_MIN)
					print("[%d%c-M%d]",probe.depth,ABE[extractFlag(probe.data)],MATE+probe.score);
				else
					print("[%d%c%d]",probe.depth,ABE[extractFlag(probe.data)],probe.score);
				
			}
			#endif
		}

		drawTable[i++]=pos.hashkey;
		/*printf("EP = %d\n",extractEP(m));*/
		makemove(&pos,m);
		//check end status
		{
			moveList ml;
			int j;
			genMoves(&pos,&ml);
			if(ml.moveCount==0)
			{
				
				if(inCheck(pos,pos.side))
				{
					if(charectersPrinted+1>=MAX_CHARS_IN_PV) return;
					charectersPrinted++;
					print("#"); //checkmate
				}return;
			}
			if(inCheck(pos,pos.side))
			{
				if(charectersPrinted+1>=MAX_CHARS_IN_PV) return;
				charectersPrinted++;
				print("+"); //check
			}
			if(pos.fifty==100) return; //fifty
			for(j=i-4;j>=0;j-=2) //repetition
				if(drawTable[j]==pos.hashkey) return;
		}
	}
}
void fen(const board* pos)
{
	int r;
	int c;
	for(r=56;r>=0;r-=8)
	{
		int emptySquares=0;
		for(c=0;c<8;c++)
		{
			if(toBit(r+c)&pos->PiecesSide[WHITE])
			{
				if(emptySquares)
				{
					print("%d",emptySquares);
					emptySquares=0;
				}
				print("%c",pieceToCharUC(pos->PieceTypes[r+c]));
			}
			else if(toBit(r+c)&pos->PiecesSide[BLACK])
			{
				if(emptySquares)
				{
					print("%d",emptySquares);
					emptySquares=0;
				}
				print("%c",pieceToCharLC(pos->PieceTypes[r+c]));
			}
			else
				emptySquares++;
		}
		if(emptySquares)
			print("%d",emptySquares);
		if(r)
			print("/");
	}
	if(pos->side) print(" b ");
	else print(" w ");
	if(!pos->castling) print("-");
	else
	{
		if(pos->castling&WK) print("K");
		if(pos->castling&WQ) print("Q");
		if(pos->castling&BK) print("k");
		if(pos->castling&BQ) print("q");
	}
	if(!pos->EP) print(" - ");
	else print(" %c%c ",'a'+COL(pos->EP),'1'+ROW(pos->EP));
	print("%d %d\n",pos->fifty,trueMoveNumber/2);
}

void logfen(const board* pos)
{
	int r;
	int c;
	for(r=56;r>=0;r-=8)
	{
		int emptySquares=0;
		for(c=0;c<8;c++)
		{
			if(toBit(r+c)&pos->PiecesSide[WHITE])
			{
				if(emptySquares)
				{
					printLogHTML(HTMLLOG_COLOR_BLUE,"%d",emptySquares);
					emptySquares=0;
				}
				printLogHTML(HTMLLOG_COLOR_BLUE,"%c",pieceToCharUC(pos->PieceTypes[r+c]));
			}
			else if(toBit(r+c)&pos->PiecesSide[BLACK])
			{
				if(emptySquares)
				{
					printLogHTML(HTMLLOG_COLOR_BLUE,"%d",emptySquares);
					emptySquares=0;
				}
				printLogHTML(HTMLLOG_COLOR_BLUE,"%c",pieceToCharLC(pos->PieceTypes[r+c]));
			}
			else
				emptySquares++;
		}
		if(emptySquares)
			printLogHTML(HTMLLOG_COLOR_BLUE,"%d",emptySquares);
		if(r)
			printLogHTML(HTMLLOG_COLOR_BLUE,"/");
	}
	if(pos->side) printLogHTML(HTMLLOG_COLOR_BLUE," b ");
	else printLogHTML(HTMLLOG_COLOR_BLUE," w ");
	if(!pos->castling) printLogHTML(HTMLLOG_COLOR_BLUE,"-");
	else
	{
		if(pos->castling&WK) printLogHTML(HTMLLOG_COLOR_BLUE,"K");
		if(pos->castling&WQ) printLogHTML(HTMLLOG_COLOR_BLUE,"Q");
		if(pos->castling&BK) printLogHTML(HTMLLOG_COLOR_BLUE,"k");
		if(pos->castling&BQ) printLogHTML(HTMLLOG_COLOR_BLUE,"q");
	}
	if(!pos->EP) printLogHTML(HTMLLOG_COLOR_BLUE," - ");
	else printLogHTML(HTMLLOG_COLOR_BLUE," %c%c ",'a'+COL(pos->EP),'1'+ROW(pos->EP));
	printLogHTML(HTMLLOG_COLOR_BLUE,"%d %d\n",pos->fifty,trueMoveNumber/2);
}

//identifying a result
int getResult()
{
	moveList ml;
	board pos = gamePosition[moveNumber];
	genMoves(&pos,&ml);
	if(ml.moveCount==0)
	{
		if(inCheck(pos,pos.side))
		{
			if(pos.xside==WHITE) return WHITE_MATES;
			else return BLACK_MATES;
		}
		return RESULT_STALEMATE;
	}
	else if(pos.fifty==100)
		return RESULT_FIFTY;
	else if(gameDecidableDraw(&pos))
		return RESULT_MATERIAL;
	//3 rep
	else
	{
		int i;
		unsigned char count=0;
		for(i=moveNumber-4;i>=0;i-=2)
		{
			if(gamePosition[i].hashkey==gamePosition[moveNumber].hashkey)
			{
				
				count++;
				if(count>=2)
					return RESULT_REPETITION;
			}
			
		}
	}
	return RESULT_NONE;
}

//TODO: Make this function use getResult()
//returns true if there was a mate/draw and prints a result
bool claimResult()
{
	//result claiming
	board pos;
	moveList ml;
	
	
	pos=gamePosition[moveNumber];
	
	
	genMoves(&pos,&ml);
	if(ml.moveCount==0)
	{
		if(inCheck(pos,pos.side))
		{
			if(pos.xside==WHITE) print("1-0 {White mates}\n");
			else print("0-1 {Black mates}\n");
		}
		else print("1/2-1/2 {Stalemate}\n");
		return true;
	}
	else if(pos.fifty==100)
	{
		print("1/2-1/2 {Draw by fifty moves}\n");
		return true;
	}
	else if(gameDecidableDraw(&pos))
	{
		print("1/2-1/2 {Insufficient material}\n");
		return true;
	}
	//3 rep
	else
	{
		int i;
		unsigned char count=0;
		for(i=moveNumber-4;i>=0;i-=2)
		{
			if(gamePosition[i].hashkey==gamePosition[moveNumber].hashkey)
			{
				
				count++;
				if(count>=2)
				{
					print("1/2-1/2 {Draw by repetition}\n");
					return true;
				}
			}
			
		}
	}
	return false;
}

void testEPD(char* filename)
{
	FILE* f;
	U64 nodes_tot[MAX_SEARCH_DEPTH];
	char line[4096];
	int i;
	int depth = 12;
	int position = 0;

	for(i=0;i<depth;i++) nodes_tot[i]=0;
	filename[strlen(filename)-1]='\0';
	f=fopen(filename,"rt");
	if(f==NULL)
	{
		print("Bad File %s\n-\n",filename);
		return;
	}

	infiniteTime();
	searchRunning = true;

	while(fgets(line,4096,f))
	{
		position++;
		resetHashTable();
		clearHistory();
		clearKillers();
		resetGameSpecificData();
		CC_Red();
		setboard(line);
		CC_Normal();
		if(gamePosition[moveNumber].AllPieces==U64EMPTY) break;
		setStartTime();
		nodes=0;
		{
			int iterationDepth;
			Thread t;
			for(iterationDepth=1;iterationDepth<=depth;iterationDepth++)
			{
				
				bwd* rootargs=malloc(sizeof(bwd));
				print("Position %d, Depth %d\n",position,iterationDepth);
				rootargs->b=gamePosition[moveNumber];
				rootargs->d=iterationDepth;
				rootargs->alpha=-INF;
				rootargs->beta=INF;
				#ifdef NEW_SEARCHID_EVERY_ITERATION
				newSearchID();
				#endif
				#ifdef DEBUG_PV
					resetDebugDat();
				#endif
				t=BeginThread(searchRoot,(void*)rootargs);
				Join(t);
				nodes_tot[iterationDepth]+=nodes;
			}
		}	
	}

	searchRunning = false;

	for(i=1;i<=depth;i++)
	{
		print("%d ",i);
		printU64(nodes_tot[i]);
		print("\n");
	}
}

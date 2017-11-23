/**
*Buzz Chess Engine
*xboard.h
*
*Header for the Winboard/Xboard protocol manager.
*
*Copyright (C) 2007 Pradu Kannan
**/

#ifndef _xboardh
#define _xboardh

#include "board.h"
#include "thread.h"

/*
*Build Configuration
*/

//#define DEBUG_PV_PROBES

#define MAX_GAME_MOVES 2048
#define MAX_CHARS_IN_NAMES 1024
#define MAX_CHARS_IN_PV 256

#define ICS_INFOTALK_NOTALK   0
#define ICS_INFOTALK_ADAPTIVE 1
#define ICS_INFOTALK_WHISPER  2
#define ICS_INFOTALK_KIBITZ   3

#define ICS_INFOTALK_DEFAULT ICS_INFOTALK_ADAPTIVE

/*
*Functions
*/

void print(const char *format , ...);
bool makeGameMove(move m);
void makeComputerGameMove(move m);
void printError(char* reason, char* line);
void xboard();
void setboard(char* string);
void edit();
void printPV(board pos);
void fen(const board* pos);
void logfen(const board* pos);
int getResult();
bool claimResult();
void testEPD(char* filename);

void resetGameSpecificData();

/*
*Data
*/

extern board gamePosition[MAX_GAME_MOVES];
extern unsigned int moveNumber;
extern unsigned int trueMoveNumber;
extern unsigned int compSide;
extern bool openedInConsole;
extern bool post;
extern bool ponder;
extern bool analyze;
extern bool opponentIsComputer;
extern char opponentName[MAX_CHARS_IN_NAMES];
extern char ics[MAX_CHARS_IN_NAMES];
extern unsigned int myRating;
extern unsigned int opponentRating;
extern unsigned int sd;
extern int icsInfoTalk;
extern U64 ponderHashKey;
extern move ponderMove;
extern Mutex actionLock;

#endif //_xboardh

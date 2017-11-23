/**
*Buzz Chess Engine
*pgn.h
*
*A generic PGN reader that reads a "clean" PGN file.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _pgnh
#define _pgnh

/*
*Dependencies
*/

#include "defs.h"
#include "board.h"
#include "movegen.h"

/*
*Definitions
*/

#define PGN_MAX_MOVES 2048
#define PGN_LINE_LENGTH 2048

typedef char PGNheader[PGN_LINE_LENGTH];

//A single PGN game
typedef struct _PGNgame
{
	//The six required PGN headers
	PGNheader event;
	PGNheader site;
    PGNheader date;
    PGNheader round;
    PGNheader white;
    PGNheader black;
	int whiteRating;
	int blackRating;
	unsigned int result; //see board.h for format
	move moves[PGN_MAX_MOVES];
	int nummoves;
}PGNgame;

/*
*Functions
*/

void resetPGNgame(PGNgame* const pgn);

//will read the next PGN game in a file into p, returns false on failure
bool getNextPGNGame(FILE* const f, PGNgame* const pgn);

#endif //_pgnh

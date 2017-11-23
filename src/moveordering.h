/**
*Buzz Chess Engine
*moveordering.h
*
*Header for functions related to move ordering.
*
*Copyright (C) 2007 Pradu Kannan
**/

#include "defs.h"
#include "board.h"
#include "movegen.h"

/*
*Build Configuration
*/

#define SEE_CAPTURE_ONLY
//#define USE_NULL_KILLER
#define TWO_KILLERS

/*
*Definitions
*/

#define MOVEORDERING_HASHMOVE   2147483647
#define MOVEORDERING_CAPTURE    2147400000
#define MOVEORDERING_NULLKILLER 2147300002
#define MOVEORDERING_KILLER     2147300001
#define MOVEORDERING_KILLER2    2147300000

#define MOVEORDERING_EQUALCAP   2147200004
#define MOVEORDERING_CHECK      2147200003
#define MOVEORDERING_PASSER     2147200002
#define MOVEORDERING_ATTACKWEAK 2147200001
#define MOVEORDERING_MAXHIST    2147200000

#define MOVEORDERING_CASTLING            2 /*Bonus*/

//Extra info added to the move when ordered
#define MO_PASSER 1
#define MO_CHECK  2
#define MO_CHECKEVASION 3
#define MO_CAPTURE 4
#define MO_BADSEE 5

/*
*Function Prototypes
*/

void orderMoves(const board* pos, moveList* ml, int dfr);
void orderQMoves(const board* pos, moveList* ml);
void sortMoveList(moveList* ml);
void storeKiller(const move m, const int dfr);
#ifdef USE_NULL_KILLER
void storeNullKiller(const move m, const int dfr);
#endif
void clearHistory();
void clearKillers();
void storeHistory(const move m, const int depth, const bool side);
int SEE(const board* pos, const move m);
void clearMoveOrderingData();

/**
*Buzz Chess Engine
*book.h
*
*Header file for book code. Includes a high performance PGN reader
*dedicated for book creation.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "defs.h"
#include "board.h"
#include "movegen.h"

/*
*Configuration
*/

//Move choosing metrics (choose only one of these)
#define BOOK_METRIC_NORM    /*use the result score and number of games as the metric*/

/*
*Defenitions
*/

#define DEFAULT_BOOK_PATH "default.book"

//book position - this is the one that goes in the actual book
//              - for now this is the same as bookPosGen
typedef struct _bookPos
{
	U64 hashkey;
	U64 numgames;
	U64 numwins; //for opposite side
	U64 numdraws;
}bookPos;

//book position used for generation and razoring
typedef struct _bookPosGen
{
	U64 hashkey;
	U64 numgames;
	U64 numwins; //for opposite side
	U64 numdraws;
}bookPosGen;

/*
*Functions
*/

move getBookMove(const board* pos, char* info);
void makebook(int argc, char* argv[]);
bool getPGNBookPosition(bookPosGen* pos);

//Red black trees for book generation
void rb_insert(bookPosGen* bookPos);
bool rb_getleast(bookPosGen* bkpos);

/*
*Global Data
*/

char bookPath[2048];

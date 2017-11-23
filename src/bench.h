/**
*Buzz Chess Engine
*bench.h
*
*Runs a test for a built in set of positions given a test set.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _benchh
#define _benchh

#include "defs.h"
#include "board.h"

#define BENCH_HASHKEY C64(0xF0C1A89D3B2F57E4)

typedef struct _benchPosition
{
	char fen[128];
	char depth;
}benchPosition;

void bench();

#endif //_benchh

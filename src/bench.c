/**
*Buzz Chess Engine
*bench.c
*
*Runs a test for a built in set of positions.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "defs.h"
#include "bench.h"
#include "xboard.h"
#include "movegen.h"
#include "timemanager.h"
#include "search.h"
#include "moveordering.h"

benchPosition suite[] =
{
	/***************************** Crafty's Bench Test Set *****************************/
	
	//Bratko-Kopec 2, 4, 8, 12, 22 and 23
	{"3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w",                12 },
	{"rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w",        12 },
	{"4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w",                   15 },
	{"r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b",                12 },
	{"2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b", 13 },
	{"r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b",         12 }
};

void bench()
{
	time_ms time_used = 0;
	unsigned int i;
	if(searchRunning) stopSearch();
	post = false;
	compSide = NOSIDE;
	for(i=0;i<sizeof(suite)/sizeof(benchPosition);i++)
	{
		setboard(suite[i].fen);
		gamePosition[moveNumber].hashkey = BENCH_HASHKEY;
		clearMoveOrderingData();
		{
			time_ms t0;
			bwd* arg=(bwd*)malloc(sizeof(bwd));
			arg->d=suite[i].depth-3;
			arg->b=gamePosition[moveNumber];
			moveNumberDrawDetection=moveNumber;
			stopSearch();
			infiniteTime();
			t0 = getms();
			searchThread=BeginThread(searchStart,(void*)arg);
			Join(searchThread);
			time_used += getms()-t0;
			print(".");
		}
	}
	print("\nTime: %u ms\n",time_used);
	post = true;
	*gamePosition=startPos;
	moveNumber=0;
	sd=0;
	trueMoveNumber=2;
	compSide=BLACK;
	resetGameSpecificData();
}

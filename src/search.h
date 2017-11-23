/**
*Buzz Chess Engine
*search.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _searchh
#define _searchh

/*
*Dependancies
*/

#include "thread.h"
#include "defs.h"
#include "board.h"
#include "bitinstructions.h"

/*
*Build Configuration
*/

//debug Eval
//#define DO_SYMMETRY_CHECK
//#define PRINT_MAXPLY_REACHED
//#define PRINT_MOVENODES
//#define PRINT_MOVESCORES
//#define TRACE_SEARCH 0 /*millisecond pause*/
#ifdef TRACE_SEARCH
	#define TRACE_SCORES
#endif
//#define DEBUG_PV
//#define DEBUG_DEPTH_LIMIT 2

#define NTS
#define NTS_UNSTABLE /*NTS assumes an unstable search*/

#define NULLMOVE_PRUNING
#ifdef NULLMOVE_PRUNING
	//Reduction amount - pick one
	#define NULLMOVE_ADAPTIVE_R
	//#define NULLMOVE_CONST_R 3

	//#define NULLMOVE_NOREPCHECK
	#define NULLMOVE_NO_MULTI_NULLMOVE
	#define NULLMOVE_NO_BETAMATE
	//#define NULLMOVE_NO_ALLNODE
	//#define NULLMOVE_NO_OPENNODE
	//#define NULLMOVE_ENSURE_OPPMOVE
	//#define NULLMOVE_NO_QS_ABOVE_BETA
	#ifdef NULLMOVE_ENSURE_OPPMOVE
		#define NULLMOVE_DEPTH_LEFT_LIMIT 2
	#endif
	#ifndef NULLMOVE_DEPTH_LEFT_LIMIT
		#define NULLMOVE_DEPTH_LEFT_LIMIT 2
	#endif
#endif
#define PIECES_NULLMOVE_OFF 1

#define MATE_DISTANCE_PRUNING

//#define FUTILITY_PRUNING
#define FUTILITY_MARGIN 140

/*#define LMR
#define LMR_MOVENUMBER 2 /*Greater than this number*/
#define LMR_DEPTHLEFT  2 /*Must have greater than this depth left*/
#define LMR_NOEXTEND
#define LMR_NOOPENNODE

//#define RECAPTURE_EXTENSIONS
//#define RECAPTURE_OPEN_NODE_ONLY
//#define PASSER_EXTENSION
#ifndef PASSER_EXTENSION
	#define PAWN_TO_7TH_EXTENSION
#endif
#define SINGULAR_REPLY_EXTENSIONS
#ifdef SINGULAR_REPLY_EXTENSIONS
	#define SINGULAR_REPLY_EXTEND_ALWAYS
#endif
//#define MATE_THREAT_EXTENSION
#define MATE_THREAT_EXTENSION_DEPTH_LIMIT MAX_SEARCH_DEPTH
#define MATE_THREAT_EXTENSION_DFR_LIMIT (iterationDepth*2)
//#define KING_PAWN_ENDING_EXTENSION 3

//the following must work in conjunction their respective parts in moveordering.h
//#define USE_NULL_KILLER

#define USE_HASH
//#define CLEAR_HASH_EVERY_SEARCH
//#define UPDATE_BOUNDS
//#define NEW_SEARCHID_EVERY_ITERATION

#define QSEARCH_CHECKS
#define COUNT_QSEARCH_NODES

//#define PERFT_HASH_TEST

/*
*Defenitions
*/

//A board with a depth and bounds -- useful for passing data to initialize search
typedef struct _bwd
{
	board b;
	int d;
	int alpha;
	int beta;
}bwd;

//Node types
/*
OPEN Nodes
----------
The root node is an OPEN node.
All children of OPEN nodes are an OPEN nodes.
An OPEN node changes to an ALL node when alpha is raised.

ALL Nodes
---------
All children of ALL nodes are CUT nodes.
An ALL node changes an OPEN node when the scout search fails high.
(will define "scout search" later)

CUT Nodes
---------
The first child of a CUT node is an ALL node.
A CUT node changes to an ALL node when the first move fails low.

Notes:
======
Parallelism isn't good on OPEN nodes because there is too much search
overhead for the current window.  It is very likely that the window will
be smaller at a later time and that parallel search should wait until the
window is smaller. Parallelism isn't good on CUT nodes because there is
too much search overhead because one of the moves will cutoff the search.
Parallelism is possible only on ALL nodes.
*/

#define OPEN_NODE 0
#define CUT_NODE  1
#define ALL_NODE  2

/*
*Macros
*/

#ifdef max
	#undef max
#endif
static INLINE int max(int a, int b)
{
	if(a>b) return a;
	return b;
}

#ifdef min
	#undef min
#endif
static INLINE int min(int a, int b)
{
	if(a<b) return a;
	return b;
}

/*
*Function Prototypes
*/

void initSearch();
void stopSearch();
THREAD_PROC_RET searchStart(void* argbwd);
THREAD_PROC_RET searchRoot(void* bwdargs);
int search(board pos, move m, int alpha, int beta, int depth, int dfr, U64* drawTable, int p_nodeType);

int qsearchRoot(board* pos, int alpha, int beta, int dfr);
int qsearch(board pos, move m, int alpha, int beta, int dfr);

int adaptiveR(const board* pos, const int depth);
int childNodeType(const int previous_type);

U64 perft(board pos, move m, int d);
DTHREAD_PROC_RET perftStart(void* args); //bwd
void divide(board pos, int d);

/*
*Global Data
*/

extern volatile U64 nodes;
extern Thread searchThread;
extern volatile bool searchRunning; //initially false
extern volatile int moveNumberDrawDetection;   //for correct draw detection when pondering
extern volatile bool stoppingSearch;

#ifdef TRACE_SEARCH
extern bool TRACE_SEARCH_FLAG;
#endif

#endif //_searchh

/**
*Buzz Chess Engine
*moveordering.c
*
*Orders move to help alphabeta searching.
*
*Copyright (C) 2007 Pradu Kannan.
**/
#include "xboard.h"

#include "search.h"
#include "defs.h"
#include "moveordering.h"
#include "bitinstructions.h"
#include "hash.h"
#include "eval.h"
#include <stdlib.h>

move killer[MAX_SEARCH_DEPTH+1];
#ifdef TWO_KILLERS
	move killer2[MAX_SEARCH_DEPTH+1];
#endif
#ifdef USE_NULL_KILLER
	move null_killer[MAX_SEARCH_DEPTH+1];
#endif
//[piece][side][to]
int history[NUMPIECETYPES][2][64];

/**************************
*Private Functions and Data
**************************/

/*#ifdef max
	#undef max
#endif
INLINE int max(int a, int b)
{
	if(a>b) return a;
	return b;
}

#ifdef min
	#undef min
#endif
INLINE int min(int a, int b)
{
	if(a<b) return a;
	return b;
}*/

//Only ordering by hash moves for now
void orderMoves(const board* pos, moveList* ml, int dfr)
{
	//score hash move
	unsigned int i;
	move hashMove;
	U64 passedPawns;
	//possible attack squares to the opponent king
	U64 KBAttack = Bmoves(pos->KingPos[pos->xside],pos->AllPieces);
	U64 KRAttack = Bmoves(pos->KingPos[pos->xside],pos->AllPieces);
	U64 KNAttack = Nmoves(pos->KingPos[pos->xside]);
	U64 KPAttack = Pcaps(pos->KingPos[pos->xside],pos->xside);
	hashEntry probe;
	bool hashHit = probeHash(pos->hashkey,&probe);

	if(hashHit)
		hashMove=extractMove(probe.data);
	else
		hashMove=NULLMOVE;

	//Generate Passers
	{
		U64 Pawns[2]={piecesWHITE(*pos,P),piecesBLACK(*pos,P)};
		
		if(pos->side==WHITE)
		{
			U64 blockers = Pawns[BLACK]|
				((Pawns[BLACK]&C64(0x7F7F7F7F7F7F7F7F))>>7)|
				((Pawns[BLACK]&C64(0xFEFEFEFEFEFEFEFE))>>9);
			U64 doubled = fillDown(Pawns[WHITE]);
			passedPawns = Pawns[WHITE] & ~(doubled | fillDown2(blockers));
		}
		else
		{
			U64 blockers = Pawns[WHITE]|
				((Pawns[WHITE]&C64(0x7F7F7F7F7F7F7F7F))<<9)|
				((Pawns[WHITE]&C64(0xFEFEFEFEFEFEFEFE))<<7);
			U64 doubled = fillUp(Pawns[BLACK]);
			passedPawns = Pawns[BLACK] & ~(doubled | fillUp2(blockers));
		}
	}
	
	for(i=0;i<ml->moveCount;i++)
	{
		//Extra flags
		ml->moves[i].score = 0;

		//Hash Move
		if(ml->moves[i].m==hashMove) //ml->moves[i].m should already be stripped
		{
			if(toBit(extractFrom(ml->moves[i].m)) & passedPawns) ml->moves[i].m|=encodeExtra(MO_PASSER);		
			ml->moves[i].score=MOVEORDERING_HASHMOVE;
			continue;
		}

		//SEE Score
		
		if(extractPiece(ml->moves[i].m)==E) //castling
			ml->moves[i].score=MOVEORDERING_CASTLING;
		else if(extractPiece(ml->moves[i].m)==K)
		{
			if(extractCapture(ml->moves[i].m)!=E)
			{
				ml->moves[i].score=MOVEORDERING_CAPTURE+pieceValue[extractCapture(ml->moves[i].m)];
				continue;
			}
			ml->moves[i].score=0;
		}
		#ifdef SEE_CAPTURE_ONLY
		else if(extractCapture(ml->moves[i].m)==E)
			ml->moves[i].score=0;
		#endif
		else
		{
			ml->moves[i].score=SEE(pos,ml->moves[i].m);
			if(ml->moves[i].score)
			{
				if(ml->moves[i].score>0)
					ml->moves[i].score+=MOVEORDERING_CAPTURE;
				else
					ml->moves[i].score-=MOVEORDERING_CAPTURE;
				continue;
			}
		}


		//Killers
		#ifdef USE_NULL_KILLER
			if(ml->moves[i].m==null_killer[dfr])
			{
				if(toBit(extractFrom(ml->moves[i].m)) & passedPawns) ml->moves[i].m|=encodeExtra(MO_PASSER);
				ml->moves[i].score=MOVEORDERING_NULLKILLER;
				continue;
			}
		#endif
		if(ml->moves[i].m==killer[dfr])
		{
			if(toBit(extractFrom(ml->moves[i].m)) & passedPawns) ml->moves[i].m|=encodeExtra(MO_PASSER);
			ml->moves[i].score=MOVEORDERING_KILLER;
			continue;
		}
		#ifdef TWO_KILLERS
			if(ml->moves[i].m==killer2[dfr])
			{
				if(toBit(extractFrom(ml->moves[i].m)) & passedPawns) ml->moves[i].m|=encodeExtra(MO_PASSER);
				ml->moves[i].score=MOVEORDERING_KILLER2;
				continue;
			}
		#endif
		
		//Passed Pawn Pushes
		if(toBit(extractFrom(ml->moves[i].m)) & passedPawns)
		{
			ml->moves[i].m|=encodeExtra(MO_PASSER);
			ml->moves[i].score=MOVEORDERING_PASSER;
			continue;
		}
		
		//Checks
		switch(extractPiece(ml->moves[i].m))
		{
		case P:
			if(toBit(extractTo(ml->moves[i].m)) & KPAttack)
			{
				ml->moves[i].score=MOVEORDERING_CHECK;
				continue;
			}break;
		case N:
			if(toBit(extractTo(ml->moves[i].m)) & KNAttack)
			{
				ml->moves[i].score=MOVEORDERING_CHECK;
				continue;
			}break;
		case B:
			if(toBit(extractTo(ml->moves[i].m)) & KBAttack)
			{
				ml->moves[i].score=MOVEORDERING_CHECK;
				continue;
			}break;
		case R:
			if(toBit(extractTo(ml->moves[i].m)) & KRAttack)
			{
				ml->moves[i].score=MOVEORDERING_CHECK;
				continue;
			}break;
		case Q:
			if(toBit(extractTo(ml->moves[i].m)) & (KBAttack|KRAttack))
			{
				ml->moves[i].score=MOVEORDERING_CHECK;
				continue;
			}break;
		}

		//Equal Captures
		if(extractCapture(ml->moves[i].m)!=E)
		{
			ml->moves[i].score=MOVEORDERING_EQUALCAP;
			continue;
		}

		//Mobility Addition
		switch(extractPiece(ml->moves[i].m))
		{
		case N:
		ml->moves[i].score=(popcnt(Nmoves(extractTo(ml->moves[i].m)))
							-popcnt(Nmoves(extractFrom(ml->moves[i].m))));
		case B:
			ml->moves[i].score=(popcnt(Bmoves(extractTo(ml->moves[i].m),pos->AllPieces))
								-popcnt(Bmoves(extractFrom(ml->moves[i].m),pos->AllPieces)));
			break;
		case R:
			ml->moves[i].score=(popcnt(Rmoves(extractTo(ml->moves[i].m),pos->AllPieces))
								-popcnt(Rmoves(extractFrom(ml->moves[i].m),pos->AllPieces)));
			break;
		case Q:
			ml->moves[i].score=(popcnt(Qmoves(extractTo(ml->moves[i].m),pos->AllPieces))
								-popcnt(Qmoves(extractFrom(ml->moves[i].m),pos->AllPieces)));
		default:
			ml->moves[i].score=0;
			break;
		}

		//History Heuristic
		ml->moves[i].score+=history[extractPiece(ml->moves[i].m)][pos->side][extractTo(ml->moves[i].m)];
		
		/*if(dfr<=4)
			ml->moves[i].score = -qsearch(*pos, ml->moves[i].m, -INF, INF, dfr);
		else ml->moves[i].score+=history[extractPiece(ml->moves[i].m)][pos->side][extractTo(ml->moves[i].m)];
		*/
	}
	sortMoveList(ml);
}

void orderQMoves(const board* pos, moveList* ml)
{
	unsigned int i;
	for(i=0;i<ml->moveCount;i++)
		ml->moves[i].score=SEE(pos,ml->moves[i].m);
	sortMoveList(ml);
}

void sortMoveList(moveList* ml)
{
	int i;
	for(i=(ml->moveCount-1);i>0;i--)
	{
		int j;
		for(j=1;j<=i;j++)
			if(ml->moves[j-1].score < ml->moves[j].score)
			{
				smove temp = ml->moves[j-1];
				ml->moves[j-1] = ml->moves[j];
				ml->moves[j] = temp;
			}
	}
}

void storeKiller(const move m, const int dfr)
{
	if(extractCapture(m)==E)
	{
		#ifndef TWO_KILLERS
			killer[dfr]=m;
		#else
			if(killer[dfr]!=m)
			{
				killer2[dfr]=killer[dfr];
				killer[dfr]=m;
			}
		#endif
	}
}

#ifdef USE_NULL_KILLER
void storeNullKiller(const move m, const int dfr)
{
	if(extractCapture(m)==E)
		null_killer[dfr]=m;
}
#endif

void clearHistory()
{
	int i,j,k;
	for(i=0;i<6;i++)
		for(j=0;j<2;j++)
			for(k=0;k<64;k++)
				history[i][j][k]=0;
}

//Thanks to Andrew Fan for this hint
void clearKillers()
{
	int i;
	for(i=0;i<MAX_SEARCH_DEPTH+1;i++)
	{
		killer[i]=NULLMOVE;
		#ifdef TWO_KILLERS
			killer2[i]=NULLMOVE;
		#endif
	}
}

void storeHistory(const move m, const int depth, const bool side)
{
	history[extractPiece(m)][side][extractTo(m)]+=depth*depth;
	if(history[extractPiece(m)][side][extractTo(m)]>MOVEORDERING_MAXHIST)
		history[extractPiece(m)][side][extractTo(m)]=MOVEORDERING_MAXHIST;
}


//full static exchange evaluvation
int SEE(const board* pos, const move m)
{
	//The variables are set to the inital state where the move in question has been played
	int SEEscore=pieceValue[extractCapture(m)]; //The score to be returned
	int SEEmax, SEEmin; //Maximum and minimum SEE scores (alphbeta approach)
	int target=extractTo(m); //The square that we are doing a SEE on
	int currentPieceValue=pieceValue[extractPiece(m)]; //Value of the piece currently on the target square
	U64 used=toBit(extractFrom(m)); //Used attackers
	//Battery attacks will be calculated instead of regular attacks
	//Piece order isn't a problem (except when dealing with Queens)
	U64 occupancy=(pos->AllPieces^used) &
		 ~(
			(pos->Pieces[B]&BmovesNoOcc(target))|(pos->Pieces[R]&RmovesNoOcc(target))|
			( (piecesBLACK(*pos,P)&Pcaps(target,WHITE)) |  (piecesWHITE(*pos,P)&Pcaps(target,BLACK)) )
		  ); 

	U64 attackers; //attackers of each piece type from both sides
	U64 attackersSide; //attackers for each piece type for a particular side
	U64 attackersPiece; //attackers for a particular color and a particular type

	//handle enpassant and promotion
	if(currentPieceValue==Pval)
	{
		if(extractEP(m))
		{
			used|=toBit(extractEP(m));
			occupancy^=toBit(extractEP(m));
			SEEscore=Pval;
		}
		else if(extractPromotion(m)!=E)
		{
			currentPieceValue=pieceValue[extractPromotion(m)];
			SEEscore+=currentPieceValue-Pval;
		}
	}

	//these are the bounds, we will be doing an alphabeta-like search in SEE
	SEEmax=SEEscore; //upperbound
	SEEmin=-INF; //lowerbound

	//Generate attackers
	attackers=attacksToOcc(*pos,target,occupancy)&~used;
	
	

	//Loop Through Opponent Captures
	while(attackersSide=attackers&pos->PiecesSide[pos->xside])
	{
		assert(!(attackers&used));
		SEEscore-=currentPieceValue;
		if(SEEscore>=SEEmax) return SEEmax;
		if(SEEscore>SEEmin) SEEmin=SEEscore;

		if(attackersPiece=attackersSide&pos->Pieces[P]) //Pawn
		{
			attackers^=LSB(attackersPiece);
			used|=LSB(attackersPiece); 
			currentPieceValue=Pval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[N]) //Knight
		{
			attackers^=LSB(attackersPiece);
			currentPieceValue=Nval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[B]) //Bishop
		{
			attackers^=LSB(attackersPiece);
			used|=LSB(attackersPiece);
			currentPieceValue=Bval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[R]) //Rook
		{
			attackers^=LSB(attackersPiece);
			used|=LSB(attackersPiece);
			currentPieceValue=Rval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[Q]) //Queen
		{
			used|=LSB(attackersPiece);
			occupancy^=LSB(attackersPiece);
			attackers=attacksBQRToOcc(*pos,target,occupancy);
			attackers&=~used;
			currentPieceValue=Qval;
		}
		else //King
		{
			attackers^=toBit(pos->KingPos[pos->xside]);
			currentPieceValue=Kval;
		}
		//Loop Through My Captures
		if(!(attackersSide=attackers&pos->PiecesSide[pos->side])) break;
		assert(!(attackers&used));

		SEEscore+=currentPieceValue;
		if(SEEscore<=SEEmin) return SEEmin;
		if(SEEscore<SEEmax) SEEmax=SEEscore;

		if(attackersPiece=attackersSide&pos->Pieces[P]) //Pawn
		{
			attackers^=LSB(attackersPiece);
			used|=LSB(attackersPiece); 
			currentPieceValue=Pval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[N]) //Knight
		{
			attackers^=LSB(attackersPiece);
			currentPieceValue=Nval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[B]) //Bishop
		{
			attackers^=LSB(attackersPiece);
			used|=LSB(attackersPiece);
			currentPieceValue=Bval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[R]) //Rook
		{
			attackers^=LSB(attackersPiece);
			used|=LSB(attackersPiece);
			currentPieceValue=Rval;
		}
		else if(attackersPiece=attackersSide&pos->Pieces[Q]) //Queen
		{
			used|=LSB(attackersPiece);
			occupancy^=LSB(attackersPiece);
			attackers|=attacksBQRToOcc(*pos,target,occupancy);
			attackers&=~used;
			currentPieceValue=Qval;
		}
		else //King
		{
			attackers^=toBit(pos->KingPos[pos->side]);
			currentPieceValue=Kval;
		}
	}
	if(SEEscore<SEEmin) return SEEmin;
	if(SEEscore>SEEmax) return SEEmax;
	return SEEscore;
}

void clearMoveOrderingData()
{
	clearHistory();
	clearKillers();
	resetHashTable();
}

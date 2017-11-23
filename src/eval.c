/**
*Buzz Chess Engine
*eval.c
*
*I would like to note that in this eval I used, although not exactly,
*many of the principles suggested by the correspondence chess champion
*Hans Berliner.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/
#ifdef ANDRES_EVAL
	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
#endif
#include "eval.h"
#include <stdio.h>
#include <stdlib.h>
#include "thread.h"
#include "movegen.h"
#include "bitinstructions.h"
#include "recog.h"

#ifdef DEBUG_EVAL
#define DEBUG_EVAL_PRINT(TEXT) \
{ \
	int debug_op[2] = {op[WHITE]-debugop[WHITE], op[BLACK]-debugop[BLACK]}; \
	int debug_eg[2] = {eg[WHITE]-debugeg[WHITE], eg[BLACK]-debugeg[BLACK]}; \
	int debug_compiled[2] = { \
	SCALE_SCORE(debug_op[WHITE],debug_eg[WHITE],pieceMaterial[BLACK]), \
	SCALE_SCORE(debug_op[BLACK],debug_eg[BLACK],pieceMaterial[WHITE]) \
	}; \
	printf(TEXT " = %d\n",score-debugscore+debug_compiled[pos->side]-debug_compiled[pos->xside]); \
	printf("       Opening = [%4d,%4d] = %4d\n",debug_op[WHITE],debug_op[BLACK],debug_op[pos->side]-debug_op[pos->xside]); \
	printf("       Endgame = [%4d,%4d] = %4d\n",debug_eg[WHITE],debug_eg[BLACK],debug_eg[pos->side]-debug_eg[pos->xside]); \
	debugop[WHITE]=op[WHITE]; \
	debugop[BLACK]=op[BLACK]; \
	debugeg[WHITE]=eg[WHITE]; \
	debugeg[BLACK]=eg[BLACK]; \
	debugscore=score; \
}
#else
#define DEBUG_EVAL_PRINT(TEXT) /*Nothing*/
#endif

//Used for SEE and other search related stuff
int pieceValue[7]={Pval,Nval,Bval,Rval,Qval,Kval,0};

/*Ideas

Safe-Mobility
PST-Pawn Eval

Knights are easily chased away with pawn moves. Therefore it is important
to spot "holes" in the enemy position where a knight cannot be attacked,
because the pawns have already moved past. Once such a hole is identified,
a knight should be maneuvered to that location. An unchallengeable knight
on the fifth row is a strong asset, and a supported knight on the sixth
row usually decides the game.

An endgame in which the parties have bishops that live on different colours
is almost always drawn, even if one side is two pawns ahead.
simply reduce the material score
more when less material on board

Queens are the most powerful pieces in a chess game. Queens are extremely
versatile, and can threaten many pieces at once. For this reason, checkmates
involving the queen are much easier to achieve than those without her. Because
the loss of a queen usually results in the loss of the game, it is generally
wise to wait to develop a queen until after the knights and bishops have been
developed.

In the endgame, the king becomes a strong piece. With reduced material, mate
is not an immediate concern anymore, and the king should be moved towards the
center of the board.

Initiative Eval

Should mobility weight be the same for every piece?

An Isolated pawn is a pawn that has no friendly pawns on adjacent files.
Such a pawn cannot be defended by another pawn and therefore needs to be
defended by pieces instead. Nonetheless an isolated pawn in the middlegame
isn't always considered a handicap, as the player with the isolated pawn has
open files for the rooks on either side of the pawn. Also, in typical
isolated pawn positions, the isolated pawn can be used to break open the
centre with a well timed advance. However, once the endgame is reached, an
isolated pawn is more likely to become a serious weakness.

If you have a passed pawn, you may want to trade off pieces to protect it.
*/

//Material Piece Square Tables

//Ratios from Hans Berliner's system
int P_op_PST[64] = {
	 P_op*90/100,  P_op*95/100, P_op*105/100, P_op*110/100, P_op*110/100, P_op*105/100,   P_op*95/100,  P_op*90/100,
	 P_op*90/100,  P_op*95/100, P_op*105/100, P_op*110/100, P_op*110/100, P_op*105/100,   P_op*95/100,  P_op*90/100,
	 P_op*90/100,  P_op*95/100, P_op*105/100, P_op*115/100, P_op*115/100, P_op*105/100,   P_op*95/100,  P_op*90/100,
	 P_op*90/100,  P_op*95/100, P_op*110/100, P_op*120/100, P_op*120/100, P_op*110/100,   P_op*95/100,  P_op*90/100,
	 P_op*90/100,  P_op*95/100, P_op*105/100, P_op*115/100, P_op*115/100, P_op*105/100,   P_op*95/100,  P_op*90/100,
	 P_op*90/100,  P_op*95/100, P_op*105/100, P_op*110/100, P_op*110/100, P_op*105/100,   P_op*95/100,  P_op*90/100,
	 P_op*90/100,  P_op*95/100, P_op*105/100, P_op*110/100, P_op*110/100, P_op*105/100,   P_op*95/100,  P_op*90/100,
	 P_op*90/100,  P_op*95/100, P_op*105/100, P_op*110/100, P_op*110/100, P_op*105/100,   P_op*95/100,  P_op*90/100,
};

int P_eg_PST[64] = {
	P_eg*120/100, P_eg*105/100,  P_eg*95/100,  P_eg*90/100,  P_eg*90/100,  P_eg*95/100,  P_eg*105/100, P_eg*120/100,
	P_eg*120/100, P_eg*105/100,  P_eg*95/100,  P_eg*90/100,  P_eg*90/100,  P_eg*95/100,  P_eg*105/100, P_eg*120/100,
	P_eg*120/100, P_eg*105/100,  P_eg*95/100,  P_eg*90/100,  P_eg*90/100,  P_eg*95/100,  P_eg*105/100, P_eg*120/100,
	P_eg*125/100, P_eg*110/100, P_eg*100/100,  P_eg*95/100,  P_eg*95/100, P_eg*100/100,  P_eg*110/100, P_eg*125/100,
	P_eg*133/100, P_eg*117/100, P_eg*107/100, P_eg*100/100, P_eg*100/100, P_eg*107/100,  P_eg*117/100, P_eg*133/100,
	P_eg*145/100, P_eg*129/100, P_eg*116/100, P_eg*105/100, P_eg*105/100, P_eg*116/100,  P_eg*129/100, P_eg*145/100,
	P_eg*145/100, P_eg*129/100, P_eg*116/100, P_eg*105/100, P_eg*105/100, P_eg*116/100,  P_eg*129/100, P_eg*145/100,
	P_eg*145/100, P_eg*129/100, P_eg*116/100, P_eg*105/100, P_eg*105/100, P_eg*116/100,  P_eg*129/100, P_eg*145/100
};

//piece square table for pieces - similar to Toga's
int K_op_PST[64] = {
	 4*K_op_PSTW,  4*K_op_PSTW,  3*K_op_PSTW,  1*K_op_PSTW,  1*K_op_PSTW,  3*K_op_PSTW,  4*K_op_PSTW,  4*K_op_PSTW,
	 3*K_op_PSTW,  4*K_op_PSTW,  2*K_op_PSTW,            0,            0,  2*K_op_PSTW,  4*K_op_PSTW,  3*K_op_PSTW,
	 1*K_op_PSTW,  2*K_op_PSTW,            0, -2*K_op_PSTW, -2*K_op_PSTW,            0,  2*K_op_PSTW,  1*K_op_PSTW,
               0,  1*K_op_PSTW, -1*K_op_PSTW, -3*K_op_PSTW, -3*K_op_PSTW, -1*K_op_PSTW,  1*K_op_PSTW,            0,
	-1*K_op_PSTW,            0, -2*K_op_PSTW, -4*K_op_PSTW, -4*K_op_PSTW, -2*K_op_PSTW,            0, -1*K_op_PSTW,
	-2*K_op_PSTW, -1*K_op_PSTW, -3*K_op_PSTW, -5*K_op_PSTW, -5*K_op_PSTW, -3*K_op_PSTW, -1*K_op_PSTW, -2*K_op_PSTW,
	-3*K_op_PSTW, -2*K_op_PSTW, -4*K_op_PSTW, -6*K_op_PSTW, -6*K_op_PSTW, -4*K_op_PSTW, -2*K_op_PSTW, -3*K_op_PSTW,
	-4*K_op_PSTW, -3*K_op_PSTW, -5*K_op_PSTW, -7*K_op_PSTW, -7*K_op_PSTW, -5*K_op_PSTW, -3*K_op_PSTW, -4*K_op_PSTW
};

int K_eg_PST[64] = {
	-K_eg_PSTW*6, -K_eg_PSTW*4, -K_eg_PSTW*3, -K_eg_PSTW*2, -K_eg_PSTW*2, -K_eg_PSTW*3, -K_eg_PSTW*4, -K_eg_PSTW*6,
	-K_eg_PSTW*4, -K_eg_PSTW*2, -K_eg_PSTW*1,            0,            0, -K_eg_PSTW*1, -K_eg_PSTW*2, -K_eg_PSTW*4,
	-K_eg_PSTW*3, -K_eg_PSTW*1,            0,  K_eg_PSTW*1,  K_eg_PSTW*1,            0, -K_eg_PSTW*1, -K_eg_PSTW*3,
	-K_eg_PSTW*2,            0,  K_eg_PSTW*1,  K_eg_PSTW*2,  K_eg_PSTW*2,  K_eg_PSTW*1,            0, -K_eg_PSTW*2,
	-K_eg_PSTW*2,            0,  K_eg_PSTW*1,  K_eg_PSTW*2,  K_eg_PSTW*2,  K_eg_PSTW*1,            0, -K_eg_PSTW*2,	
	-K_eg_PSTW*3, -K_eg_PSTW*1,            0,  K_eg_PSTW*1,  K_eg_PSTW*1,            0, -K_eg_PSTW*1, -K_eg_PSTW*3,
	-K_eg_PSTW*4, -K_eg_PSTW*2, -K_eg_PSTW*1,            0,            0, -K_eg_PSTW*1, -K_eg_PSTW*2, -K_eg_PSTW*4,
	-K_eg_PSTW*6, -K_eg_PSTW*4, -K_eg_PSTW*3, -K_eg_PSTW*2, -K_eg_PSTW*2, -K_eg_PSTW*3, -K_eg_PSTW*4, -K_eg_PSTW*6
};

//Rank Tables for passed and halfpassed pawns - similar to Toga
int RankBonus[8] = {0, 0, 0, 26, 77, 154, 256, 256}; //Divide by 256

//Another RankBonus
int RankBonus2[8] = {184, 184, 184, 202, 220, 238, 256, 256}; //Divide by 256

//pawn structure piece square tables for opening
//endgame pawn structure scores are the opening scores multiplied by P_eg/P_op.
int passedPawnPST[64] = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	27, 26, 24, 23, 23, 24, 26, 27,
	27, 26, 24, 23, 23, 24, 26, 27,
	36, 35, 33, 31, 31, 33, 35, 36,
	49, 47, 44, 42, 42, 44, 47, 49,
	66, 63, 60, 56, 56, 60, 63, 66,
	90, 85, 81, 76, 76, 81, 85, 90,
	 0,  0,  0,  0,  0,  0,  0,  0
};

int halfpassedPawnPST[64] = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	 8,  8,  7,  7,  7,  7,  8,  8,
	 8,  8,  7,  7,  7,  7,  8,  8,
	11, 10, 10,  9,  9, 10, 10, 11,
	15, 14, 13, 12, 12, 13, 14, 15,
	20, 19, 17, 16, 16, 17, 19, 20,
	27, 25, 24, 22, 22, 24, 25, 27,
	 0,  0,  0,  0,  0,  0,  0,  0
};

//Kingsafety Weight Table
//int KSWT[]=KS_NUMATTACKERS_WEIGHT_TABLE;

//Lone King Mate Recognizer PSTs
int recog_R_Q_BB[64] = {
	9, 8, 7, 6, 6, 7, 8, 9,
	8, 5, 4, 3, 3, 4, 5, 8,
	7, 4, 2, 1, 1, 2, 4, 7,
	6, 3, 1, 0, 0, 1, 3, 6,
	6, 3, 1, 0, 0, 1, 3, 6,
	7, 4, 2, 1, 1, 2, 4, 7,
	8, 5, 4, 3, 3, 4, 5, 8,
	9, 8, 7, 6, 6, 7, 8, 9,
};

//recognizer for BN black bishop mate
//use the flip pst for the white bishop mate
int recog_BN[64] = {
	10,  9,  8,  7,  6,  5,  4,  3,
	 9,  7,  6,  5,  4,  3,  2,  4,
	 8,  6,  4,  3,  2,  1,  3,  5,
	 7,  5,  3,  1,  0,  2,  4,  6,
	 6,  4,  2,  0,  1,  3,  5,  7,
	 5,  3,  1,  2,  3,  4,  6,  8,
	 4,  2,  3,  4,  5,  6,  7,  9,
	 3,  4,  5,  6,  7,  8,  9, 10,
};

#ifndef ANDRES_EVAL

//evauation that takes into account recognition of the type of position
int eval(const board* pos, int alpha, int beta)
{
	/***********************************************************************/
	/*                         Lone King Recognizer                        */
	/***********************************************************************/
	if(!extractMatSig(pos->recogsig,WHITE)) //lone white king
	{
		bool recognized=false;
		//scoring for black
		int score = RECOG_WIN+materialBLACK(pos);
		unsigned int msig = extractMatSig(pos->recogsig,BLACK);
		//Queen/Rook/Two Bishops
		if(msigHasPiece(msig,Q) || msigHasPiece(msig,R) || (pos->Pieces[B]&WHITESQUARES && pos->Pieces[B]&BLACKSQUARES))
		{
			recognized=true;
			score+=recog_R_Q_BB[pos->KingPos[WHITE]]-distance(pos->KingPos[WHITE],pos->KingPos[BLACK]);
		}
		//Bishop+Knight
		else if(msigHasPiece(msig,B) && msigHasPiece(msig,N))
		{
			recognized=true;
			if(pos->Pieces[B]&BLACKSQUARES)
				score+=recog_BN[pos->KingPos[WHITE]];
			else
				score+=recog_BN[FLIPV(pos->KingPos[WHITE])];
			score-=distance(pos->KingPos[WHITE],pos->KingPos[BLACK]);
		}
		//multiply score by -1 if the current side is white
		if(recognized)
		{
			if(pos->side==WHITE) return -score;
			else return score;
		}
	}
	else if(!extractMatSig(pos->recogsig,BLACK)) //lone black king
	{
		bool recognized=false;
		int score = RECOG_WIN+materialWHITE(pos);
		unsigned int msig = extractMatSig(pos->recogsig,WHITE);
		//Queen/Rook/Two Bishops
		if(msigHasPiece(msig,Q) || msigHasPiece(msig,R) || (pos->Pieces[B]&WHITESQUARES && pos->Pieces[B]&BLACKSQUARES))
		{
			recognized=true;
			score+=recog_R_Q_BB[pos->KingPos[BLACK]]-distance(pos->KingPos[BLACK],pos->KingPos[WHITE]);
		}
		//Bishop+Knight
		else if(msigHasPiece(msig,B) && msigHasPiece(msig,N))
		{
			recognized=true;
			if(pos->Pieces[B]&BLACKSQUARES)
				score+=recog_BN[pos->KingPos[BLACK]];
			else
				score+=recog_BN[FLIPV(pos->KingPos[BLACK])];
			score-=distance(pos->KingPos[BLACK],pos->KingPos[WHITE]);
		}
		//multiply score by -1 if the current side is black
		if(recognized)
		{
			if(pos->side==BLACK) return -score;
			else return score;
		}
	}
#ifdef ZERO_EVAL
	return 0;
#else
	return fulleval(pos,alpha,beta);
#endif
}

//pure eval
int fulleval(const board* pos, int alpha, int beta)
{
	/***********************************************************************/
	/*                         Variable Declaration                        */
	/***********************************************************************/

	//Evaluation Scores
	//phase-independent score
	int score = 0;

	//phase-dependent score
	//op[side], eg[side]
	//white's score gets mixed according to black's piece material
	//black's score gets mixed according to white's piece material
	int op[2] = {0,0};
	int eg[2] = {0,0};
	

	#ifdef DEBUG_EVAL
		int debugscore = 0;
		int debugop[2] = {0,0};
		int debugeg[2] = {0,0};
	#endif

	//the piece bitboards for each side
	U64 Pawns[2]={piecesWHITE(*pos,P),piecesBLACK(*pos,P)};
	U64 Knights[2]={piecesWHITE(*pos,N),piecesBLACK(*pos,N)};
	U64 Bishops[2]={piecesWHITE(*pos,B),piecesBLACK(*pos,B)};
	U64 Rooks[2]={piecesWHITE(*pos,R),piecesBLACK(*pos,R)};
	U64 Queens[2]={piecesWHITE(*pos,Q),piecesBLACK(*pos,Q)};
	U64 Kings[2]={piecesWHITE(*pos,K),piecesBLACK(*pos,K)};

	//the number of pieces for each side (except for kings)
	int numPawns[2]={popcnt(Pawns[WHITE]),popcnt(Pawns[BLACK])};
	int numKnights[2]={popcnt(Knights[WHITE]),popcnt(Knights[BLACK])};
	int numBishops[2]={popcnt(Bishops[WHITE]),popcnt(Bishops[BLACK])};
	int numRooks[2]={popcnt(Rooks[WHITE]),popcnt(Rooks[BLACK])};
	int numQueens[2]={popcnt(Queens[WHITE]),popcnt(Queens[BLACK])};
	
	//simple material score of sliders and knights
	int pieceMaterial[2]=
	{
		Nval*numKnights[WHITE]+Bval*numBishops[WHITE]+Rval*numRooks[WHITE]+Qval*numQueens[WHITE],
		Nval*numKnights[BLACK]+Bval*numBishops[BLACK]+Rval*numRooks[BLACK]+Qval*numQueens[BLACK]
	};

	//attacks for each side
	U64 Pattacks[2];
	U64 PdoubleAttacks[2];
	U64 Battacks[2]={U64EMPTY,U64EMPTY};
	U64 Nattacks[2]={U64EMPTY,U64EMPTY};
	U64 NdoubleAttacks[2]={U64EMPTY,U64EMPTY};
	U64 BNattacks[2];
	U64 Rattacks[2]={U64EMPTY,U64EMPTY};
	U64 RdoubleAttacks[2]={U64EMPTY,U64EMPTY};
	U64 Qattacks[2]={U64EMPTY,U64EMPTY};
	U64 Kattacks[2]={Kmoves(pos->KingPos[WHITE]),Kmoves(pos->KingPos[BLACK])};
	U64 attacks[2]; //discluding King attacks
	U64 multiattacks[2]; //square attacked multiple times

	//safe squares for each piece type discluding King attacks
	/*U64 Psafe[2];
	U64 BNsafe[2]={U64EMPTY,U64EMPTY};
	U64 Rsafe[2]={U64EMPTY,U64EMPTY};
	U64 Qsafe[2]={U64EMPTY,U64EMPTY};*/

	//safe attacks for each side
	/*U64 BsafeAttacks[2];
	U64 NsafeAttacks[2];
	U64 RsafeAttacks[2];
	U64 QsafeAttacks[2];*/

	//pawns bitboards
	U64 immobilePawns[2];
	U64 Pdefendable[2]; //squares that can possibly be attacked by pawn attacks
	U64 Pdefendable_x2[2]; //squares that can be possible attacked twice by pawn attacks
	U64 Pblockable[2];  //squares that can possibly be occupied by pawn moves
	//U64 PdefendableSafe[2]; //squares that can safely be attacked by pawn attacks
	//U64 PblockableSafe[2];  //squares that can safely be occupied by pawn moves
	U64 doubledPawns[2];
	U64 isolatedPawns[2];
	U64 unprotectedPawns[2]; //Pawns that are not immediately protected by a pawn or side-to-side with a pawn
	U64 backwardPawns[2]; //A pawn that cannot advance beyond a enemy pawn formation with help from only it's own pawns
	U64 passedPawns[2];
	U64 halfpassedPawns[2];
	//Pawn Targets
	//Backward pawns+root pawns that are parallel to each other
	//+pawns that are not backward but are blocked from pawn defence
	//+pawns that are not defended at all
	//basically hard to defend pawns
	//U64 pawnTargets[2];

	/***********************************************************************/
	/*                            Data Generation                          */
	/***********************************************************************/

	/******************** Initialize Attack Bitboards ********************/

	//Pawn attacks
	Pattacks[WHITE]=((Pawns[WHITE]&C64(0x7F7F7F7F7F7F7F7F))<<9)|((Pawns[WHITE]&C64(0xFEFEFEFEFEFEFEFE))<<7);
	PdoubleAttacks[WHITE]=((Pawns[WHITE]&C64(0x7F7F7F7F7F7F7F7F))<<9)&((Pawns[WHITE]&C64(0xFEFEFEFEFEFEFEFE))<<7);
	Pattacks[BLACK]=((Pawns[BLACK]&C64(0x7F7F7F7F7F7F7F7F))>>7)|((Pawns[BLACK]&C64(0xFEFEFEFEFEFEFEFE))>>9);
	PdoubleAttacks[BLACK]=((Pawns[BLACK]&C64(0x7F7F7F7F7F7F7F7F))>>7)&((Pawns[BLACK]&C64(0xFEFEFEFEFEFEFEFE))>>9);
	{
		//Knight attacks
		U64 pieceboard=Knights[WHITE];
		while(pieceboard)
		{
			int index;
			GetBitAndClear(pieceboard,index);
			NdoubleAttacks[WHITE]|=Nattacks[WHITE]&Nmoves(index);
			Nattacks[WHITE]|=Nmoves(index);
		}
		pieceboard=Knights[BLACK];
		while(pieceboard)
		{
			int index;
			GetBitAndClear(pieceboard,index);
			NdoubleAttacks[BLACK]|=Nattacks[BLACK]&Nmoves(index);
			Nattacks[BLACK]|=Nmoves(index);
		}

		//Bishop attacks
		pieceboard=Bishops[WHITE];
		while(pieceboard)
		{
			int index;
			GetBitAndClear(pieceboard,index);
			Battacks[WHITE]|=Bmoves(index,pos->AllPieces);
		}
		pieceboard=Bishops[BLACK];
		while(pieceboard)
		{
			int index;
			GetBitAndClear(pieceboard,index);
			Battacks[BLACK]|=Bmoves(index,pos->AllPieces);
		}

		BNattacks[WHITE]=Battacks[WHITE]|Nattacks[WHITE];
		BNattacks[BLACK]=Battacks[BLACK]|Nattacks[BLACK];

		//Rook attacks
		pieceboard=Rooks[WHITE];
		while(pieceboard)
		{
			U64 temp;
			int index;
			GetBitAndClear(pieceboard,index);
			temp=Rmoves(index,pos->AllPieces);
			RdoubleAttacks[WHITE]|=Rattacks[WHITE]&temp;
			Rattacks[WHITE]|=temp;
		}
		pieceboard=Rooks[BLACK];
		while(pieceboard)
		{
			U64 temp;
			int index;
			GetBitAndClear(pieceboard,index);
			temp=Rmoves(index,pos->AllPieces);
			RdoubleAttacks[BLACK]|=Rattacks[BLACK]&temp;
			Rattacks[BLACK]|=temp;
		}

		//Queen attacks
		pieceboard=Queens[WHITE];
		while(pieceboard)
		{
			int index;
			GetBitAndClear(pieceboard,index);
			Qattacks[WHITE]|=Qmoves(index,pos->AllPieces);
		}
		pieceboard=Queens[BLACK];
		while(pieceboard)
		{
			int index;
			GetBitAndClear(pieceboard,index);
			Qattacks[BLACK]|=Qmoves(index,pos->AllPieces);
		}

		//King attacks are not considered true attacks for the purposes of kingsafety
		//Nevertheless - when doing flood fills you must take into account the king attacks
		attacks[WHITE]=Pattacks[WHITE]|Nattacks[WHITE]|Battacks[WHITE]|Rattacks[WHITE]|Qattacks[WHITE]|Kattacks[WHITE];
		attacks[BLACK]=Pattacks[BLACK]|Nattacks[BLACK]|Battacks[BLACK]|Rattacks[BLACK]|Qattacks[BLACK]|Kattacks[BLACK];

		//multi-atacks
		multiattacks[WHITE]=
			NdoubleAttacks[WHITE]|RdoubleAttacks[WHITE]|
			(Nattacks[WHITE]&Battacks[WHITE])|(Nattacks[WHITE]&Rattacks[WHITE])|(Nattacks[WHITE]&Qattacks[WHITE])|
			(Battacks[WHITE]&Rattacks[WHITE])|(Battacks[WHITE]&Qattacks[WHITE])|
			(Rattacks[WHITE]&Qattacks[WHITE])|
			(Kattacks[WHITE]&Nattacks[WHITE])|(Kattacks[WHITE]&Battacks[WHITE])|(Kattacks[WHITE]&Rattacks[WHITE])|
			(Kattacks[WHITE]&Qattacks[WHITE]);
		multiattacks[BLACK]=
			NdoubleAttacks[BLACK]|RdoubleAttacks[BLACK]|
			(Nattacks[BLACK]&Battacks[BLACK])|(Nattacks[BLACK]&Rattacks[BLACK])|(Nattacks[BLACK]&Qattacks[BLACK])|
			(Battacks[BLACK]&Rattacks[BLACK])|(Battacks[BLACK]&Qattacks[BLACK])|
			(Rattacks[BLACK]&Qattacks[BLACK])|
			(Kattacks[BLACK]&Nattacks[BLACK])|(Kattacks[BLACK]&Battacks[BLACK])|(Kattacks[BLACK]&Rattacks[BLACK])|
			(Kattacks[BLACK]&Qattacks[BLACK]);
	}

	/******************** Initialize Safe Squares ********************/
	/*{
		U64 unsafe[2]={~attacks[WHITE]&attacks[BLACK],~attacks[BLACK]&attacks[WHITE]};
		Psafe[WHITE]=~(unsafe[WHITE]|pos->AllPieces);
		Psafe[BLACK]=~(unsafe[BLACK]|pos->AllPieces);
		unsafe[WHITE]|=Pattacks[BLACK];
		unsafe[BLACK]|=Pattacks[WHITE];
		unsafe[WHITE]|=(attacks[BLACK]&~multiattacks[WHITE])|multiattacks[BLACK];
		unsafe[BLACK]|=(attacks[WHITE]&~multiattacks[BLACK])|multiattacks[WHITE];
		BNsafe[WHITE]=~unsafe[WHITE];
		BNsafe[BLACK]=~unsafe[BLACK];
		unsafe[WHITE]|=Nattacks[BLACK]|Battacks[BLACK];
		unsafe[BLACK]|=Nattacks[WHITE]|Battacks[WHITE];
		Rsafe[WHITE]=~unsafe[WHITE];
		Rsafe[BLACK]=~unsafe[BLACK];
		unsafe[WHITE]|=Rattacks[BLACK];
		unsafe[BLACK]|=Rattacks[WHITE];
		Qsafe[WHITE]=~unsafe[WHITE];
		Qsafe[BLACK]=~unsafe[BLACK];
	}

	/******************** Initialize Safe Attacks ********************/
	/*BsafeAttacks[WHITE]=Battacks[WHITE]&BNsafe[WHITE];
	NsafeAttacks[WHITE]=Nattacks[WHITE]&BNsafe[WHITE];
	RsafeAttacks[WHITE]=Rattacks[WHITE]&Rsafe[WHITE];
	QsafeAttacks[WHITE]=Qattacks[WHITE]&Qsafe[WHITE];
	BsafeAttacks[BLACK]=Battacks[BLACK]&BNsafe[BLACK];
	NsafeAttacks[BLACK]=Nattacks[BLACK]&BNsafe[BLACK];
	RsafeAttacks[BLACK]=Rattacks[BLACK]&Rsafe[BLACK];
	QsafeAttacks[BLACK]=Qattacks[BLACK]&Qsafe[BLACK];

	/******************** Initialize Pawn Structure Bitboards ********************/
	immobilePawns[WHITE]=Pawns[WHITE]&((pos->PiecesSide[BLACK]|Pawns[WHITE])>>8);
	immobilePawns[BLACK]=Pawns[BLACK]&((pos->PiecesSide[WHITE]|Pawns[BLACK])<<8);

	Pblockable[WHITE] = fillUp2(Pawns[WHITE]);
	Pblockable[BLACK] = fillDown2(Pawns[BLACK]);

	/*PblockableSafe[WHITE]=fillUpOccluded(Pawns[WHITE],Psafe[WHITE]);
	PblockableSafe[BLACK]=fillDownOccluded(Pawns[BLACK],Psafe[BLACK]);*/

	Pdefendable[WHITE]=((Pblockable[WHITE]&C64(0x7F7F7F7F7F7F7F7F))<<9)|((Pblockable[WHITE]&C64(0xFEFEFEFEFEFEFEFE))<<7);
	Pdefendable[BLACK]=((Pblockable[BLACK]&C64(0x7F7F7F7F7F7F7F7F))>>7)|((Pblockable[BLACK]&C64(0xFEFEFEFEFEFEFEFE))>>9);

	Pdefendable_x2[WHITE]=((Pblockable[WHITE]&C64(0x7F7F7F7F7F7F7F7F))<<9)&((Pblockable[WHITE]&C64(0xFEFEFEFEFEFEFEFE))<<7);
	Pdefendable_x2[BLACK]=((Pblockable[BLACK]&C64(0x7F7F7F7F7F7F7F7F))>>7)&((Pblockable[BLACK]&C64(0xFEFEFEFEFEFEFEFE))>>9);

	/*PdefendableSafe[WHITE]=((PblockableSafe[WHITE]&C64(0x7F7F7F7F7F7F7F7F))<<9)|((PblockableSafe[WHITE]&C64(0xFEFEFEFEFEFEFEFE))<<7);
	PdefendableSafe[BLACK]=((PblockableSafe[BLACK]&C64(0x7F7F7F7F7F7F7F7F))>>7)|((PblockableSafe[BLACK]&C64(0xFEFEFEFEFEFEFEFE))>>9);*/

	doubledPawns[WHITE]=Pawns[WHITE] & fillDown(Pawns[WHITE]);
	doubledPawns[BLACK]=Pawns[BLACK] & fillUp(Pawns[BLACK]);

	passedPawns[WHITE]=Pawns[WHITE] & ~doubledPawns[WHITE] & ~(Pdefendable[BLACK]|Pblockable[BLACK]);
	passedPawns[BLACK]=Pawns[BLACK] & ~doubledPawns[BLACK] & ~(Pdefendable[WHITE]|Pblockable[WHITE]);

	unprotectedPawns[WHITE]=Pawns[WHITE]&~fillUp2(Pattacks[WHITE]>>8 & ~immobilePawns[WHITE]);
	unprotectedPawns[BLACK]=Pawns[BLACK]&~fillDown2(Pattacks[BLACK]<<8 & ~immobilePawns[BLACK]);

	backwardPawns[WHITE]=Pawns[WHITE] & 
		fillDown((Pattacks[BLACK]&~Pdefendable[WHITE]) | (PdoubleAttacks[BLACK]&~Pdefendable_x2[WHITE]));
	backwardPawns[BLACK]=Pawns[BLACK] &
		fillUp((Pattacks[WHITE]&~Pdefendable[BLACK]) | (PdoubleAttacks[WHITE]&~Pdefendable_x2[BLACK]));

	isolatedPawns[WHITE]=backwardPawns[WHITE]&~fillDown2(Pattacks[WHITE]>>8);
	isolatedPawns[BLACK]=backwardPawns[BLACK]&~fillUp2(Pattacks[BLACK]<<8);

	halfpassedPawns[WHITE]=~backwardPawns[WHITE]&(~passedPawns[WHITE])&(
		(Pawns[WHITE] & ~fillDown2(Pawns[BLACK]|((Pawns[BLACK]&C64(0x7F7F7F7F7F7F7F7F))>>7)))|
		(Pawns[WHITE] & ~fillDown2(Pawns[BLACK]|((Pawns[BLACK]&C64(0xFEFEFEFEFEFEFEFE))>>9))));
	halfpassedPawns[BLACK]=~backwardPawns[BLACK]&(~passedPawns[BLACK])&(
		(Pawns[BLACK] & ~fillUp2(Pawns[WHITE]|((Pawns[WHITE]&C64(0x7F7F7F7F7F7F7F7F))<<9)))|
		(Pawns[BLACK] & ~fillUp2(Pawns[WHITE]|((Pawns[WHITE]&C64(0xFEFEFEFEFEFEFEFE))<<7))));


	/*pawnTargets[WHITE]=Pawns[BLACK]&(~attacks[BLACK] |
		~fillDownOccluded(Pattacks[BLACK]<<8 & ~immobilePawns[BLACK],~(Pattacks[WHITE]>>8)));
	pawnTargets[BLACK]=Pawns[WHITE]&(~attacks[WHITE] |
		~fillUpOccluded(Pattacks[WHITE]>>8 & ~immobilePawns[WHITE],~(Pattacks[BLACK]<<8)));*/


	/***********************************************************************/
	/*                            Data analysis                            */
	/***********************************************************************/

	//Listed in order of decreasing importance for lazy eval

	/******************** Material and Piece Square Evaluation ********************/
	#ifdef MATERIAL_EVAL
	{
		//Pawn Material Eval is done in Pawn Eval
		U64 WhitePawns = Pawns[WHITE];
		U64 BlackPawns = Pawns[BLACK];
		
		//Pawn Material Eval done by Piece Square
		while(WhitePawns)
		{
			int index;
			GetBitAndClear(WhitePawns,index);
			op[WHITE]+=P_op_PST[index];
			eg[WHITE]+=P_eg_PST[index];

			//Distance of own king to pawn should be scaled by own material
			//Distance of opponent king to pawn should be scaled by opponent material
			#ifdef KING_DISTANCE_EVAL
			eg[BLACK]+=distance(index,pos->KingPos[WHITE])*RankBonus2[RANK(index)]*Kdist_side/256;
			eg[WHITE]+=distance(index,pos->KingPos[BLACK])*RankBonus2[RANK(index)]*Kdist_xside/256;
			#endif
		}
		while(BlackPawns)
		{
			int index;
			GetBitAndClear(BlackPawns,index);
			op[BLACK]+=P_op_PST[FLIPV(index)];
			eg[BLACK]+=P_eg_PST[FLIPV(index)];

			//Distance of own king to pawn should be scaled by own material
			//Distance of opponent king to pawn should be scaled by opponent material
			#ifdef KING_DISTANCE_EVAL
			eg[WHITE]+=distance(index,pos->KingPos[BLACK])*RankBonus2[RANK(FLIPV(index))]*Kdist_side/256;
			eg[BLACK]+=distance(index,pos->KingPos[WHITE])*RankBonus2[RANK(FLIPV(index))]*Kdist_xside/256;
			#endif
		}

		DEBUG_EVAL_PRINT("Pawn Material Eval and King Distance")

		//King Piece Square Eval
		op[WHITE]+=K_op_PST[pos->KingPos[WHITE]];
		eg[WHITE]+=K_eg_PST[pos->KingPos[WHITE]];
		op[BLACK]+=K_op_PST[FLIPV(pos->KingPos[BLACK])];
		eg[BLACK]+=K_eg_PST[FLIPV(pos->KingPos[BLACK])];

		DEBUG_EVAL_PRINT("King Piece Square Eval")

		//Regular Material Eval
		op[WHITE]+=
			N_op*(numKnights[WHITE]) +
			B_op*(numBishops[WHITE]) +
			R_op*(numRooks[WHITE])   +
			Q_op*(numQueens[WHITE]);
		eg[WHITE]+=
			N_eg*(numKnights[WHITE]) +
			B_eg*(numBishops[WHITE]) +
			R_eg*(numRooks[WHITE])   +
			Q_eg*(numQueens[WHITE]);
		op[BLACK]+=
			N_op*(numKnights[BLACK]) +
			B_op*(numBishops[BLACK]) +
			R_op*(numRooks[BLACK])   +
			Q_op*(numQueens[BLACK]);
		eg[BLACK]+=
			N_eg*(numKnights[BLACK]) +
			B_eg*(numBishops[BLACK]) +
			R_eg*(numRooks[BLACK])   +
			Q_eg*(numQueens[BLACK]);

		DEBUG_EVAL_PRINT("Piece Material Eval")
	}
	#endif
	

	/******************** Pawn Evaluvation ********************/

	#ifdef PAWN_EVAL
	{
		U64 doubledWHITE=doubledPawns[WHITE];
		U64 doubledBLACK=doubledPawns[BLACK];


		U64 isolatedWHITE=isolatedPawns[WHITE];
		U64 isolatedBLACK=isolatedPawns[BLACK];

		U64 unprotectedWHITE = unprotectedPawns[WHITE];
		U64 unprotectedBLACK = unprotectedPawns[BLACK];

		/*U64 backwardWHITE=backwardPawns[WHITE]&~(isolatedPawns[WHITE]|doubledPawns[WHITE]);
		U64 backwardBLACK=backwardPawns[BLACK]&~(isolatedPawns[BLACK]|doubledPawns[BLACK]);*/

		U64 passedWHITE=passedPawns[WHITE];
		U64 passedBLACK=passedPawns[BLACK];

		U64 halfpassedWHITE=halfpassedPawns[WHITE];
		U64 halfpassedBLACK=halfpassedPawns[BLACK];

		while(doubledWHITE)
		{
			int sq;
			GetBitAndClear(doubledWHITE,sq);
			op[WHITE]-=doubledPenalty_op;
			eg[WHITE]-=doubledPenalty_eg;
		}
		while(doubledBLACK)
		{
			int sq;
			GetBitAndClear(doubledBLACK,sq);
			op[BLACK]-=doubledPenalty_op;
			eg[BLACK]-=doubledPenalty_eg;
		}
		DEBUG_EVAL_PRINT("Doubled Pawn Eval")

		//unprotected pawns
		while(unprotectedWHITE)
		{
			int sq;
			GetBitAndClear(unprotectedWHITE,sq);
			op[WHITE]-=unprotectedPenalty_op;
			eg[WHITE]-=unprotectedPenalty_eg;
		}
		while(unprotectedBLACK)
		{
			int sq;
			GetBitAndClear(unprotectedBLACK,sq);
			op[BLACK]-=unprotectedPenalty_op;
			eg[BLACK]-=unprotectedPenalty_eg;
		}
		DEBUG_EVAL_PRINT("Unprotected Pawn Eval")

		//backward pawns
		/*while(backwardWHITE)
		{
			int sq;
			GetBitAndClear(backwardWHITE,sq);
			op[WHITE]-=backwardPenalty_op;
			eg[WHITE]-=backwardPenalty_eg;
		}
		while(backwardBLACK)
		{
			int sq;
			GetBitAndClear(backwardBLACK,sq);
			op[BLACK]-=backwardPenalty_op;
			eg[BLACK]-=backwardPenalty_eg;
		}
		DEBUG_EVAL_PRINT("Backward Pawn Eval")*/

		//isolated pawns
		while(isolatedWHITE)
		{
			int sq;
			GetBitAndClear(isolatedWHITE,sq);
			op[WHITE]-=isolatedPenalty_op;
			eg[WHITE]-=isolatedPenalty_eg;
			//Bonus for enemy king too far
			//Bonus for our king close to it
		}
		while(isolatedBLACK)
		{
			int sq;
			GetBitAndClear(isolatedBLACK,sq);
			op[BLACK]-=isolatedPenalty_op;
			eg[BLACK]-=isolatedPenalty_eg;
			//Bonus for enemy king too far
			//Bonus for our king close to it
		}
		DEBUG_EVAL_PRINT("Isolated Pawn Eval")

		//Passed pawns

		while(passedWHITE)
		{
			int sq;
			GetBitAndClear(passedWHITE,sq);
			op[WHITE]+=passerBonus_op+passerRBonus_op*RankBonus[RANK(sq)]/256;
			eg[WHITE]+=passerBonus_eg+passerRBonus_eg*RankBonus[RANK(sq)]/256;

			if(passedPawns[WHITE]&((FILE(sq)!=FILEA?FILEBB(FILE(sq)-1):0) | (FILE(sq)!=FILEH?FILEBB(FILE(sq)+1):0)))
			{
				op[WHITE]+=connectedPasser_Bonus_op;
				eg[WHITE]+=connectedPasser_Bonus_eg;
			}

			#ifdef KING_DISTANCE_EVAL
			eg[BLACK]+=distance(sq+8,pos->KingPos[WHITE])*RankBonus2[RANK(sq)]*Kdist_passer_side/256;
			eg[WHITE]+=distance(sq+8,pos->KingPos[BLACK])*RankBonus2[RANK(sq)]*Kdist_passer_xside/256;
			#endif
		}
		while(passedBLACK)
		{
			int sq;
			GetBitAndClear(passedBLACK,sq);
			op[BLACK]+=passerBonus_op+passerRBonus_op*RankBonus[RANK(FLIPV(sq))]/256;
			eg[BLACK]+=passerBonus_eg+passerRBonus_eg*RankBonus[RANK(FLIPV(sq))]/256;

			if(passedPawns[BLACK]&((FILE(sq)!=FILEA?FILEBB(FILE(sq)-1):0) | (FILE(sq)!=FILEH?FILEBB(FILE(sq)+1):0)))
			{
				op[BLACK]+=connectedPasser_Bonus_op;
				eg[BLACK]+=connectedPasser_Bonus_eg;
			}
			
			/*if(pos->AllPieces^(Kings[WHITE]|Kings[BLACK]))
			{
				if(pos->side==WHITE)
				{
					if(distance(KingPos[WHITE]))
				}

			}*/

			#ifdef KING_DISTANCE_EVAL
			eg[WHITE]+=distance(sq-8,pos->KingPos[BLACK])*RankBonus2[RANK(FLIPV(sq))]*Kdist_passer_side/256;
			eg[BLACK]+=distance(sq-8,pos->KingPos[WHITE])*RankBonus2[RANK(FLIPV(sq))]*Kdist_passer_xside/256;
			#endif
		}
		DEBUG_EVAL_PRINT("Passed Pawn Eval")

		//half-passed pawns that are defendable

		while(halfpassedWHITE)
		{
			int sq;
			GetBitAndClear(halfpassedWHITE,sq);
			op[WHITE]+=halfPasserBonus_op+halfPasserRBonus_op*RankBonus[RANK(sq)]/256;
			eg[WHITE]+=halfPasserBonus_eg+halfPasserRBonus_eg*RankBonus[RANK(sq)]/256;

			#ifdef KING_DISTANCE_EVAL
			eg[BLACK]+=distance(sq+8,pos->KingPos[WHITE])*RankBonus2[RANK(sq)]*Kdist_halfpasser_side/256;
			eg[WHITE]+=distance(sq+8,pos->KingPos[BLACK])*RankBonus2[RANK(sq)]*Kdist_halfpasser_xside/256;
			#endif
			//Bonus for enemy king too far
			//Bonus for our king close to it
		}
		while(halfpassedBLACK)
		{
			int sq;
			GetBitAndClear(halfpassedBLACK,sq);
			op[BLACK]+=halfPasserBonus_op+halfPasserRBonus_op*RankBonus[RANK(FLIPV(sq))]/256;
			eg[BLACK]+=halfPasserBonus_eg+halfPasserRBonus_eg*RankBonus[RANK(FLIPV(sq))]/256;

			#ifdef KING_DISTANCE_EVAL
			eg[WHITE]+=distance(sq-8,pos->KingPos[BLACK])*RankBonus2[RANK(FLIPV(sq))]*Kdist_halfpasser_side/256;
			eg[BLACK]+=distance(sq-8,pos->KingPos[WHITE])*RankBonus2[RANK(FLIPV(sq))]*Kdist_halfpasser_xside/256;
			#endif
			//Bonus for enemy king too far
			//Bonus for our king close to it
		}
		DEBUG_EVAL_PRINT("Half-passed Pawn Eval")
		//DEBUG_EVAL_PRINT("Pawn Eval")
	}
	#endif

	/******************** Mobility Evaluation ********************/
	#ifdef MOBILITY_EVAL
	{
		int mobility[2];
		mobility[WHITE] =
			MobB_op*popcnt(Battacks[WHITE]) + 
			MobN_op*popcnt(Nattacks[WHITE]) +
			MobR_op*popcnt(Rattacks[WHITE]) +
			MobQ_op*popcnt(Qattacks[WHITE]) ;

		mobility[BLACK] =
			MobB_op*popcnt(Battacks[BLACK]) + 
			MobN_op*popcnt(Nattacks[BLACK]) +
			MobR_op*popcnt(Rattacks[BLACK]) +
			MobQ_op*popcnt(Qattacks[BLACK]) ;

		score += mobility[pos->side] - mobility[pos->xside];

		DEBUG_EVAL_PRINT("Mobility Eval")
	}
	#endif

	/******************** Compile Scores ********************/
	{
		int compiled_score[2] = {
			SCALE_SCORE(op[WHITE],eg[WHITE],pieceMaterial[BLACK]),
			SCALE_SCORE(op[BLACK],eg[BLACK],pieceMaterial[WHITE])
		};
		score += compiled_score[pos->side] - compiled_score[pos->xside];
	}

	if(score>=RECOG_WIN)
		score = RECOG_WIN-1;
	else if(score<=-RECOG_WIN)
		score = -RECOG_WIN+1;

	return score;
}
#endif

//returns a flipped bitboard
U64 symmetryCheckHelper(U64 x)
{
	U64 ret=U64EMPTY;
	ret|=(C64(0xFF)&x)<<56;
	ret|=(C64(0xFF00)&x)<<40;
	ret|=(C64(0xFF0000)&x)<<24;
	ret|=(C64(0xFF000000)&x)<<8;
	ret|=(C64(0xFF00000000000000)&x)>>56;
	ret|=(C64(0xFF000000000000)&x)>>40;
	ret|=(C64(0xFF0000000000)&x)>>24;
	ret|=(C64(0xFF00000000)&x)>>8;
	return ret;
}

//checks if the eval for the current position is symmetrical
//this should always return true unless there is a bug
bool symmetryCheck(const board* b)
{
	board flipb;
	flipb.castling=(b->castling>>2)|((b->castling&0x3)<<2);
	flipb.side=b->xside;
	flipb.xside=b->side;
	flipb.EP=(unsigned char)(FLIPV(b->EP));
	flipb.AllPieces=symmetryCheckHelper(b->AllPieces);
	flipb.fifty=b->fifty;
	flipb.hashkey=b->hashkey;
	{
		int i;
		for(i=P;i<=K;i++)
			flipb.Pieces[i]=symmetryCheckHelper(b->Pieces[i]);
		for(i=0;i<64;i++)
			flipb.PieceTypes[i]=b->PieceTypes[FLIPV(i)];
	}
	flipb.PiecesSide[WHITE]=symmetryCheckHelper(b->PiecesSide[BLACK]);
	flipb.PiecesSide[BLACK]=symmetryCheckHelper(b->PiecesSide[WHITE]);
	flipb.KingPos[WHITE]=(unsigned char)(FLIPV(b->KingPos[BLACK]));
	flipb.KingPos[BLACK]=(unsigned char)(FLIPV(b->KingPos[WHITE]));
	{
		unsigned int MaT_SiG[2] = {extractMatSig(b->recogsig,WHITE),extractMatSig(b->recogsig,BLACK)};
		flipb.recogsig = rsig(MaT_SiG[BLACK],MaT_SiG[WHITE]);
	}
	if(eval(&flipb,-INF,INF)!=eval(b,-INF,INF))
	{
		/*print("Symmetry Check Failed!\n");
		printBoard(b);
		fen(b);
		print("Eval = %d\n",eval(b,-INF,INF));
		printBoard(&flipb);
		fen(&flipb);
		print("Eval = %d\n",eval(&flipb,-INF,INF));
		system("Pause");*/
		return false;
	}
	return true;
}

int materialWHITE(const board* pos)
{
	return   Pval*(popcnt(piecesWHITE(*pos,P)) - popcnt(piecesBLACK(*pos,P)))
			+Nval*(popcnt(piecesWHITE(*pos,N)) - popcnt(piecesBLACK(*pos,N)))
			+Rval*(popcnt(piecesWHITE(*pos,R)) - popcnt(piecesBLACK(*pos,R)))
			+Bval*(popcnt(piecesWHITE(*pos,B)) - popcnt(piecesBLACK(*pos,B)))
			+Qval*(popcnt(piecesWHITE(*pos,Q)) - popcnt(piecesBLACK(*pos,Q)));
}

int materialBLACK(const board* pos)
{
	return  -materialWHITE(pos);
}

int materialSide(const board* pos)
{
	if(pos->side) //Black
		return materialBLACK(pos);
	else
		return materialWHITE(pos);
}

int materialXSide(const board* pos)
{
	return -materialSide(pos);
}

/*******************************************************************************
*                                  ANDRES EVAL                                 *
********************************************************************************/

#ifdef ANDRES_EVAL
//load Dll
//typedef int (__stdcall *importFunction)(int side, int alpha, int beta, int pieceList[2][16], int pieces[64], int color[64], int passed[2][8], int halfpassed[2][8], int doubled[2][8], int isolated[2][8], int backward[2][8]);
//typedef int (__stdcall *importFunction)(int side, int alpha, int beta, int pieceList[2][16], int pieces[64], int color[64]);
typedef int (__stdcall *importFunction)(int side, int alpha, int beta, int pieces[64], int color[64]);
typedef int (__stdcall *importInitFunction)(char* path, char* password, int cache_size);
importFunction dirtyeval=NULL;
importInitFunction InitDirtyDll=NULL;
char degbb[4096]="";
int dcache=32;

void init_AndresEval()
{
	HINSTANCE hinstLib = NULL;
	hinstLib=LoadLibrary("dirty.dll");
	if(hinstLib == NULL)
	{
		printf("Error could not load eval dll\n");
		exit(EXIT_FAILURE);
	}
	dirtyeval = (importFunction)GetProcAddress(hinstLib, "DirtyEval");
	InitDirtyDll = (importInitFunction)GetProcAddress(hinstLib, "InitDirtyDll");
	
	if(dirtyeval==NULL)
	{
		printf("Error could not get function pointer for eval from dirty.dll\n");
		exit(EXIT_FAILURE);
	}
	if(InitDirtyDll==NULL)
	{
		printf("Error could not get function pointer for init from dirty.dll\n");
		exit(EXIT_FAILURE);
	}
	InitDirtyDll(degbb,"DIRTY DOZEN INC.",dcache);
}

int AndresEval(const board* pos, int alpha, int beta)
{
	int color[64], pieces[64];
	int i; U64 j;
	for(i=0, j=1; j ; i++, j<<=1)
	{
		pieces[i]=pos->PieceTypes[i];
		if(j&pos->PiecesSide[WHITE])
		{
			color[i]=WHITE;
			//pieceList[WHITE][plcount[WHITE]++]=i;
		}
		else if(j&pos->PiecesSide[BLACK])
		{
			color[i]=BLACK;
			//pieceList[BLACK][plcount[BLACK]++]=i;
		}
		else
			color[i]=NOSIDE;
	}
	return dirtyeval(pos->side,alpha,beta,pieces,color);
}
#endif


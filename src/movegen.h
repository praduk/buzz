/**
*Buzz Chess Engine
*movegen.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _movegenh
#define _movegenh

/*
*Dependancies
*/

#include "defs.h"
#include "board.h"
#include "magicmoves.h"

/*
*Build Configuration
*/

#define QSEARCH_QPROMOTIONS_ONLY

//#define KING_MOVEGEN1 /*Generates attacksTo squares around the king.*/
#define KING_MOVEGEN2 /*Ands out attacks from squares around the king.*/

/*
*Defenitions and Macros
*/

#define Pmoves(square, side) Pmoves[side][square]
#define Pcaps(square, side) Pcaps[side][square]
#define Kmoves(square) Kmoves[square]
#define Nmoves(square) Nmoves[square]
#define Rmoves(square,occupancy) Rmagic(square,occupancy)
#define Bmoves(square,occupancy) Bmagic(square,occupancy)
#define Qmoves(square,occupancy) Qmagic(square,occupancy)

#define BmovesNoOcc(square) BmovesNoOcc[square]
#define RmovesNoOcc(square) RmovesNoOcc[square]
#define QmovesNoOcc(square) (BmovesNoOcc(square)|RmovesNoOcc(square))

#define inBetween(sq1,sq2) inBetween[sq1][sq2]
#define lineOf(sq1,sq2) lineOf[sq1][sq2]

//Defenition of a move is in board.h
#define	MAX_MOVECHOICES 256

//a scored move
typedef struct _smove
{
	move m;
	int score;
}smove;

//a move list
typedef struct _moveList
{
	smove moves[MAX_MOVECHOICES];
	unsigned int moveCount;
}moveList;

/*
*Function Prototypes
*/


void initMoveGen();
void genMoves(const board* pos, moveList* list);
void genQMoves(const board* pos, moveList* list);
void genQCMoves(const board* pos, moveList* list);
void genEvasions(const board* pos, moveList* list, U64 checkers);
U64 possiblePinned(const board* pos, const bool side);
bool moveIsLegal(const board* pos, const move m);
U64 Rfill(U64 pieces, U64 occupancy);
U64 Bfill(U64 pieces, U64 occupancy);
U64 Qfill(U64 pieces, U64 occupancy);
U64 Nfill(U64 pieces);
U64 Kfill(U64 pieces);

/*
*Global Data
*/

extern const U64 Pmoves[2][64];
extern const U64 Pcaps[2][64];
extern const U64 Kmoves[64];
extern const U64 Nmoves[64];

extern const U64 BmovesNoOcc[64];
extern const U64 RmovesNoOcc[64];

extern U64 inBetween[64][64];
extern U64 lineOf[64][64];

extern const U64 KNFill[64];
extern const U64 KBFill[64];
extern const U64 KRFill[64];

/*
*Inlined Functions
*/

//gets all attackers to a square from both sides
#define attacksToB(pos,square) (Bmoves(square,(pos).AllPieces) & (pos).Pieces[B])
#define attacksToR(pos,square) (Rmoves(square,(pos).AllPieces) & (pos).Pieces[R])
#define attacksToQ(pos,square) (Qmoves(square,(pos).AllPieces) & (pos).Pieces[Q])
#define attacksToRBQ(pos,square) ((Bmoves(square,(pos).AllPieces) & ((pos).Pieces[B]|(pos).Pieces[Q])) | (Rmoves(square,(pos).AllPieces) & ((pos).Pieces[R]|(pos).Pieces[Q])))
#define attacksToN(pos,square) (Nmoves(square) & (pos).Pieces[N])
#define attacksToK(pos,square) (Kmoves(square) & (pos).Pieces[K])

//pawn attackers from a certain side (attackers from a side, not the guy being attacked)
#define attacksToP(pos,square,side) (Pcaps[!(side)][square]&((pos).Pieces[P]&(pos).PiecesSide[side]))

#define attacksTo(pos,square) ((Bmoves(square,(pos).AllPieces) & ((pos).Pieces[B]|(pos).Pieces[Q]))|(Rmoves(square,(pos).AllPieces)& ((pos).Pieces[R]|(pos).Pieces[Q]))|(Nmoves[square]&(pos).Pieces[N])|(Kmoves[square]&(pos).Pieces[K])|(Pcaps[WHITE][square]&((pos).Pieces[P]&(pos).PiecesSide[BLACK]))|(Pcaps[BLACK][square]&((pos).Pieces[P]&(pos).PiecesSide[WHITE])))
//returns whether the side is attacked at the square - also retruns locatin of all attackers
#define isAttacked(pos,square,side) (attacksTo(pos,square)& (pos).PiecesSide[!(side)])
//returns whether the side is in check and the pieces checking
#define inCheck(pos,side) (attacksToP(pos,(pos).KingPos[side],!(side)) | (((pos).PiecesSide[!(side)]) & (attacksToRBQ(pos,(pos).KingPos[side]) | attacksToN(pos,(pos).KingPos[side]) | attacksToK(pos,(pos).KingPos[side]) )))

#define attacksToOcc(pos,square,occ) ((Bmoves(square,(occ)) & ((pos).Pieces[B]|(pos).Pieces[Q]))|(Rmoves(square,(occ))& ((pos).Pieces[R]|(pos).Pieces[Q]))|(Nmoves[square]&(pos).Pieces[N])|(Kmoves[square]&(pos).Pieces[K])|(Pcaps[WHITE][square]&((pos).Pieces[P]&(pos).PiecesSide[BLACK]))|(Pcaps[BLACK][square]&((pos).Pieces[P]&(pos).PiecesSide[WHITE])))
#define attacksBQRToOcc(pos,square,occ) ((Bmoves(square,(occ)) & ((pos).Pieces[B]|(pos).Pieces[Q]))|(Rmoves(square,(occ))& ((pos).Pieces[R]|(pos).Pieces[Q])))
#define isAttackedOcc(pos,square,side,occ) (attacksToOcc(pos,square,occ) & (pos).PiecesSide[!(side)])

/*INLINE U64 attacksTo(const board pos, const unsigned int square)
{
	return ((Bmoves(square,pos.AllPieces) & (pos.Pieces[B]|pos.Pieces[Q])) 
		|(Rmoves(square,pos.AllPieces)& (pos.Pieces[R]|pos.Pieces[Q]))
		|(Nmoves[square]&pos.Pieces[N])|(Kmoves[square]&pos.Pieces[K])
		|(Pcaps[WHITE][square]&(pos.Pieces[P]&pos.PiecesSide[BLACK]))
		|(Pcaps[BLACK][square]&(pos.Pieces[P]&pos.PiecesSide[WHITE])));
}

//returns opponent attacks to the square
INLINE U64 isAttacked(const board pos, const unsigned int square, const unsigned int side)
{
	return attacksTo(pos, square) & pos.PiecesSide[!side];
}

INLINE U64 inCheck(const board pos, const unsigned int side)
{
	return isAttacked(pos,toIndex(pos.Pieces[K]&pos.PiecesSide[side]), side);
}*/

#endif //movegenh

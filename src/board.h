/**
*Buzz Chess Engine
*board.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _boardh
#define _boardh

#include "defs.h"


/*
*Build Configuration
*/

//#define REVERSE_ZOBRIST_COLORS

#if defined(_WIN32) || defined(_WIN64)
	#define FANCY_PRINTBOARD
#endif
#define DEFAULT_HORIZONTAL_CHARECTER '-'
#define DEFAULT_VERTICAL_CHARECTER '|'
#define DEFAULT_INTERSECTION_CHARECTER '+'

/*
*Definitions
*/

//Define the colors
#define WHITE  0
#define BLACK  1
#define NOSIDE 2

//Define the various types of game results
#define RESULT_NONE       0

#define RESULT_WHITE_WINS 1
#define WHITE_MATES       (RESULT_WHITE_WINS|(1<<2))
#define BLACK_RESIGNS     (RESULT_WHITE_WINS|(2<<2))
#define WHITE_FLAGS       (RESULT_WHITE_WINS|(3<<2))

#define RESULT_BLACK_WINS 2
#define BLACK_MATES       (RESULT_BLACK_WINS|(1<<2))
#define WHITE_RESIGNS     (RESULT_BLACK_WINS|(2<<2))
#define BLACK_FLAGS       (RESULT_BLACK_WINS|(3<<2))

#define RESULT_DRAW       3
#define RESULT_STALEMATE  (RESULT_DRAW|(1<<2))
#define RESULT_REPETITION (RESULT_DRAW|(2<<2))
#define RESULT_FIFTY      (RESULT_DRAW|(3<<2))
#define RESULT_MATERIAL   (RESULT_DRAW|(4<<2))
#define RESULT_AGREEMENT  (RESULT_DRAW|(5<<2))

#define RESULT_PLAIN(X)   ((X)&3)

//First off the pieces
#define P 0
#define N 1
#define B 2
#define R 3
#define Q 4
#define K 5
#define NOPIECE 6
#define E NOPIECE
#define NUMPIECETYPES 6

//The squares - I didn't want to use enum to make these general purpose
#define A1  0
#define B1  1
#define C1  2
#define D1  3
#define E1  4
#define F1  5
#define G1  6
#define H1  7
#define A2  8
#define B2  9
#define C2 10
#define D2 11
#define E2 12
#define F2 13
#define G2 14
#define H2 15
#define A3 16
#define B3 17
#define C3 18
#define D3 19
#define E3 20
#define F3 21
#define G3 22
#define H3 23
#define A4 24
#define B4 25
#define C4 26
#define D4 27
#define E4 28
#define F4 29
#define G4 30
#define H4 31
#define A5 32
#define B5 33
#define C5 34
#define D5 35
#define E5 36
#define F5 37
#define G5 38
#define H5 39
#define A6 40
#define B6 41
#define C6 42
#define D6 43
#define E6 44
#define F6 45
#define G6 46
#define H6 47
#define A7 48
#define B7 49
#define C7 50
#define D7 51
#define E7 52
#define F7 53
#define G7 54
#define H7 55
#define A8 56
#define B8 57
#define C8 58
#define D8 59
#define E8 60
#define F8 61
#define G8 62
#define H8 63

//Files and ranks
#define FILEA 0
#define FILEB 1
#define FILEC 2
#define FILED 3
#define FILEE 4
#define FILEF 5
#define FILEG 6
#define FILEH 7
#define RANK1 0
#define RANK2 1
#define RANK3 2
#define RANK4 3
#define RANK5 4
#define RANK6 5
#define RANK7 6
#define RANK8 7

//Given the square number X
#define FILE(X) ((X)%8)
#define RANK(X) ((X)/8)

//Get the file bitboard for a file or rank
#define FILEBB(file) (C64(0x0101010101010101)<<(file))
#define RANKBB(rank) (C64(0xFF)<<(rank))

//Flip a square vertically or horizontally
#define FLIPV(X) ((X)^56) /*Example: square 0 becomes 56*/
#define FLIPH(X) ((X)^7)  /*Example: square 0 becomes  7*/


//A move
typedef unsigned int move;

/*********************Format of a Move**************************
Bits    (6)	0-5     are for the "to square".
Bits    (3)	6-8     are for the "capture piece".
Bits    (3)	9-11    are for the "moving piece".
Bits    (6)	12-17   are for the "from square".
Bits    (3)	18-20   are for the "promoting piece".
Bits    (6) 21-26   is for the "enpassant pawn being captured"
Bit     (1) 21      is for "kingside castling"
Bit     (1) 22      is for "queenside castling"
Bits    (5) 27-31   Extra info that will be ignored by makemove
****************************************************************/

//A null move
#define NULLMOVE 0

//macros to help encode data X into a move
#define encodeTo(X)         ((move)X)
#define encodeCapture(X)    (((move)X)<<6)
#define encodePiece(X)      (((move)X)<<9)
#define encodeFrom(X)       (((move)X)<<12)
#define encodePromotion(X)  (((move)X)<<18)
#define encodeEP(X)         (((move)X)<<21)
#define encodeExtra(X)      (((move)X)<<27)

//macros to help extract data from move X
#define extractTo(m)        ((m)&0x3F)
#define extractCapture(m)   (((m)>>6)&7)
#define extractPiece(m)     (((m)>>9)&7)
#define extractFrom(m)      (((m)>>12)&0x3F)
#define extractPromotion(m) (((m)>>18)&7)
#define extractEP(m)	    (((m)>>21)&0x3F)
#define extractCastling(m)  (((m)>>21)&7)
#define extractExtra(m)     ((m)>>27)
                                       
#define stripMove(m)        ((m)&0x7FFFFFF)

//castling moves
#define KingsideCastle             0x201C00
#define QueensideCastle            0x401C00

//the board structure
typedef struct _board //104 bytes
{
	U64   AllPieces;                 //All the pieces on the board
	U64   PiecesSide[2];             //All the pieces belonging to each side
	U64   Pieces[NUMPIECETYPES];     //Specific pieces on the board eg Pieces[R]
	U64   hashkey;                   //the hash key for the transposition table
	unsigned int  recogsig;          //the recognizer signature
	unsigned char PieceTypes[64];    //All the piece types according to squares
	unsigned char KingPos[2];        //King position for a side
	unsigned char EP;                //The enpassant square
	unsigned char castling;          //Castling privilages - format defined below
	unsigned char fifty;             //Fifty move count
	unsigned char SF;                //Search Flags (see Search Related section)
	bool side;                       //the side to play
	bool xside;                      //the side opposite to play
}board;

#define piecesSide(pos,piece)   ((pos).Pieces[(piece)]&(pos).PiecesSide[(pos).side])
#define piecesXSide(pos,piece)  ((pos).Pieces[(piece)]&(pos).PiecesSide[(pos).xside])
#define piecesWHITE(pos,piece)   ((pos).Pieces[(piece)]&(pos).PiecesSide[WHITE])
#define piecesBLACK(pos,piece)   ((pos).Pieces[(piece)]&(pos).PiecesSide[BLACK])

//SearchFlags are of type U8
/*******************Format of a Search Flag*********************
Bit     (1)	0		nullmove was just made in search
****************************************************************/
#define encodeSFNull             1
#define extractSFNull(SF)        ((SF)&encodeSFNull)

#define clearSearchFlags(SF)       (SF)=0


/*******************Castling Format*********************************************
 *(White KingSide) (White QueenSide) (Black KingSide) (Black QueenSide)
 *If black can castle on both sides and white can castle only on kingside, then
 *the number would be 0111 in binary.
 *
 *BK - Black Kingside
 *BQ - Black Queenside
 *WK - White Kingside
 *WQ - White Queenside
 ******************************************************************************/

#define KINGSIDE	0
#define QUEENSIDE	1

//castlings
#define WK 1 // 0001
#define WQ 2 // 0010
#define BK 4 // 0100
#define BQ 8 // 1000

//castlings for each side
#define BCASTLE	(BK|BQ) //(1100)
#define WCASTLE	(WK|WQ) //(0011)

//some defines for castling privilages
#define canCastle(pos,side) ((side)?((pos).castling)>>2:((pos).castling)&0x3)
#define canCastleWhite(pos) (((pos).castling)&0x3)
#define canCastleBlack(pos) (((pos).castling)>>2)

#define canCastleKingside(pos,side)  (canCastle(pos,side)&0x1)
#define canCastleQueenside(pos,side)  (canCastle(pos,side)&0x1)

//finding the column and row number of a square
#define COL(X) ((X)&0x7) /* X%8 */
#define ROW(X) ((X)>>3)  /* X/8 */

//white and black squares
#define WHITESQUARES C64(0x55AA55AA55AA55AA)
#define BLACKSQUARES C64(0xAA55AA55AA55AA55)

//Charecters used for printBoard
#ifdef FANCY_PRINTBOARD
	//corners
	#define PB_UL	218	//upper left corner
	#define PB_UR	191	//upper right corner
	#define PB_LL	192	//lower left corner
	#define PB_LR	217	//lower right corner
	//sides and intersections
	#define PB_H	196	//horizontal piece
	#define PB_V	179	//vertical piece
	#define PB_HD	194	//horizontal down
	#define PB_HU	193	//horizontal up
	#define PB_VR	195	//vertical right
	#define PB_VL	180	//vertical left
	#define PB_CRS	197	//cross
#else
	//corners
	#define PB_UL	DEFAULT_INTERSECTION_CHARECTER //upper left corner
	#define PB_UR	DEFAULT_INTERSECTION_CHARECTER //upper right corner
	#define PB_LL	DEFAULT_INTERSECTION_CHARECTER //lower left corner
	#define PB_LR	DEFAULT_INTERSECTION_CHARECTER //lower right corner
	//sides and intersections
	#define PB_H	DEFAULT_HORIZONTAL_CHARECTER    //horizontal piece
	#define PB_V	DEFAULT_VERTICAL_CHARECTER	    //vertical piece
	#define PB_HD	DEFAULT_INTERSECTION_CHARECTER  //horizontal down
	#define PB_HU	DEFAULT_INTERSECTION_CHARECTER  //horizontal up
	#define PB_VR	DEFAULT_INTERSECTION_CHARECTER  //vertical right
	#define PB_VL	DEFAULT_INTERSECTION_CHARECTER  //vertical left
	#define PB_CRS	DEFAULT_INTERSECTION_CHARECTER  //cross
#endif

//defenition of the length of a SAN/coordinate notation string
#define SAN_LENGTH 8
#define COORD_LENGTH 7

//definition of a SAN/coordinate notation string
typedef char sanString[SAN_LENGTH];
typedef char coordString[COORD_LENGTH];

#define ZOBRIST_CHANGE_SIDE(key) (~(key));

/*
*Macros
*/

#define pieceToChar(piece) pieceToCharUC[piece]
#define pieceToCharUC(piece) pieceToCharUC[piece]
#define pieceToCharLC(piece) pieceToCharLC[piece]

/*
*Function Prototypes
*/

void printBB(const U64 bb);
void printHex(const U64 bb);
void printU64(U64 bb);
void printBoard(const board* pos);
void printDebugBoard(const board* pos);
bool boardIsOk(const board* pos);
int coordToSquare(const char* coord);
unsigned char charToPiece(char c);
void moveToCoords(const board* pos, const move m, char* string);
void moveToSAN(const board* pos, const move m, char* string);
void moveToCBSAN(const board* pos, const move m, char* string);
void moveToLSAN(const board* pos, const move m, char* string);
move coordsToMove(const board* pos, const char* coord);
move stringToMove(const board* pos, const char* string);

U64 staticHashKey(const board* pos);
void generateZobrist();

void makemove(board* pos, const move m);

bool gameDecidableDraw(board* pos);
int distance(int sq1, int sq2);

/*
*Global Data
*/

extern board startPos;
extern board nullPos;
extern char pieceToCharUC[NUMPIECETYPES+1];
extern char pieceToCharLC[NUMPIECETYPES+1];
extern char* resultString[3];
extern U64 zobrist[2][NUMPIECETYPES][64];

#endif

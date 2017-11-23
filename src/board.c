/**
*Buzz Chess Engine
*board.c
*
*Copyright (C) 2007 Pradu Kannan.
**/

#include "board.h"
#include "bitinstructions.h"
#include "movegen.h"
#include "rand64.h"
#include "xboard.h" //needed for print(...)
#include "recog.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*Global Data*/

board startPos=
{
	C64(0xFFFF00000000FFFF), /*All Pieces*/
	{C64(0x000000000000FFFF) , C64(0xFFFF000000000000)}, /*Pieces Side*/
	{ /*Pieces[6]*/
		C64(0x00FF00000000FF00), /*Pawns*/
		C64(0x4200000000000042), /*Knights*/
		C64(0x2400000000000024), /*Bishops*/
		C64(0x8100000000000081), /*Rooks*/
		C64(0x0800000000000008), /*Queens*/
		C64(0x1000000000000010)  /*Kings*/
	},
	U64EMPTY, /*Hashkey*/
	KPNBRQKPNBRQ, /*Recognizer Signature*/
	{ /*PieceTypes[64]*/
		R, N, B, Q, K, B, N, R,
		P, P, P, P, P, P, P, P,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		P, P, P, P, P, P, P, P,
		R, N, B, Q, K, B, N, R
	},
	{E1,E8}, /*King position*/
	0, /*enpassant square*/
	BCASTLE|WCASTLE, /*castling privilages*/
	0, /*fifty move count*/
	0, /*search flags*/
	WHITE, /*side to play*/
	BLACK, /*other side*/
};

board nullPos=
{
	C64(0x0000000000000000), /*All Pieces*/
	{C64(0x0000000000000000),C64(0x0000000000000000)}, /*Pieces Side*/
	{ /*Pieces[6]*/
		C64(0x0000000000000000), /*Pawns*/
		C64(0x0000000000000000), /*Knights*/
		C64(0x0000000000000000), /*Bishops*/
		C64(0x0000000000000000), /*Rooks*/
		C64(0x0000000000000000), /*Queens*/
		C64(0x0000000000000000)  /*Kings*/
	},
	U64FULL, /*Hashkey*/
	0, /*Recognizer Signature*/
	{ /*PieceTypes[64]*/
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
		E, E, E, E, E, E, E, E,
	},
	{E1,E8}, /*King position*/
	0, /*enpassant square*/
	0, /*castling privilages*/
	0, /*fifty move count*/
	0, /*search flags*/
	WHITE, /*side to play*/
	BLACK, /*other side*/
};

char pieceToCharUC[NUMPIECETYPES+1]={'P','N','B','R','Q','K',' '};
char pieceToCharLC[NUMPIECETYPES+1]={'p','n','b','r','q','k',' '};

//Result strings
char* resultString[3]={"1-0","0-1","1/2-1/2"};

//Vertical Flip
//A square on the board flipped about the horizontal axis
#define flipV(X) ((X)^56)
//#define HK_flipV(X) (((X)&C64(0xAAAAAAAAAAAAAAAA)>>1) | ((X)&C64(0x5555555555555555)<<1))
//#define HK_SWTICH_SIDE(X) HK_FlipV(X)
//zobrist[color^1][piece][flipV(sq)] = HK_FlipV(zobrist[color][piece][sq])

//Horizontal Flip
//A square on the board flipped about the vertical axis
#define flipH(X) ((X)^7)
//#define HK_flipH(X) (((X)&C64(0xCCCCCCCCCCCCCCCC)>>2) | ((X)&C64(0x3333333333333333)<<2))
//zobrist[color^1][piece][flipH(sq)] = HK_FlipH(zobrist[color][piece][sq])

/* Private Data */
//Zobrist Keys [side][piece][square]
U64 zobrist[2][NUMPIECETYPES][64];
//zobrist Key for EP
U64 zobristEP[64];
U64 zobristCastling[2][2]; /*[side][castlingside]*/

/* Functions */

void printBB(const U64 bb)
{
	int r,c;
	for(r=8*(8-1);r>=0;r-=8)
	{
		for(c=0;c<8;c++)
			bb&(((U64)(1))<<(r+c))?print("1"):print("0");
		print("\n");
	}
	print("\n");
}

void printHex(const U64 bb)
{
	int i;
	for(i=2*sizeof(bb)-1;i>=0;i--)
		print("%X",(bb>>(4*i))&0xF);
}


void printU64(U64 bb)
{
	int numdigits=0;
	U64 bbreversed=0;
	do
	{
		bbreversed*=10;
		bbreversed+=bb%10;
		bb/=10;
		numdigits++;
	}while(bb);
	while(numdigits)
	{
		print("%u",(unsigned int)(bbreversed%10));
		bbreversed/=10;
		numdigits--;
	}
}
void printBoard(const board* pos)
{
	int i;
	if(!pos->AllPieces)
		puts("\t  \t  Illegal Position!");
	else if(pos->side==BLACK)
		puts("\t  \t    Black to Move");
	else if(pos->side==WHITE)
		puts("\t  \t    White to Move");
	print("\t  %c",PB_UL);
	for(i=0;i<7;i++)
		print("%c%c%c%c",PB_H,PB_H,PB_H,PB_HD);
	print("%c%c%c%c\n",PB_H,PB_H,PB_H,PB_UR);
	for(i=56;i>=0;i-=8)
	{
		int j;
		print("\t%c %c ",'1'+(char)ROW(i),PB_V);
		for(j=0;j<8;j++)
		{
			if(toBit(i+j)&pos->PiecesSide[BLACK])
				print("%c %c ",pieceToCharLC(pos->PieceTypes[i+j]), PB_V);
			else
				print("%c %c ",pieceToCharUC(pos->PieceTypes[i+j]), PB_V);
		}
		print("\n");
		if(i)
		{
			print("\t  %c",PB_VR);
			for(j=0;j<7;j++)
				print("%c%c%c%c",PB_H,PB_H,PB_H,PB_CRS);
			print("%c%c%c%c\n",PB_H,PB_H,PB_H,PB_VL);
		}
	}
	print("\t  %c",PB_LL);
	for(i=0;i<7;i++)
		print("%c%c%c%c",PB_H,PB_H,PB_H,PB_HU);
	print("%c%c%c%c\n",PB_H,PB_H,PB_H,PB_LR);
	
	print("\t   ");
	for(i=0;i<8;i++)
		print(" %c  ",'a'+i);
	print("\n");
}

void printDebugBoard(const board* pos)
{
	//Prints all board data in bitboards
	print("All Pieces\n");
	printBB(pos->AllPieces);
	print("White\n");
	printBB(pos->PiecesSide[WHITE]);
	print("Black\n");
	printBB(pos->PiecesSide[BLACK]);
	
	print("Pawns\n");
	printBB(pos->Pieces[P]);
	print("Knights\n");
	printBB(pos->Pieces[N]);
	print("Bishops\n");
	printBB(pos->Pieces[B]);
	print("Rooks\n");
	printBB(pos->Pieces[R]);
	print("Queens\n");
	printBB(pos->Pieces[Q]);
	print("Kings\n");
	printBB(pos->Pieces[K]);
}

/*returns 1 if the board position is legal and uncorrupted*/
bool boardIsOk(const board* pos)
{
	
	//check for corrupted board
	if(!pos->AllPieces) return false;
	if(pos->AllPieces!=(pos->PiecesSide[WHITE]|pos->PiecesSide[BLACK])) return false;
	
	//make sure the occupancy and the pieces coresspond
	{
		U64 temp;
		temp=pos->Pieces[K]|pos->Pieces[Q]|pos->Pieces[R]|
		pos->Pieces[B]|pos->Pieces[N]|pos->Pieces[P];
		if(temp!=pos->AllPieces) return false;
	}

	//two pieces cannot be on the same square
	{
		U64 temp=U64EMPTY;
		int i;
		int j;
		for(i=0;i<NUMPIECETYPES-1;i++)
			for(j=i+1;j<NUMPIECETYPES;j++)
				temp|=pos->Pieces[i]&pos->Pieces[j];
		if(temp) return false;
	}


	//Checking that the piece-array coressponds to the
	//bitboards
	{
		int i;
		U64 j;
		for(i=0,j=1; j ; i++, j<<=1)
		{
			switch(pos->PieceTypes[i])
			{
			case K:
				if(!(pos->Pieces[K]&j)) return false;
				break;
			case Q:
				if(!(pos->Pieces[Q]&j)) return false;
				break;
			case R:
				if(!(pos->Pieces[R]&j)) return false;
				break;
			case B:
				if(!(pos->Pieces[B]&j)) return false;
				break;
			case N:
				if(!(pos->Pieces[N]&j)) return false;
				break;
			case P:
				if(!(pos->Pieces[P]&j)) return false;
				break;
			case E:
				if(pos->AllPieces&j) return false;
				break;
			default: return false;
			}
		}
	}

	//Make sure the EP square is correct
	if(pos->EP)
	{
		if(pos->side==BLACK)
		{
			if(toBit(pos->EP)&C64(0xFFFFFFFFFF00FFFF)) return false;
		}
		else
		{
			if(toBit(pos->EP)&C64(0xFFFF00FFFFFFFFFF)) return false;
		}
	}

	//If there are incorrect number of kings for each side - the position is illegal
	if(!piecesWHITE(*pos,K))
		return false;
	if(!piecesBLACK(*pos,K))
		return false;
	if(piecesWHITE(*pos,K)&(piecesWHITE(*pos,K)-1))
		return false;
	if(piecesBLACK(*pos,K)&(piecesBLACK(*pos,K)-1))
		return false;

	//If the opponent side is in check - the position is illegal
	if(inCheck(*pos,pos->xside))
		return false;

	//If there are pawns on the 8th or 1st rank - the position is illegal
	if(pos->Pieces[P]&C64(0x00000000000000FF))
		return false;
	if(pos->Pieces[P]&C64(0xFF00000000000000))
		return false;

	//check if the recognizer signature is ok
	if((unsigned int)genrsig(*pos)!=pos->recogsig);
	return true;
}

/*returns -1 if the coordiate is invalid*/
int coordToSquare(const char* coord)
{
	int i;
	if(!(coord[1]>='1' && coord[1]<='8'))
		return -1;
	i=8*(coord[1]-'1');
	if(coord[0]>='a' && coord[0]<='h')
		return i+(coord[0]-'a');
	else if(coord[0]>='A' && coord[0]<='H')
		return i+(coord[0]-'A');
	return -1;
}

unsigned char charToPiece(char c)
{
	switch(c)
	{
		case 'p':case'P':return P;
		case 'n':case'N':return N;
		case 'b':case'B':return B;
		case 'r':case'R':return R;
		case 'q':case'Q':return Q;
		case 'k':case'K':return K;
		default: return E;
	}
}

void moveToCoords(const board* pos, const move m, char* string)
{
	int from=extractFrom(m);
	int to=extractTo(m);
	int piece=extractPiece(m);
	int promotion=extractPromotion(m);

	if(m==NULLMOVE)
	{
		string[0]='N';
		string[1]='u';
		string[2]='l';
		string[3]='l';
		string[4]='\0';
	}

	if(m==KingsideCastle)
	{
		if(pos->side) //If Black
		{
			from=E8;
			to=G8;
		}
		else
		{
			from=E1;
			to=G1;
		}
	}
	else if(m==QueensideCastle)
	{
		if(pos->side) //If Black
		{
			from=E8;
			to=C8;
		}
		else
		{
			from=E1;
			to=C1;
		}
	}

	string[0]='a'+(char)COL(from);
	string[1]='1'+(char)ROW(from);
	string[2]='a'+(char)COL(to);
	string[3]='1'+(char)ROW(to);
	string[4]='\0';
	if(piece==P && promotion!=E)
	{
		string[4]=pieceToCharLC(promotion);
		string[5]='\0';
	}
}



void moveToSAN(const board* pos, const move m, char* string)
{
	unsigned int piece=extractPiece(m), from=extractFrom(m),
		to=extractTo(m), capture=extractCapture(m);
	int i=0;
	if(m==NULLMOVE)
	{
		strcpy(string,"Null");
		return;
	}

	if(piece==E) //castling
	{
		if(m==KingsideCastle)
			strcpy(string,"O-O");
		else
			strcpy(string,"O-O-O");
		return;
	}
	if(piece==P)
	{
		int promotion=extractPromotion(m);
		if(capture!=E)
		{
			string[i++]='a'+(char)COL(from);
			string[i++]='x';
		}
		string[i++]='a'+(char)COL(to);
		string[i++]='1'+(char)ROW(to);
		if(promotion!=E)
		{
			string[i++]='=';
			string[i++]=pieceToChar(promotion);
		}
		string[i++]='\0';
		return;
	}
	//regular move
	string[i++]=pieceToChar(piece);
	//disambiguating moves
	if(piece!=K)
	{
		U64 pieceboard=pos->PiecesSide[pos->side]&~toBit(from);
		switch(piece)
		{
		case N: pieceboard&=attacksToN(*pos,to); break;
		case B: pieceboard&=attacksToB(*pos,to); break;
		case R: pieceboard&=attacksToR(*pos,to); break;
		case Q: pieceboard&=attacksToQ(*pos,to); break;
		}
		if(pieceboard)
		{
			U64 pinned=pieceboard&possiblePinned(pos,pos->side);
			//remove pinned pieces from pieceboard
			while(pinned)
			{
				int tempSquare;
				GetBitAndClear(pinned,tempSquare);
				if(!(lineOf(pos->KingPos[pos->side],tempSquare)&toBit(to)))
					pieceboard&=~toBit(tempSquare);
			}
			if(pieceboard)
			{
				bool canDistinguishRank=true;
				bool canDistinguishFile=true;
				while(pieceboard)
				{
					unsigned int tempSquare;
					GetBitAndClear(pieceboard,tempSquare);
					if(ROW(tempSquare)==ROW(from)) canDistinguishRank=false;
					if(COL(tempSquare)==COL(from)) canDistinguishFile=false;
				}
				if(canDistinguishFile)
					string[i++]='a'+(char)COL(from);
				else if(canDistinguishRank)
					string[i++]='1'+(char)ROW(from);
				else
				{
					string[i++]='a'+(char)COL(from);
					string[i++]='1'+(char)ROW(from);
				}
			}
		}
	}
	if(capture!=E) string[i++]='x';
	string[i++]='a'+(char)COL(to);
	string[i++]='1'+(char)ROW(to);
	string[i++]='\0';
}

//same as move to SAN but does not check pins when disambiguating moves
void moveToCBSAN(const board* pos, const move m, char* string)
{
	unsigned int piece=extractPiece(m), from=extractFrom(m),
		to=extractTo(m), capture=extractCapture(m);
	int i=0;
	if(m==NULLMOVE)
	{
		strcpy(string,"Null");
		return;
	}

	if(piece==E) //castling
	{
		if(m==KingsideCastle)
			strcpy(string,"O-O");
		else
			strcpy(string,"O-O-O");
		return;
	}
	if(piece==P)
	{
		int promotion=extractPromotion(m);
		if(capture!=E)
		{
			string[i++]='a'+(char)COL(from);
			string[i++]='x';
		}
		string[i++]='a'+(char)COL(to);
		string[i++]='1'+(char)ROW(to);
		if(promotion!=E)
		{
			string[i++]='=';
			string[i++]=pieceToChar(promotion);
		}
		string[i++]='\0';
		return;
	}
	//regular move
	string[i++]=pieceToChar(piece);
	//disambiguating moves
	if(piece!=K)
	{
		U64 pieceboard=pos->PiecesSide[pos->side]&~toBit(from);
		switch(piece)
		{
		case N: pieceboard&=attacksToN(*pos,to); break;
		case B: pieceboard&=attacksToB(*pos,to); break;
		case R: pieceboard&=attacksToR(*pos,to); break;
		case Q: pieceboard&=attacksToQ(*pos,to); break;
		}
		if(pieceboard)
		{

			bool canDistinguishRank=true;
			bool canDistinguishFile=true;
			while(pieceboard)
			{
				unsigned int tempSquare;
				GetBitAndClear(pieceboard,tempSquare);
				if(ROW(tempSquare)==ROW(from)) canDistinguishRank=false;
				if(COL(tempSquare)==COL(from)) canDistinguishFile=false;
			}
			if(canDistinguishFile)
				string[i++]='a'+(char)COL(from);
			else if(canDistinguishRank)
				string[i++]='1'+(char)ROW(from);
			else
			{
				string[i++]='a'+(char)COL(from);
				string[i++]='1'+(char)ROW(from);
			}
		}
	}
	if(capture!=E) string[i++]='x';
	string[i++]='a'+(char)COL(to);
	string[i++]='1'+(char)ROW(to);
	string[i++]='\0';
}

//same as move to SAN but it always specifies the from square for pieces; eg Nb1c3
void moveToLSAN(const board* pos, const move m, char* string)
{
	unsigned int piece=extractPiece(m), from=extractFrom(m),
		to=extractTo(m), capture=extractCapture(m);
	int i=0;
	if(m==NULLMOVE)
	{
		strcpy(string,"Null");
		return;
	}

	if(piece==E) //castling
	{
		if(m==KingsideCastle)
			strcpy(string,"O-O");
		else
			strcpy(string,"O-O-O");
		return;
	}
	if(piece==P)
	{
		int promotion=extractPromotion(m);
		if(capture!=E)
		{
			string[i++]='a'+(char)COL(from);
			string[i++]='x';
		}
		string[i++]='a'+(char)COL(to);
		string[i++]='1'+(char)ROW(to);
		if(promotion!=E)
		{
			string[i++]='=';
			string[i++]=pieceToChar(promotion);
		}
		string[i++]='\0';
		return;
	}
	//regular move
	string[i++]=pieceToChar(piece);
	//from square is always used
	string[i++]='a'+(char)COL(from);
	string[i++]='1'+(char)ROW(from);
	
	if(capture!=E) string[i++]='x';
	string[i++]='a'+(char)COL(to);
	string[i++]='1'+(char)ROW(to);
	string[i++]='\0';
}

//returns NULLMOVE if the move is not a coordiate notation move
move coordsToMove(const board* pos, const char* coord)
{
	int from, to;
	move piece, capture, promotion=0, enpassant=0;
	move ret;

	from=coordToSquare(coord);
	to=coordToSquare(coord+2);
	if(from<0 || to<0)
		return NULLMOVE;
	ret=encodeFrom((move)from)|encodeTo((move)(to));
	piece=pos->PieceTypes[from];
	if(piece==E)
		return NULLMOVE;
	if(piece==K)
	{
		if(from-to==2)
			return QueensideCastle;
		else if(to-from==2)
			return KingsideCastle;
	}
	ret|=encodePiece(piece);
	capture=pos->PieceTypes[to];
	ret|=encodePromotion(E);
	if(piece==P)
	{
		if(ROW(to)==0 || ROW(to)==7)
		{
			promotion=charToPiece(coord[4]);
			ret&=~encodePromotion(E);
			if(promotion!=N || promotion!=B || promotion!=R || promotion!=Q)
				return NULLMOVE;
			ret|=encodePromotion(promotion);
		}
		else if(pos->EP && to==pos->EP)
		{
			capture=P;
			if(pos->side) //BLACK
				enpassant=(pos->EP)-8;
			else
				enpassant=(pos->EP)+8;
			ret|=encodeEP(enpassant);
		}
	}
	ret|=encodeCapture(capture);
	return ret;
}

//converts a string to a legal move, returns NULLMOVE if the string is not recognized
move stringToMove(const board* pos, const char* string)
{
	unsigned int i;
	moveList ml;
	genMoves(pos,&ml);
	for(i=0;i<ml.moveCount;i++)
	{
		coordString coords;
		sanString san;
		moveToCoords(pos,ml.moves[i].m,coords);
		if(!strncmp(coords,string,strlen(coords)))
			return ml.moves[i].m;
		moveToSAN(pos,ml.moves[i].m,san);
		if(!strncmp(san,string,strlen(san)))
		{
			if(strcmp(san,"O-O") || strncmp(string,"O-O-O",5))
				return ml.moves[i].m;
		}
		moveToCBSAN(pos,ml.moves[i].m,san);
		if(!strncmp(san,string,strlen(san)))
		{
			if(strcmp(san,"O-O") || strncmp(string,"O-O-O",5))
				return ml.moves[i].m;
		}
		moveToLSAN(pos,ml.moves[i].m,san);
		if(!strncmp(san,string,strlen(san)))
		{
			if(strcmp(san,"O-O") || strncmp(string,"O-O-O",5))
				return ml.moves[i].m;
		}
	}
	return NULLMOVE;
}
void generateZobrist()
{
	int i,j,k;
	for(k=0;k<64;k++)
	{
		zobristEP[k]=rand64();
		#ifdef REVERSE_ZOBRIST_COLORS
		for(i=1;i>=0;i--)
		#else
		for(i=0;i<2;i++)
		#endif	
			for(j=0;j<NUMPIECETYPES;j++)
				zobrist[i][j][k]=rand64();
	}
	#ifdef REVERSE_ZOBRIST_COLORS
	for(i=1;i>=0;i--)
	#else
	for(i=0;i<2;i++)
	#endif	
		for(j=0;j<2;j++)
			zobristCastling[i][j]=rand64();
}

//hashkey generated from board position
U64 staticHashKey(const board* pos)
{
	U64 key = U64EMPTY;
	int piece, side;
	

	//Add the pieces
	for(side=WHITE;side<=BLACK;side++)
		for(piece=P;piece<=K;piece++)
		{
			U64 temp = pos->Pieces[piece]&pos->PiecesSide[side];
			while(temp)
			{
				int sq;
				GetBitAndClear(temp,sq);
				key^=zobrist[side][piece][sq];
			}
		}

	//Castling  privilages
	if(pos->castling&BK) key^=zobristCastling[BLACK][KINGSIDE];
	if(pos->castling&BQ) key^=zobristCastling[BLACK][QUEENSIDE];
	if(pos->castling&WK) key^=zobristCastling[WHITE][KINGSIDE];
	if(pos->castling&WQ) key^=zobristCastling[WHITE][QUEENSIDE];

	//Enpassant
	if(pos->EP) key^=zobristEP[pos->EP];

	//Side to play
	if(pos->side==BLACK) key=ZOBRIST_CHANGE_SIDE(key);
}

void makemove(board* pos, const move m)
{
	const unsigned int to=extractTo(m), piece=extractPiece(m), capture=extractCapture(m),
			from=extractFrom(m);
	U64 temp=toBit(from)^toBit(to);
	//fifty move rule
	pos->fifty++;

	if(pos->EP) //clear ep flag
	{
		pos->hashkey^=zobristEP[pos->EP];
		pos->EP=0;
	}
	
	//clear search flags
	clearSearchFlags(pos->SF);

	if(m==NULLMOVE)
	{
		pos->hashkey=ZOBRIST_CHANGE_SIDE(pos->hashkey);
		pos->side=!pos->side;
		pos->xside=!pos->xside;
		pos->SF|=encodeSFNull;
		return;
	}

	//Castling specific code
	if(pos->castling)
	{
		if(piece==E) //castling
		{
			if(pos->side) //BLACK
			{
				if(pos->castling&BK)
				{
					pos->castling^=BK;
					pos->hashkey^=zobristCastling[BLACK][KINGSIDE];
				}
				if(pos->castling&BQ)
				{
					pos->castling^=BQ;
					pos->hashkey^=zobristCastling[BLACK][QUEENSIDE];
				}
				if(m==KingsideCastle)
				{
					pos->hashkey^=zobrist[BLACK][K][E8]^zobrist[BLACK][K][G8];
					pos->hashkey^=zobrist[BLACK][R][H8]^zobrist[BLACK][R][F8];
					pos->Pieces[K]^=toBit(E8)|toBit(G8);
					pos->Pieces[R]^=toBit(H8)|toBit(F8);
					pos->AllPieces^=toBit(E8)|toBit(G8)|toBit(H8)|toBit(F8);
					pos->PiecesSide[BLACK]^=toBit(E8)|toBit(G8)|toBit(H8)|toBit(F8);
					pos->PieceTypes[E8]=E;
					pos->PieceTypes[G8]=K;
					pos->PieceTypes[H8]=E;
					pos->PieceTypes[F8]=R;
					pos->KingPos[BLACK]=G8;
				}
				else //QueensideCastle
				{
					pos->hashkey^=zobrist[BLACK][K][E8]^zobrist[BLACK][K][C8];
					pos->hashkey^=zobrist[BLACK][R][A8]^zobrist[BLACK][R][D8];
					pos->Pieces[K]^=toBit(E8)|toBit(C8);
					pos->Pieces[R]^=toBit(A8)|toBit(D8);
					pos->AllPieces^=toBit(E8)|toBit(C8)|toBit(A8)|toBit(D8);
					pos->PiecesSide[BLACK]^=toBit(E8)|toBit(C8)|toBit(A8)|toBit(D8);
					pos->PieceTypes[E8]=E;
					pos->PieceTypes[C8]=K;
					pos->PieceTypes[A8]=E;
					pos->PieceTypes[D8]=R;
					pos->KingPos[BLACK]=C8;
				}
			}
			else //WHITE
			{
				if(pos->castling&WK)
				{
					pos->castling^=WK;
					pos->hashkey^=zobristCastling[WHITE][KINGSIDE];
				}
				if(pos->castling&WQ)
				{
					pos->castling^=WQ;
					pos->hashkey^=zobristCastling[WHITE][QUEENSIDE];
				}
				if(m==KingsideCastle)
				{
					pos->hashkey^=zobrist[WHITE][K][E1]^zobrist[WHITE][K][G1];
					pos->hashkey^=zobrist[WHITE][R][H1]^zobrist[WHITE][R][F1];
					pos->Pieces[K]^=toBit(E1)|toBit(G1);
					pos->Pieces[R]^=toBit(H1)|toBit(F1);
					pos->AllPieces^=toBit(E1)|toBit(G1)|toBit(H1)|toBit(F1);
					pos->PiecesSide[WHITE]^=toBit(E1)|toBit(G1)|toBit(H1)|toBit(F1);
					pos->PieceTypes[E1]=E;
					pos->PieceTypes[G1]=K;
					pos->PieceTypes[H1]=E;
					pos->PieceTypes[F1]=R;
					pos->KingPos[WHITE]=G1;
				}
				else //QueensideCastle
				{
					pos->hashkey^=zobrist[WHITE][K][E1]^zobrist[WHITE][K][C1];
					pos->hashkey^=zobrist[WHITE][R][A1]^zobrist[WHITE][R][D1];
					pos->Pieces[K]^=toBit(E1)|toBit(C1);
					pos->Pieces[R]^=toBit(A1)|toBit(D1);
					pos->AllPieces^=toBit(E1)|toBit(C1)|toBit(A1)|toBit(D1);
					pos->PiecesSide[WHITE]^=toBit(E1)|toBit(C1)|toBit(A1)|toBit(D1);
					pos->PieceTypes[E1]=E;
					pos->PieceTypes[C1]=K;
					pos->PieceTypes[A1]=E;
					pos->PieceTypes[D1]=R;
					pos->KingPos[WHITE]=C1;
				}
			}
			pos->hashkey=~pos->hashkey;
			pos->side=pos->xside;
			pos->xside=!pos->xside;
			return;
		}
		if(piece==R) //adjust castling if rook moved
		{
			if(pos->side) //BLACK
			{
				if(from==A8 && pos->castling&BQ)
				{
					pos->castling&=~BQ;
					pos->hashkey^=zobristCastling[BLACK][QUEENSIDE];
				}
				else if(from==H8 && pos->castling&BK)
				{
					pos->castling&=~BK;
					pos->hashkey^=zobristCastling[BLACK][KINGSIDE];
				}
			}
			else //WHITE
			{
				if(from==A1 && pos->castling&WQ)
				{
					pos->castling&=~WQ;
					pos->hashkey^=zobristCastling[WHITE][QUEENSIDE];
				}
				else if(from==H1 && pos->castling&WK)
				{
					pos->castling&=~WK;
					pos->hashkey^=zobristCastling[WHITE][KINGSIDE];
				}
			}
		}
		else if(piece==K) //adjust castling if king moved
		{
			if(pos->side) //Black
			{
				if(pos->castling&BK)
				{
					pos->castling^=BK;
					pos->hashkey^=zobristCastling[BLACK][KINGSIDE];
				}
				if(pos->castling&BQ)
				{
					pos->castling^=BQ;
					pos->hashkey^=zobristCastling[BLACK][QUEENSIDE];
				}
			}
			else
			{
				if(pos->castling&WK)
				{
					pos->castling^=WK;
					pos->hashkey^=zobristCastling[WHITE][KINGSIDE];
				}
				if(pos->castling&WQ)
				{
					pos->castling^=WQ;
					pos->hashkey^=zobristCastling[WHITE][QUEENSIDE];
				}
			}
		}
		if(capture==R) //adjust castling if rook got captured
		{
			if(pos->xside) //If BLACK got captured
			{

				if(to==A8 && pos->castling&BQ)
				{
					pos->castling&=~BQ;
					pos->hashkey^=zobristCastling[BLACK][QUEENSIDE];
				}
				else if(to==H8 && pos->castling&BK)
				{
					pos->castling&=~BK;
					pos->hashkey^=zobristCastling[BLACK][KINGSIDE];
				}
			}
			else
			{
				if(to==A1 && pos->castling&WQ)
				{
					pos->castling&=~WQ;
					pos->hashkey^=zobristCastling[WHITE][QUEENSIDE];
				}
				else if(to==H1 && pos->castling&WK)
				{
					pos->castling&=~WK;
					pos->hashkey^=zobristCastling[WHITE][KINGSIDE];
				}
			}
		}
	}

	if(piece==K)
		pos->KingPos[pos->side]=(unsigned char)to;

	//Generic code
	if(capture!=E)
	{
		pos->fifty=0;
		pos->hashkey^=zobrist[pos->xside][capture][to]^zobrist[pos->side][piece][from]^zobrist[pos->side][piece][to];
		pos->PiecesSide[pos->xside]^=toBit(to);
		pos->Pieces[piece]^=temp;
		pos->Pieces[capture]^=toBit(to);
		pos->AllPieces^=toBit(from);
		pos->PiecesSide[pos->side]^=temp;
		pos->PieceTypes[from]=E;
		pos->PieceTypes[to]=(unsigned char)piece;

		//Update recognizer
		updateRecogSig(pos->recogsig,pos->xside,capture,pos->Pieces[capture]&pos->PiecesSide[pos->xside]);
	}
	else
	{
		pos->hashkey^=zobrist[pos->side][piece][from]^zobrist[pos->side][piece][to];
		pos->Pieces[piece]^=temp;
		pos->AllPieces^=temp;
		pos->PiecesSide[pos->side]^=temp;
		pos->PieceTypes[from]=E;
		pos->PieceTypes[to]=(unsigned char)piece;
	}

	//Special code for pawns
	if(piece==P)
	{
		unsigned int enpassant=extractEP(m);
		unsigned int promotion=extractPromotion(m);
		pos->fifty=0;
		if(to==from+16  && Pcaps(from+8,WHITE)&piecesBLACK(*pos,P))
		{
			pos->EP=(unsigned char)(from+8);
			pos->hashkey^=zobristEP[pos->EP];
		}
		else if(from==to+16 && Pcaps(to+8,BLACK)&piecesWHITE(*pos,P))
		{
			pos->EP=(unsigned char)(to+8);
			pos->hashkey^=zobristEP[pos->EP];
		}
		else if(enpassant!=0) //Remove the pawn
		{
			pos->hashkey^=zobrist[pos->xside][P][enpassant];
			pos->Pieces[P]^=toBit(enpassant)|toBit(to);
			pos->AllPieces^=toBit(enpassant)|toBit(to);
			pos->PiecesSide[pos->xside]^=toBit(enpassant)|toBit(to);
			pos->PieceTypes[enpassant]=E;
		}
		else if(promotion!=E)
		{
			pos->hashkey^=zobrist[pos->side][P][to]^zobrist[pos->side][promotion][to];
			pos->PieceTypes[to]=(unsigned char)promotion;
			pos->Pieces[P]^=toBit(to);
			pos->Pieces[promotion]^=toBit(to);
			updateRecogSig(pos->recogsig,pos->side,P,pos->Pieces[P]&pos->PiecesSide[pos->side]);
			updateRecogSig(pos->recogsig,pos->side,promotion,pos->Pieces[promotion]&pos->PiecesSide[pos->side]);
		}
	}
	pos->hashkey=ZOBRIST_CHANGE_SIDE(pos->hashkey);
	pos->side=!pos->side;
	pos->xside=!pos->xside;
	assert(boardIsOk(pos));
}

//draw by rule
bool gameDecidableDraw(board* pos)
{
	int numpieces=popcnt(pos->AllPieces);
	switch(numpieces)
	{
	case 0: return false; //illegal
	case 1: return false; //illegal
	case 2: return true;  //two kings
	case 3:
		if(pos->Pieces[B] || pos->Pieces[N])
			return true;
		else return false;
	default:
		return 0;
	}
}

//The number or kingmoves it takes
int distance(int sq1, int sq2)
{
	int cdist = COL(sq1)-COL(sq2);
	int rdist = ROW(sq1)-ROW(sq2);
	
	assert(sq1<64);
	assert(sq2<64);

	if(cdist<0) cdist=-cdist;
	if(rdist<0) rdist=-rdist;
	if(cdist>rdist) return cdist; else return rdist;
}

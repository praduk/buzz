/**
*Buzz Chess Engine
*recog.c
*
*Interior-node recognizers.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "recog.h"
#include "board.h"
#include "bitinstructions.h"
#include "eval.h"
#include "movegen.h"

//drawn recognitions are exact, otherwise inexact
bool recogDraw(const board* pos)
{
	#ifdef USE_RECOGDRAW
	int men=popcnt(pos->AllPieces);
	
	//recognition by number of men
	switch(men)
	{
	case 0:
	case 1: return false; //illegal
	case 2: return true; //two kings
	case 3:
		switch(pos->recogsig)
		{
		//Lone bishops and knights are draws
		case KBK: case KKB: case KNK: case KKN: return true;
		//KPK recognizers are adapted from Fruit 2.1
		case KPK:
			{
				int pawnsq = toIndex(pos->Pieces[P]);
				
				//If opponent king is a square in front of the pawn
				//and the pawn is on rank 6 or lower, then draw
				if(pos->KingPos[BLACK]==(pawnsq+8))
				{
					if(ROW(pawnsq)<=RANK6) return true;
					//stalemate or KK game coming
					assert(ROW(pawnsq)==RANK7);
					if(pos->xside) //white to move
					{
						if(Pcaps(pawnsq,BLACK)&toBit(pos->KingPos[WHITE])) return true;
					}
					else //black to move
					{
						if((~Pcaps(pawnsq,BLACK))&toBit(pos->KingPos[WHITE])) return true;
					}
				}

				//If opponent king is two squares in front of the pawn
				else if(pos->KingPos[BLACK]==(pawnsq+16))
				{
					//if pawn is on rank 5 or lower, then draw
					if(ROW(pawnsq)<=RANK5) return true;
					//pawn is now on RANK6
					assert(ROW(pawnsq)==RANK6);
					if(pos->side) return true; //if black to move then draw
					//If the king is not beside the pawn then draw
					if((~(Pcaps(pawnsq,WHITE)>>1))&toBit(pos->KingPos[WHITE])) return true;
				}

				//Testing opposition
				//If white king is beside the pawn
				else if((Pcaps(pawnsq,WHITE)>>1)&toBit(pos->KingPos[WHITE]))
				{
					if(pos->KingPos[WHITE]+16 == pos->KingPos[BLACK] && pos->xside) return true;
				}
				//If the king is in front of the pawn
				else if((Pcaps(pawnsq,WHITE)|Pmoves(pawnsq,WHITE)) & pos->KingPos[WHITE])
				{
					//If the pawn's rank is on rank 4 or lower
					if(ROW(pawnsq)<=RANK4)
					{
						//If the black king is in front of the white king and white to move
						if(pos->KingPos[WHITE]+16 == pos->KingPos[BLACK] && pos->xside)
							return true;
					}
				}

				//For pawns on A file or H file
				if(COL(pawnsq)==FILEA)
				{
					//If the black king can reach square A8 then it's drawn
					if((Kmoves(pos->KingPos[BLACK])|toBit(pos->KingPos[BLACK]))&toBit(A8)) return true;
					if(COL(pos->KingPos[WHITE])==FILEA)
					{
						//to take care of double pawn moves
						int pawnrank=ROW(pawnsq);
						if(pawnrank==RANK2) pawnrank++;

						if(COL(pos->KingPos[BLACK])==FILEC && ROW(pos->KingPos[BLACK])>pawnrank)
							return true;
					}
				}
				if(COL(pawnsq)==FILEH)
				{
					//If the black king can reach square H8 then it's drawn
					if((Kmoves(pos->KingPos[BLACK])|toBit(pos->KingPos[BLACK]))&toBit(H8)) return true;
					if(COL(pos->KingPos[WHITE])==FILEH)
					{
						//to take care of double pawn moves
						int pawnrank=ROW(pawnsq);
						if(pawnrank==RANK2) pawnrank++;

						if(COL(pos->KingPos[BLACK])==FILEF && ROW(pos->KingPos[BLACK])>pawnrank)
							return true;
					}
				}
				return false;
			}
		case KKP: //KPK for black
			{
				int pawnsq = toIndex(pos->Pieces[P]);
				
				//If opponent king is a square in front of the pawn
				//and the pawn is on rank 3 or higher, then draw
				if(pos->KingPos[WHITE]==(pawnsq-8))
				{
					if(ROW(pawnsq)>=RANK3) return true;
					//stalemate or KK game coming
					assert(ROW(pawnsq)==RANK2);
					if(pos->side) //black to move
					{
						if(Pcaps(pawnsq,WHITE)&toBit(pos->KingPos[BLACK])) return true;
					}
					else //white to move
					{
						if((~Pcaps(pawnsq,WHITE))&toBit(pos->KingPos[BLACK])) return true;
					}
				}

				//If opponent king is two squares in front of the pawn
				else if(pos->KingPos[WHITE]==(pawnsq-16))
				{
					//if pawn is on rank 4 or higher, then draw
					if(ROW(pawnsq)<=RANK4) return true;
					//pawn is now on RANK3
					assert(ROW(pawnsq)==RANK3);
					if(pos->xside) return true; //if white to move then draw
					//If the king is not beside the pawn then draw
					if((~(Pcaps(pawnsq,BLACK)<<1))&toBit(pos->KingPos[BLACK])) return true;
				}

				//Testing opposition
				//If black king is beside the pawn
				else if((Pcaps(pawnsq,BLACK)<<1)&toBit(pos->KingPos[BLACK]))
				{
					if(pos->KingPos[BLACK]-16 == pos->KingPos[WHITE] && pos->side) return true;
				}
				//If the king is in front of the pawn
				else if((Pcaps(pawnsq,BLACK)|Pmoves(pawnsq,BLACK)) & pos->KingPos[BLACK])
				{
					//If the pawn's rank is on rank 5 or higher
					if(ROW(pawnsq)>=RANK5)
					{
						//If the white king is in front of the black king and black to move
						if(pos->KingPos[BLACK]-16 == pos->KingPos[WHITE] && pos->side)
							return true;
					}
				}

				//For pawns on A file or H file
				if(COL(pawnsq)==FILEA)
				{
					//If the white king can reach square A1 then it's drawn
					if((Kmoves(pos->KingPos[WHITE])|toBit(pos->KingPos[WHITE]))&toBit(A1)) return true;
					if(COL(pos->KingPos[BLACK])==FILEA)
					{
						//to take care of double pawn moves
						int pawnrank=ROW(pawnsq);
						if(pawnrank==RANK7) pawnrank--;

						if(COL(pos->KingPos[WHITE])==FILEC && ROW(pos->KingPos[WHITE])<pawnrank)
							return true;
					}
				}
				if(COL(pawnsq)==FILEH)
				{
					//If the white king can reach square H1 then it's drawn
					if((Kmoves(pos->KingPos[WHITE])|toBit(pos->KingPos[WHITE]))&toBit(H1)) return true;
					if(COL(pos->KingPos[BLACK])==FILEH)
					{
						//to take care of double pawn moves
						int pawnrank=ROW(pawnsq);
						if(pawnrank==RANK7) pawnrank--;

						if(COL(pos->KingPos[WHITE])==FILEF && ROW(pos->KingPos[WHITE])<pawnrank)
							return true;
					}
				}
				return false;
			}
		default: return false;
		}
	case 4:
		switch(pos->recogsig)
		{
		case KPBK:
			{
				int pawnsq=toIndex(pos->Pieces[P]);
				if(COL(pawnsq)==FILEA && (Kmoves(pos->KingPos[BLACK])|toBit(pos->KingPos[BLACK]))&toBit(A8)
					&& BLACKSQUARES&pos->Pieces[B])
					return true;
				if(COL(pawnsq)==FILEH && (Kmoves(pos->KingPos[BLACK])|toBit(pos->KingPos[BLACK]))&toBit(H8)
					&& WHITESQUARES&pos->Pieces[B])
					return true;
				return false;
			}
		case KKPB:
			{
				int pawnsq=toIndex(pos->Pieces[P]);
				if(COL(pawnsq)==FILEA && (Kmoves(pos->KingPos[WHITE])|toBit(pos->KingPos[WHITE]))&toBit(A1)
					&& WHITESQUARES&pos->Pieces[B])
					return true;
				if(COL(pawnsq)==FILEH && (Kmoves(pos->KingPos[WHITE])|toBit(pos->KingPos[WHITE]))&toBit(H1)
					&& BLACKSQUARES&pos->Pieces[B])
					return true;
				return false;
			}
		case KBK: case KBKB: case KKB:
			if( !(WHITESQUARES&pos->Pieces[B]) || !(BLACKSQUARES&pos->Pieces[B]) )
				return true;
		default: return false;
		}
	case 5:
	case 6:
	case 7:
	case 8:
		switch(pos->recogsig)
		{
			case KBK: case KBKB: case KKB:
			if( !(WHITESQUARES&pos->Pieces[B]) || !(BLACKSQUARES&pos->Pieces[B]) )
				return true;
		default: return false;
		}
	default:
		return false;
	}
	#else
		return false;
	#endif //USE_RECOGDRAW
}

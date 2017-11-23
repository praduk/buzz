/**
*Buzz Chess Engine
*pgntable.h
*
*Reads a pgn file and writes to a table.
*
*Experimental Module
*
*Copyright (C) 2008 Pradu Kannan. All rights reserved.
**/

#define PGNTABLE_PLYS 30

#include "board.h"
#include "pgn.h"
#include "eval.h"

struct _pgnTable
{
	int wwins;
	int bwins;
	int draws;
}pgnTable[PGNTABLE_PLYS][9][9][3][3][3][3][3][3][2][2];
//[ply][WP][BP][WN][BN][WB][BB][WR][BR][WQ][BQ];

struct _pgnTablePMT
{
	int score;
	int gamesx2;
}pgnTablePMT[1000];

void pgnTable_call()
{
	PGNgame g;
	FILE* f;
	int numgames=0;

	f = fopen("games.pgn","rt");

	if(f==NULL) return;
	{
		int wp, wn, wb, wr, wq, bp, bn, bb, br, bq;
		int plys;
		for (plys=0;plys<PGNTABLE_PLYS;plys++)
		for (wp=0;wp<=8;wp++) {
		for (wn=0;wn<=2;wn++) {
		for (wb=0;wb<=2;wb++) {
		for (wr=0;wr<=2;wr++) {
		for (wq=0;wq<=1;wq++) {
		for (bp=0;bp<=8;bp++) {
		for (bn=0;bn<=2;bn++) {
		for (bb=0;bb<=2;bb++) {
		for (br=0;br<=2;br++) {
		for (bq=0;bq<=1;bq++) {
			struct _pgnTable* entry=
				&pgnTable[plys][wp][bp][wn][bn][wb][bb][wr][br][wq][bq];
			entry->bwins=0;
			entry->draws=0;
			entry->wwins=0;
		}}}}}}}}}}
	}
	printf("gets here\n");
	

	//Get Data
	while(getNextPGNGame(f,&g))
	{
		int i;
		int lastCap = 0;
		board pos = startPos;
		numgames++;
		for(i=0;i<g.nummoves;i++)
		{
			makemove(&pos,g.moves[i]);
			if(extractCapture(g.moves[i])!=E || (extractPiece(g.moves[i])==P && extractPromotion(g.moves[i])!=E))
			{
				lastCap = i;
				continue;
			}
			if(i-lastCap < PGNTABLE_PLYS)
			{
				int plys = i-lastCap-1;
				int wp = popcnt(piecesWHITE(pos,P));
                int wn = popcnt(piecesWHITE(pos,N));
                int wb = popcnt(piecesWHITE(pos,B));
                int wr = popcnt(piecesWHITE(pos,R));
                int wq = popcnt(piecesWHITE(pos,Q));
                
				int bp = popcnt(piecesBLACK(pos,P));
                int bn = popcnt(piecesBLACK(pos,N));
                int bb = popcnt(piecesBLACK(pos,B));
                int br = popcnt(piecesBLACK(pos,R));
                int bq = popcnt(piecesBLACK(pos,Q));

                /*if(wp==bp && wn==bn && wb==bb && wr==br && wq==bq &&
					wp<=8 && bp<=8 && wn<=2 && wb<=2 && wr<=2 && br<=2 &&
					bn<=2 && bb<=2 && wq<=1 && bq<=1)
				{
					//get count of doubled pawns
					int doubledWHITE = popcnt((piecesWHITE(pos,P) & fillDown(piecesWHITE(pos,P))));
					int doubledBLACK = popcnt((piecesBLACK(pos,P) & fillUp(piecesBLACK(pos,P))));

					if(abs(doubledWHITE-doubledBLACK)==1 && (!doubledWHITE || !doubledBLACK))
					{
						struct _pgnTable* entry =
						&pgnTable[wp][bp][wn][bn][wb][bb][wr][br][wq][bq];

						switch(RESULT_PLAIN(g.result))
						{
							case RESULT_WHITE_WINS:
								if(doubledWHITE)
									entry->wwins++;
								else
									entry->bwins++;
								break;
							case RESULT_BLACK_WINS:
								if(doubledBLACK)
									entry->bwins++;
								else
									entry->wwins++;
								break;
							case RESULT_DRAW:
								entry->draws++;
								break;
						}
					}
				}*/

				if((wp!=bp || wn!=bn || wb!=bb || wr!=br || wq!=bq) &&
					wp<=8 && bp<=8 && wn<=2 && wb<=2 && wr<=2 && br<=2 &&
					bn<=2 && bb<=2 && wq<=1 && bq<=1
					)
				{
					struct _pgnTable* entry1 =
					&pgnTable[plys][wp][bp][wn][bn][wb][bb][wr][br][wq][bq];
					struct _pgnTable* entry2 =
					&pgnTable[plys][bp][wp][bn][wn][bb][wb][br][wr][bq][wq];

					switch(RESULT_PLAIN(g.result))
					{
						case RESULT_WHITE_WINS:
							entry1->wwins++;
							entry2->bwins++;
							break;
						case RESULT_BLACK_WINS:
							entry1->bwins++;
							entry2->wwins++;
							break;
                        case RESULT_DRAW:
							entry1->draws++;
							entry2->draws++;
							break;
					}
				}
			}
		}
	}
	fclose(f);
	f = fopen("results.bin","wb");
	fwrite(pgnTable,sizeof(pgnTable),1,f);
	fclose(f);

	printf("%d\n",numgames);
	system("Pause");


	/*FILE* f;
	f = fopen("results.bin","rb");
	fread(pgnTable,sizeof(pgnTable),1,f);
	fclose(f);*/

	printf("%d\n",pgnTable[5][1][0][0][0][0][0][0][0][0][0].wwins);
	printf("%d\n",pgnTable[5][0][1][0][0][0][0][0][0][0][0].wwins);
	printf("%d\n",pgnTable[5][0][0][1][0][0][0][0][0][0][0].wwins);
	printf("%d\n",pgnTable[5][0][0][0][1][0][0][0][0][0][0].wwins);
	printf("%d\n",pgnTable[5][0][0][0][0][1][0][0][0][0][0].wwins);
	printf("%d\n",pgnTable[5][0][0][0][0][0][1][0][0][0][0].wwins);
	printf("%d\n",pgnTable[5][0][0][0][0][0][0][1][0][0][0].wwins);
	printf("%d\n",pgnTable[5][0][0][0][0][0][0][0][1][0][0].wwins);
	printf("%d\n",pgnTable[5][0][0][0][0][0][0][0][0][1][0].wwins);
	printf("%d\n",pgnTable[5][0][0][0][0][0][0][0][0][0][1].wwins);
	system("Pause");


	//Print results
	/*{int i; for(i=0;i<500;i++) {pgnTablePMT[i].gamesx2=pgnTablePMT[i].score = 0;}}
	{
        int wp, wn, wb, wr, wq, bp, bn, bb, br, bq;
		for (wp=0;wp<=8;wp++) {
		for (wn=0;wn<=2;wn++) {
		for (wb=0;wb<=2;wb++) {
		for (wr=0;wr<=2;wr++) {
		for (wq=0;wq<=1;wq++) {
		for (bp=0;bp<=8;bp++) {
		for (bn=0;bn<=2;bn++) {
		for (bb=0;bb<=2;bb++) {
		for (br=0;br<=2;br++) {
		for (bq=0;bq<=1;bq++) {
			struct _pgnTable* entry =
			&pgnTable[wp][bp][wn][bn][wb][bb][wr][br][wq][bq];
			if( !((wp-1 == bp) || (bp-1 == wp)) ) continue; //only a single pawn
			if(wn!=bn || wb!=bb || wr!=br || wq!=bq) continue; //other material equal

			if(entry->wwins || entry->bwins || entry->draws)
			{
				bool sign = 1;
				int mat = 10*(wp-bp) + 40*(wn-bn) + 41*(wb-bb)
					+60*(wr-br) + 120*(wq-bq);
				int totalmat = 10*(wp+bp) + 40*(wn+bn) + 41*(wb+bb)
					+60*(wr+br) + 120*(wq+bq);
				if(mat<0) { mat=-mat; sign = 0; }
				pgnTablePMT[totalmat].gamesx2+=2*(entry->wwins+entry->draws+entry->bwins);
				pgnTablePMT[totalmat].score+=entry->draws;
				if(sign)
					pgnTablePMT[totalmat].score += 2*entry->wwins;
				else
					pgnTablePMT[totalmat].score += 2*entry->bwins;
			}
		}}}}}}}}}}
	}


	f = fopen("material.txt","wt");
	{
		int i;
		for(i=0;i<500;i++)
		{
			if(pgnTablePMT[i].gamesx2)
				fprintf(f,"%f %d %d\n",i/10.0,winToPawn(((float)pgnTablePMT[i].score)/pgnTablePMT[i].gamesx2),pgnTablePMT[i].gamesx2/2);
		}
	}
	fclose(f);
	system("Pause");*/
}

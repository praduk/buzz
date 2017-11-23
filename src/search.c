/**
*Buzz Chess Engine
*search.c
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "thread.h"
#include "timemanager.h"
#include "search.h"
#include "movegen.h"
#include "xboard.h"
#include "hash.h"
#include "eval.h"
#include "moveordering.h"
#include "consolecolors.h"
#include "log.h"
#include "recog.h"
#include <stdio.h>
#include <stdlib.h>

volatile U64 nodes;
volatile int iterationDepth;
volatile int moveNumberDrawDetection;
Mutex searchStatus;

#ifdef PRINT_MAXPLY_REACHED
int maxply_reached;
#endif

#ifdef TRACE_SEARCH
bool TRACE_SEARCH_FLAG = true;
#endif

#ifdef DEBUG_PV
	volatile U64 nodesd[MAX_SEARCH_DEPTH+1]; //How many nodes for each depth
	volatile U64 qnodes;
	volatile U64 extensions[MAX_SEARCH_DEPTH+1];
	volatile U64 hashhits[MAX_SEARCH_DEPTH+1];
	volatile U64 hashprobes[MAX_SEARCH_DEPTH+1];
	volatile U64 cutoffs[MAX_SEARCH_DEPTH+1];
	volatile U64 reductions[MAX_SEARCH_DEPTH+1];
	volatile U64 OPEN_in[MAX_SEARCH_DEPTH+1];
	volatile U64 CUT_in[MAX_SEARCH_DEPTH+1];
	volatile U64 CUT_change[MAX_SEARCH_DEPTH+1];
	volatile U64 ALL_in[MAX_SEARCH_DEPTH+1];
	volatile U64 ALL_change[MAX_SEARCH_DEPTH+1];
	volatile U64 Sune_first[MAX_SEARCH_DEPTH+1];
	volatile U64 Sune_total[MAX_SEARCH_DEPTH+1];
	volatile U64 NTS_research[MAX_SEARCH_DEPTH+1];
	volatile time_ms previousDepthTime;
	void resetDebugDat()
	{
		int i;
		//PVnodes=0;
		for(i=0;i<=MAX_SEARCH_DEPTH;i++)
		{
			nodesd[i]=extensions[i]=hashhits[i]=hashprobes[i]
			=cutoffs[i]=reductions[i]=OPEN_in[i]=CUT_in[i]=CUT_change[i]
			=NTS_research[i]=ALL_in[i]=ALL_change[i]=Sune_first[i]
			=Sune_total[i]=U64EMPTY;
		}
	}
	U64 totalStat(volatile U64* arr,int plies_to_go)
	{
		int i;
		U64 total=U64EMPTY;
		for(i=0;i<plies_to_go;i++)
			total+=arr[i];
		return total;
	}
	void printSearchStats()
	{
		int i, maxply_reached;
		U64 nodes_sum=U64EMPTY;
		for(maxply_reached=1;
			nodesd[maxply_reached] && maxply_reached<=MAX_SEARCH_DEPTH+1;
			nodes_sum+=nodesd[maxply_reached], maxply_reached++);

		if(previousDepthTime)
			print("Branching Factor = %.2f\n",timeUsed()/(double)previousDepthTime);
		previousDepthTime=timeUsed();
		print("Qsearch Nodes/Nodes = %.2f%%\n",100*(qnodes/(double)nodes));
		print("Hash Table Usage = %f%%\n",100*HashTableUsage());
		{
			U64 sune_sum=0;
			U64 sune_first_sum=0;
			for(i=1;i<maxply_reached;i++)
			{
				sune_first_sum+=Sune_first[i];
				sune_sum+=Sune_total[i];
			}
			print("Sune%% = %.2lf%%\n",(double)100*sune_first_sum/sune_sum);
		}
		print("Depth  Node%%   Ext%% HashHit%%   Cut%% "
			"  Red%% CUTpred%% ALLpred%%  Sune%% NTSRS%%\n");
		for(i=1;i<maxply_reached;i++)
		{
			print("|%4d %6.2f %6.2f %8.2f %6.2f %6.2f %8.2f %8.2f %6.2f %6.2f\n",
				i,
				(double)100*nodesd[i]/totalStat(nodesd,maxply_reached),
				(double)100*extensions[i]/nodesd[i],
				(double)100*hashhits[i]/hashprobes[i],
				(double)100*cutoffs[i]/nodesd[i],
				(double)100*reductions[i]/nodesd[i],
				(double)100*(CUT_in[i]-CUT_change[i])/(CUT_in[i]),
				(double)100*(ALL_in[i]-ALL_change[i])/(ALL_in[i]),
				(double)100*Sune_first[i]/Sune_total[i],
				(double)100*NTS_research[i]/OPEN_in[i]
				);
		}
		print("\\-------------------------------------------------------------------------\n");


		/*print("ext=%.2f%% ",100*extensions/((double)(nodes-qnodes)));

		print("hh=%.1f%% ",100*hashhits/((double)hashprobes));
		print("cut=%.2f%% ",100*cutoffs/((double)(nodes-qnodes)));
		if(CUT_in!=U64EMPTY)
			print("CUTpred=%.2f%% ",100*(CUT_in-CUT_change_ALL)/((double)(CUT_in)));

		print("q/n=%.0f%%] ",100*qnodes/((double)nodes));

		print("Node Statistics\n");
		printU64(nodes_sum);
		print("\n");*/
	}
#endif
Thread searchThread;
volatile bool searchRunning=false; //initially false
volatile int rootMaterialBalance;
volatile bool stoppingSearch=false;



/****************
*Search algorithm
****************/

void initSearch()
{
	InitMutex(&searchStatus);
}

//Make sure actionLock is NOT locked when calling this
void stopSearch()
{
	stoppingSearch=true;
	LockM(searchStatus);
	printLogHTML(HTMLLOG_COLOR_GREEN,"Requesting search to stop\n");
	if(searchRunning)
	{
		searchRunning=false;
		ReleaseM(searchStatus);
		Join(searchThread);
	}
	else ReleaseM(searchStatus);
	stoppingSearch=false;
}

//returns true if it made a move
THREAD_PROC_RET searchStart(void* argbwd)
{
	board pos;
	int d;

	assert(argbwd!=NULL);
	{
		bwd* tempbwd=(bwd*)argbwd;
		pos=tempbwd->b;
		d=tempbwd->d;
		assert(d<=MAX_SEARCH_DEPTH);
		#ifdef DEBUG_DEPTH_LIMIT
			if(d>DEBUG_DEPTH_LIMIT) d=DEBUG_DEPTH_LIMIT;
		#endif
		free(argbwd);
	}

	LockM(searchStatus);
	if(searchRunning)
	{
		ReleaseM(searchStatus);
		QuitThread(0);
	}
	if(!stoppingSearch) searchRunning=true;
	ReleaseM(searchStatus);

	printLogHTML(HTMLLOG_COLOR_GREEN,"Started search thread - 0x%X\n",pos.hashkey);

	if(pos.AllPieces==U64EMPTY) //don't try searching illegal positions
	{
		searchRunning=false;
		QuitThread(false);
	}

	//Result checking
	{
		moveList ml;
		genMoves(&pos,&ml);

		if(ml.moveCount==0)
		{
			searchRunning=false;
			printLogHTML(HTMLLOG_COLOR_GREEN,"Quit search thread - 0x%X\n",pos.hashkey);
			QuitThread(false);
		}
		else if(pos.fifty==100)
		{
			searchRunning=false;
			printLogHTML(HTMLLOG_COLOR_GREEN,"Quit search thread - 0x%X\n",pos.hashkey);
			QuitThread(false);
		}
		//3 rep
		else
		{
			int i;
			unsigned char count=0;
			for(i=moveNumberDrawDetection-4;i>=0;i-=2)
			{
				if(gamePosition[i].hashkey==gamePosition[moveNumber].hashkey)
				{
					count++;
					if(count>=2)
					{
						searchRunning=false;
						printLogHTML(HTMLLOG_COLOR_GREEN,"Quit search thread - 0x%X\n",pos.hashkey);
						QuitThread(false);
					}
				}
			}
		}


		//insufficient material
		if(gameDecidableDraw(&pos))
		{
			searchRunning=false;
			printLogHTML(HTMLLOG_COLOR_GREEN,"Quit search thread - 0x%X\n",pos.hashkey);
			QuitThread(false);
		}
		//if(ml.moveCount==1)
		{
			LockM(actionLock);
			if(pos.hashkey==gamePosition[moveNumber].hashkey && compSide==gamePosition[moveNumber].side)
			{
				if(ml.moveCount==1)
				{
					searchRunning=false;
					printLogHTML(HTMLLOG_COLOR_GREEN,"Making singular move from search thread %0xX\n",pos.hashkey);
					makeComputerGameMove(ml.moves->m);
					printLogHTML(HTMLLOG_COLOR_GREEN,"Quit search thread - 0x%X\n",pos.hashkey);
					ReleaseM(actionLock);
					QuitThread(true);
				}
			}
			ReleaseM(actionLock);
			
			//start searching - expect side to move to change
			{
				//iterative deepening
				Thread searchingThread;
				rootMaterialBalance=materialWHITE(&pos);
				setStartTime();
				newSearchID();
				clearHistory();
				clearKillers();
				#ifdef CLEAR_HASH_EVERY_SEARCH
					resetHashTable();
				#endif
				nodes=0;
				#ifdef DEBUG_PV
				previousDepthTime=0;
				resetDebugDat();
				#endif
				#ifdef PRINT_MAXPLY_REACHED
					maxply_reached=0;
				#endif

				for(iterationDepth=1;iterationDepth<=d && searchRunning && !timeUp();iterationDepth++)
				{
					bwd* rootargs=malloc(sizeof(bwd));
					rootargs->b=pos;
					rootargs->d=iterationDepth;
					rootargs->alpha=-INF;
					rootargs->beta=INF;
					#ifdef NEW_SEARCHID_EVERY_ITERATION
					newSearchID();
					#endif
					#ifdef DEBUG_PV
						resetDebugDat();
					#endif
					searchingThread=BeginThread(searchRoot,(void*)rootargs);
					Join(searchingThread);
				}

				LockM(actionLock);
				if(gamePosition[moveNumber].hashkey==pos.hashkey && compSide==gamePosition[moveNumber].side)
				{
					hashEntry probe;
					bool hit;
					hit=probeHash(pos.hashkey,&probe);
					searchRunning=false;
					printLogHTML(HTMLLOG_COLOR_GREEN,"Making a move now - 0x%X\n",pos.hashkey);
					if(hit)
						makeComputerGameMove(extractMove(probe.data));
					else
					{
						print("Error (no move in hash table): "__FILE__ " %d\n",__LINE__);
						makeComputerGameMove(ml.moves->m);
					}
					printLogHTML(HTMLLOG_COLOR_GREEN,"Quit search thread - 0x%X\n",pos.hashkey);
					ReleaseM(actionLock);
					QuitThread(true);
				}
				ReleaseM(actionLock);
				searchRunning=false;
				printLogHTML(HTMLLOG_COLOR_GREEN,"Quit search thread - 0x%X\n",pos.hashkey);
				QuitThread(false);
			}
		}
	}
	//GCC will complain about the control reaching here but no worries here
}

/*************************************
*Search Algorithm: Alphabeta on a move
*************************************/

THREAD_PROC_RET searchRoot(void* bwdargs)
{
	moveList ml;
	int score=-INF;
	move bestMove=NULLMOVE;
	unsigned int i;
	U64 drawTable[MAX_SEARCH_DEPTH+1];

	board pos=((bwd*)bwdargs)->b;
	int depth=((bwd*)bwdargs)->d;
	int alpha=((bwd*)bwdargs)->alpha;
	int beta=((bwd*)bwdargs)->beta;

	int nodeType = OPEN_NODE;

	free(bwdargs);

	assert(alpha<beta);
	
	genMoves(&pos,&ml);
	orderMoves(&pos,&ml,0);


	*drawTable=pos.hashkey;

	assert(ml.moveCount>0);
	for(i=0;i<ml.moveCount;i++)
	{
		int tempVal;
		int lowerbound=max(alpha,score);
		
		
		#ifdef NTS
		if(lowerbound+1==beta || nodeType == OPEN_NODE)
		#endif
			tempVal=-search(pos,ml.moves[i].m,-beta,-lowerbound,depth-1,1,drawTable,nodeType);
		#ifdef NTS
		else
		{
			tempVal=-search(pos,ml.moves[i].m,-(lowerbound+1),-lowerbound,depth-1,1,drawTable,nodeType);
			if(tempVal > lowerbound && tempVal < beta)
			{
				tempVal=-search(pos,ml.moves[i].m,-beta,-(tempVal-1),depth-1,1,drawTable,OPEN_NODE);
			}
		}
		#endif

		nodeType = ALL_NODE;
		#ifdef PRINT_MOVENODES
		{
			sanString s;
			moveToSAN(&pos,ml.moves[i].m,s);
			print("%s ",s);
			printU64(nodes);
			print("\n");
		}
		#endif
		#ifdef PRINT_MOVESCORES
		{
			sanString s;
			moveToSAN(&pos,ml.moves[i].m,s);
			print("%s (%d,%d) %d\n",s,score,beta,tempVal);
		}
		#endif
		if(tempVal>score)
		{
			bestMove=ml.moves[i].m;
			score=tempVal;
			storeRootHash(pos.hashkey,bestMove,-INF,-INF+1,score,depth,0); //lowerbound
			#ifndef DEBUG_PV
				if(post) //printing PV (depth score time nodes PV)
				{
					print("%d ",depth);
					if(score<0) CC_Red();
					if(score==0) CC_Blue();
					if(score>0) CC_Green();
					print("%d ",score);
					CC_Normal();
					print("%u ",timeUsed()/10);
					printU64(nodes);
					print(" ");
					if(ponder && compSide==gamePosition[moveNumber].xside)
					{
						sanString ss;
						moveToSAN(gamePosition+moveNumber,ponderMove,ss);
						if(inCheck(pos,pos.side)) print("(%s+) ",ss);
						else print("(%s) ",ss);
					}
					
					printPV(pos);
					print("\n");
				}
			#endif
			if(score>=beta) break;
		}
	}

	storeRootHash(pos.hashkey,bestMove,alpha,beta,score,depth,0);
	storeHistory(bestMove,depth,pos.side);
	#ifdef DUMP_REAL_PV
	hashPV(&pv,pos,alpha,beta,score);
	#endif
	if(post) //printing PV (depth score time nodes PV)
	{
		CC_INormal();
		print("%d ",depth);
		if(score<0) CC_IRed();
		if(score==0) CC_IBlue();
		if(score>0) CC_IGreen();
		print("%d ",score);
		CC_INormal();
		print("%u ",timeUsed()/10);
		printU64(nodes);
		print(" ");
		if(ponder && compSide==gamePosition[moveNumber].xside)
		{
			sanString ss;
			moveToSAN(gamePosition+moveNumber,ponderMove,ss);
			if(inCheck(pos,pos.side)) print("(%s+) ",ss);
			else print("(%s) ",ss);
		}
		printPV(pos);
		print("\n");
		#ifdef DEBUG_PV
			printSearchStats();
		#endif
		CC_Normal();
	}
	QuitThread(score);
}

int search(board pos, move m, int alpha, int beta, int depth, int dfr, U64* drawTable, int p_nodeType)
{
	hashEntry probe;
	bool hashHit = false;
	moveList ml;
	int score=-INF;
	move bestMove=NULLMOVE;
	unsigned int i;
	bool extended=false;
	int nodeType = childNodeType(p_nodeType);
	#ifdef DEBUG_PV
		int enterType = nodeType;
	#endif

	#ifdef PRINT_MAXPLY_REACHED
	if(dfr>maxply_reached)
		print("Max ply reached = %d\n",maxply_reached=dfr);
	#endif

	assert(alpha<beta);

	nodes++;
	#ifdef DEBUG_PV
	nodesd[dfr]++;
	#endif
	if(!(nodes&1024) && (timeUp() || searchRunning==false)) QuitThread(false);

	#ifdef TRACE_SEARCH
	if(TRACE_SEARCH_FLAG)
	{
		int i;
		sanString s;
		for(i=0;i<dfr-1;i++) print(" ");
		moveToSAN(&pos,m,s);
		print("%s (%d, %d) ",s,alpha,beta);
		printU64(nodes);
		print("\n");
		MilliSleep(TRACE_SEARCH);
	}
	#endif
	

	makemove(&pos,m);

	//test stop conditions here
	if(pos.fifty==100)//fifty move rule
	{
		U64 checkers;
		if(!(checkers=inCheck(pos,pos.side)))
			return 0;
		else //make sure we arn't mated
		{
			moveList ml;
			//this case is rare, so hardly any loss in speed to generate moves here
			genEvasions(&pos,&ml,checkers);
			if(ml.moveCount==0)
				return -MATE+dfr;
			else
				return 0;
		}
	}
	//do repetition here
	#ifdef NULLMOVE_NOREPCHECK
	if(m!=NULLMOVE)
	#endif
	{
		int j;
		for(j=dfr-4;j>=0;j-=2)
			if(pos.hashkey==drawTable[j])
				return 0;
		j+=moveNumberDrawDetection;
		while(j>=0)
		{
			if(pos.hashkey==gamePosition[j].hashkey)
				return 0;
			j-=2;
		}
	}
	drawTable[dfr]=pos.hashkey;

	if(recogDraw(&pos)) return 0;

	#ifdef MATE_DISTANCE_PRUNING
	{
		//The maximum mate score that can be attained by 
		//the current player
		//MATE-(dfr+1)
		int mate_score_bound=MATE-(dfr+1);
		if(alpha>=mate_score_bound) return mate_score_bound;
		if(beta>mate_score_bound) beta=mate_score_bound;

		//The maximum mate score that the opponent can
		//have (mate right now)
		mate_score_bound=-(MATE-dfr);
		if(mate_score_bound>=beta) return mate_score_bound;
		if(mate_score_bound>alpha) alpha=mate_score_bound;
	}
	#endif
	
	//Make sure depth is ok
	if(depth<0) depth=0;

	//extensions
	if(inCheck(pos,pos.side)) //check extension
	{
		depth++; 
		assert(extended==false);
		extended=true;
	}
	#ifdef KING_PAWN_ENDING_EXTENSION	
	else if(extractCapture(m)!=E && extractCapture(m)!=P
		&& pos.Pieces[R]==pos.Pieces[Q] && pos.Pieces[Q]==pos.Pieces[B] 
		&& pos.Pieces[B]==pos.Pieces[N] && pos.Pieces[N]==U64EMPTY)
	{
		depth+=KING_PAWN_ENDING_EXTENSION;
		assert(extended==false);
		extended=true;
	}
	#endif
	#ifdef PAWN_TO_7TH_EXTENSION
	else if(extractExtra(m)==MO_PASSER && toBit(extractTo(m))&C64(0x00FF00000000FF00))
	{
		depth++;
		assert(extended==false);
		extended=true;
	}
	#endif
	#ifdef PASSER_EXTENSION
	else if(extractExtra(m)==MO_PASSER)
	{
		depth++;
		assert(extended==false);
		extended=true;
	}
	#endif

	//probing hash table
	hashHit=probeHash(pos.hashkey,&probe);
	#ifdef DEBUG_PV
		hashprobes[dfr]++;
	#endif
	if(hashHit)
	{
		int tt_depth=probe.depth;
		int tt_flag=extractFlag(probe.data);
		int tt_score=probe.score;

		#ifdef DEBUG_PV
			hashhits[dfr]++;
		#endif

		#ifdef USE_HASH
		//Regular Hash Usage by Depth	
		if(tt_depth>=depth)
		{
			if(tt_score>=MATE_MIN) tt_score-=dfr;
			else if(tt_score<=-MATE_MIN) tt_score+=dfr;

			switch(tt_flag)
			{
			case HASHFLAG_EXACT:
				if(tt_depth>=depth)
					return tt_score;
			case HASHFLAG_LOWER_BOUND:
				{
					if(tt_score>=beta) 
					{
						#ifdef DEBUG_PV
							cutoffs[dfr]++;
						#endif
						return tt_score;
					}
					#ifdef UPDATE_BOUNDS
					if(tt_score>alpha)
						alpha=tt_score;
					#endif
					break;
				}
			case HASHFLAG_UPPER_BOUND:
				{
					if(tt_score<=alpha)
					{
						#ifdef DEBUG_PV
							cutoffs[dfr]++;
						#endif
						return tt_score;
					}
					#ifdef UPDATE_BOUNDS
					if(tt_score<beta)
						beta=tt_score;
					#endif
					break;
				}
			}
		}
		#endif //USE_HASH
	}

	assert(alpha<beta);
	if(depth<=0 || dfr>=MAX_SEARCH_DEPTH)
	{
		return qsearchRoot(&pos,alpha,beta,dfr);
	}

	#ifdef NULLMOVE_PRUNING
		if(
			#ifdef NULLMOVE_NO_ALLNODE
			nodeType!=ALL_NODE &&
			#endif
			#ifdef NULLMOVE_NO_OPENNODE
			nodeType!=OPEN_NODE &&
			#endif
			#ifdef NULLMOVE_NO_MULTI_NULLMOVE
			!extractSFNull(pos.SF) &&
			#endif
			#ifdef NULLMOVE_NO_BETAMATE
			beta<MATE_MIN &&
			#endif
			#ifdef NULLMOVE_DEPTH_LEFT_LIMIT
			depth>=NULLMOVE_DEPTH_LEFT_LIMIT &&
			#endif
			!inCheck(pos,pos.side) && 
			popcnt(pos.PiecesSide[pos.side]&~(pos.Pieces[P]|pos.Pieces[K]))>PIECES_NULLMOVE_OFF
			
			#ifdef NULLMOVE_NO_QS_ABOVE_BETA
			//&& eval(&pos,alpha,beta)>=beta
			#endif
			)

		{
			int tempVal=-search(pos,NULLMOVE,-beta,-(beta-1),depth-1-adaptiveR(&pos,depth),dfr+1,drawTable,nodeType);
			if(tempVal>=beta)
			{
				#ifdef DEBUG_PV
					cutoffs[dfr]++;
				#endif
				return beta;
			}
			#ifdef MATE_THREAT_EXTENSION
			if(!extended && 
				tempVal<=-MATE_MIN && 
				//nodeType==OPEN_NODE &&
				depth<MATE_THREAT_EXTENSION_DEPTH_LIMIT &&
				dfr<MATE_THREAT_EXTENSION_DFR_LIMIT)
			{
				#ifdef DEBUG_PV
					extensions[dfr]++;
				#endif
				depth++;
			}
			#endif
		}
	#endif

	//buggy over here (can't detect stalemates well)
	genMoves(&pos,&ml);
	if(ml.moveCount==0)
	{
		if(inCheck(pos,pos.side))
			return -MATE+dfr; //mate
		else return 0; //stalemate
	}
	#ifdef SINGULAR_REPLY_EXTENSIONS
		if(ml.moveCount==1
			#ifndef SINGULAR_REPLY_EXTEND_ALWAYS
				&& !extended
			#endif
			)
		{
			#ifdef DEBUG_PV
				extensions[dfr]++;
			#endif
			extended=true;
			depth++;
		}
		else
	#endif

	//if(nodeType!=ALL_NODE)
	orderMoves(&pos,&ml,dfr);

	#ifdef DEBUG_PV
		if(extended) extensions[dfr]++;
	#endif

	#ifdef DEBUG_PV
	switch(nodeType)
	{
		case CUT_NODE: CUT_in[dfr]++; break;
		case ALL_NODE: ALL_in[dfr]++; break;
		case OPEN_NODE: OPEN_in[dfr]++; break;
	}
	#endif

	//Move loop
	{
		for(i=0;i<ml.moveCount;i++)
		{
			int tempVal;
			int lowerbound=max(alpha,score);
			int extendDepth=0;
			#ifdef RECAPTURE_EXTENSIONS
			if(!extended 
				#ifdef RECAPTURE_OPEN_NODE_ONLY
				&& (nodeType==OPEN_NODE /*|| nodeType==CUT_NODE*/)
				#endif
				&& extractCapture(m)!=E
				&& extractTo(m)==extractTo(ml.moves[i].m)
				&& ml.moves[i].score>=MOVEORDERING_CAPTURE) //recapture extension
				{extendDepth=1;}
			#endif

			#ifdef NTS
			if(lowerbound+1==beta || (nodeType == OPEN_NODE))
			#endif			
				tempVal=-search(pos,ml.moves[i].m,-beta,-lowerbound,depth-1+extendDepth,dfr+1,drawTable,nodeType);
			#ifdef NTS
			else if(nodeType == ALL_NODE)
			{
				tempVal=-search(pos,ml.moves[i].m,-(lowerbound+1),-lowerbound,depth-1+extendDepth,dfr+1,drawTable,nodeType);
				if(tempVal > lowerbound && tempVal < beta)
				{
					#ifdef DEBUG_PV
					if(alpha+1!=beta) NTS_research[dfr]++;
					#endif
					tempVal=-search(pos,ml.moves[i].m,-beta,-(tempVal-1),depth-1+extendDepth,dfr+1,drawTable,OPEN_NODE);
				}
			}
			else
			{
				tempVal=-search(pos,ml.moves[i].m,-beta,-(beta-1),depth-1+extendDepth,dfr+1,drawTable,nodeType);
				if(tempVal > lowerbound && tempVal < beta)
					tempVal=-search(pos,ml.moves[i].m,-(tempVal+1),-lowerbound,depth-1+extendDepth,dfr+1,drawTable,OPEN_NODE);
			}
			#endif //NTS

			if(nodeType == CUT_NODE)
			{
				if(tempVal<beta)
				{
					nodeType = ALL_NODE;
					#ifdef DEBUG_PV
					CUT_change[dfr]++;
					#endif
				}
			}
			else nodeType = ALL_NODE;
			


			#ifdef TRACE_SEARCH
			if(TRACE_SEARCH_FLAG)
			{
				int index;
				for(index=0;index<dfr;index++) print(" ");
				print("Value = %d\n",tempVal);
				MilliSleep(TRACE_SEARCH);
			}
			#endif
			if(tempVal>score)
			{
				bestMove=ml.moves[i].m;
				score=tempVal;
				if(score>=beta)
				{
					#ifdef USE_NULL_KILLER
					if(m==NULLMOVE)
						storeNullKiller(bestMove,dfr);
					else
					#endif
					storeKiller(bestMove,dfr);
					#ifdef TRACE_SCORES
					if(TRACE_SEARCH_FLAG)
					print("beta cut (%d,%d) %d\n", max(alpha,score), beta, tempVal);
					#endif
					storeHash(pos.hashkey,bestMove,alpha,beta,score,depth,dfr);
					storeHistory(bestMove,depth,pos.side);

					#ifdef DEBUG_PV
					cutoffs[dfr]++;
					if(enterType==ALL_NODE) ALL_change[dfr]++;
					if(!i) Sune_first[dfr]++;
					Sune_total[dfr]++;
					#endif
					return score;
				}
				#ifdef TRACE_SCORES
				if(TRACE_SEARCH_FLAG)
				print("val>score (%d,%d) %d\n", max(alpha,score), beta, tempVal);
				#endif
			}
		}
	}

	
	storeHash(pos.hashkey,bestMove,alpha,beta,score,depth,dfr);
	/*if(score>=alpha) //History heuristic works better only in beta cutoffs
		storeHistory(bestMove,depth,pos.side);*/
	return score;
}

/*****************
*Quiescence Search
*****************/

int qsearchRoot(board* pos, int alpha, int beta, int dfr)
{
	int score;
	score=eval(pos,alpha,beta);

	//fen(pos);

	//print("Gets here\n");
	#ifdef DO_SYMMETRY_CHECK
	symmetryCheck(pos);
	#endif
	if(score>=beta)
		return score;
	
	{
		moveList ml;
		unsigned int i;
		//we never enter qsearch in check
		assert(!inCheck(*pos,pos->side));
		#ifdef QSEARCH_CHECKS
			genQCMoves(pos,&ml);
		#else
			genQMoves(pos,&ml);
		#endif
		orderQMoves(pos,&ml);
		for(i=0;i<ml.moveCount;i++)
		{
			int tempVal;
			if(ml.moves[i].score<0) break;

			tempVal=-qsearch(*pos,ml.moves[i].m,-beta,-max(alpha,score),dfr+1);
			if(tempVal>score)
			{
				score=tempVal;
				if(score>=beta) return score;
			}
		}
	}
	return score;
}

int qsearch(board pos, move m, int alpha, int beta, int dfr)
{
	int score;
	#ifdef COUNT_QSEARCH_NODES
		nodes++;
	#endif
	#ifdef DEBUG_PV
		qnodes++;
	#endif
	//if(!(nodes&1024) && (timeUp() || searchRunning==false)) QuitThread(false);

	makemove(&pos,m);

	#ifdef TRACE_SEARCH
	if(TRACE_SEARCH_FLAG)
	{
		int i;
		sanString s;
		print("q");
		for(i=1;i<dfr-1;i++) print(" ");
		moveToSAN(&pos,m,s);
		print("%s (%d, %d) ",s,alpha,beta);
		printU64(nodes);
		print("\n");
		MilliSleep(TRACE_SEARCH);
	}
	#endif

	if(recogDraw(&pos)) return 0;

	#ifdef MATE_DISTANCE_PRUNING
	{
		//The maximum mate score that can be attained by 
		//the current player
		//MATE-(dfr+1)
		int mate_score_bound=MATE-(dfr+1);
		if(alpha>=mate_score_bound) return mate_score_bound;
		if(beta>mate_score_bound) beta=mate_score_bound;

		//The maximum mate score that the opponent can
		//have (mate right now)
		mate_score_bound=-(MATE-dfr);
		if(mate_score_bound>=beta) return mate_score_bound;
		if(mate_score_bound>alpha) alpha=mate_score_bound;
	}
	#endif
	
	#ifdef DO_SYMMETRY_CHECK
		if(!symmetryCheck(&pos))
		{
			print("Symmetry Check Failed!\n");
			printBoard(&pos);
			fen(&pos);
			system("Pause");
		}
	#endif

	{
		moveList ml;

		//check evasions
		{
			U64 checkers=inCheck(pos,pos.side);
			if(checkers)
			{
				unsigned int i;
				genEvasions(&pos,&ml,checkers);
				if(ml.moveCount==0)
					return -MATE+dfr;
				score=-INF;
				for(i=0;i<ml.moveCount;i++)
				{
					int tempVal;
					tempVal=-qsearch(pos,ml.moves[i].m,-beta,-max(alpha,score),dfr+1);
					if(tempVal>score)
					{
						score=tempVal;
						if(score>=beta) return score;
					}
				}
				return score;
			}
		}

		score=eval(&pos,alpha,beta);

		
		if(score>=beta)
			return score;

		genQMoves(&pos,&ml);
		orderQMoves(&pos,&ml);

		//iterate through moves
		{
			unsigned int i;
			for(i=0;i<ml.moveCount;i++)
			{
				int tempVal;
				if(ml.moves[i].score<0) break;
				tempVal=-qsearch(pos,ml.moves[i].m,-beta,-max(alpha,score),dfr+1);
				if(tempVal>score)
				{
					score=tempVal;
					if(score>=beta) return score;
				}
			}
		}
	}
	return score;
}

int adaptiveR(const board* pos, const int depth)
{
	#ifdef NULLMOVE_ENSURE_OPPMOVE
	if(depth<=3) return 1;
	if(depth==4) return 2;
	#endif
	#ifdef NULLMOVE_ADAPTIVE_R
	{
		U64 pieces=pos->PiecesSide[pos->side]&~(pos->Pieces[P]|pos->Pieces[K]);
		int piecesCurrentSide = popcnt(pieces);
		return 2 + ((depth) > (6 + ((piecesCurrentSide<3)?2:0)));
	}
	#endif
	#ifdef NULLMOVE_CONST_R
		return NULLMOVE_CONST_R;
	#endif
}

int childNodeType(const int previous_type)
{
	switch(previous_type)
	{
	case ALL_NODE: return CUT_NODE;
	case CUT_NODE: return ALL_NODE;
	default: return OPEN_NODE;
	}
}

/****************
*Perft and Divide
*****************/

U64 perft(board pos, move m, int d)
{
	U64 currnodes;
	unsigned int i;
	moveList ml;

	makemove(&pos,m);
#ifdef PERFT_HASH_TEST
	{
		perftEntry* probe=probePerftHash(pos.hashkey);
		if(probe!=NULL && extractPerftDepth(probe->data)==d)
			return extractPerftNodes(probe->data);
	}
#endif

	genMoves(&pos,&ml);
	if(d-1==0)
		return ml.moveCount;
	currnodes=0;
	for(i=0;i<ml.moveCount;i++)
		currnodes+=perft(pos,ml.moves[i].m,d-1);
#ifdef PERFT_HASH_TEST
	storePerftHash(pos.hashkey,currnodes,d);
#endif
	return currnodes;
}

//takes in a bwd and free's data
DTHREAD_PROC_RET perftStart(void* args)
{
	bwd* temp;
	board pos;
	moveList ml;
	unsigned int d;
	unsigned int i;
	int t0;
	U64 currnodes=0;

	assert(args!=NULL);
	resetHashTable();

	temp=(bwd*)(args);
	pos=temp->b;
	d=temp->d;
	free(temp);

	nodes=0;
	t0=getms();
	genMoves(&pos,&ml);
	for(i=0;i<ml.moveCount;i++)
		currnodes+=perft(pos,ml.moves[i].m,d-1);
#ifdef PERFT_HASH_TEST
	print("\nPerft %d (with lousy hashing scheme)\n",d);
#else
	print("\nPerft %d\n",d);
#endif
	print("Time: %d ms\n",(getms()-t0));
	print("Leaf Nodes: ");
	printU64(currnodes);
	print("\n");
}

void divide(board pos, int d)
{
	unsigned int i;
	moveList ml;

	genMoves(&pos,&ml);
	for(i=0;i<ml.moveCount;i++)
	{
		coordString mstr;
		moveToCoords(&pos, ml.moves[i].m,mstr);
		print("%s ",mstr);
		printU64(perft(pos,ml.moves[i].m,d-1));
		print("\n");
	}
}

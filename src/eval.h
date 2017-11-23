/**
*Buzz Chess Engine
*eval.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _evalh
#define _evalh

#include <math.h>
#include "defs.h"
#include "board.h"


/*
*Debugging
*/

//#define DEBUG_EVAL
//#define ZERO_EVAL

/*
*Build Configuration
*/

//Number of terms in the eval in the define
//set to 0 if not used
#define MATERIAL_EVAL
#define KING_DISTANCE_EVAL
#define MOBILITY_EVAL
#define PAWN_EVAL

/*
*Defenitions
*/

#define MatTot 5000

#define MATE 32000
#define MATE_MIN (MATE-MAX_SEARCH_DEPTH)
#define INF MATE
#define ILLEGAL_SCORE (MATE+1);

/*
*Evaluation Weights and Macros
*/

//Piece Square Tables can be found in eval.c

//generic material values, used for SEE, game stage and so on
//These values are from suggestions by fellow programmers
#define Pval 100
#define Nval 400
#define Bval 400
#define Rval 600
#define Qval 1200
#define Kval 10000

//The game stage is equivilent to the piece material for one side
#define STAGE_MAX (2*(Rval+Bval+Nval)+Qval)

//opening score, endgame score, game stage of the opponent
#define SCALE_SCORE(op,eg,stage) ((stage>=STAGE_MAX)? op : ((((op) - (eg))*(stage) + STAGE_MAX*(eg))/STAGE_MAX))
//#define SCALE_SCORE(op,eg,stage) ((stage>=STAGE_MAX)? op : ((((op) - (eg))*(stage))/STAGE_MAX) + (eg))

//Average opening and endgame material weights
//Pawns are more valuable in the EG (from experience)
#define P_op Pval
#define P_eg (Pval+(Nval+Bval)/20)
//From Wikipedia - Chess piece point value:
//Alburt, Lev & Nikolai Krogius, Just the Facts!: Winning Endgame Knowledge in One Volume (second ed.)
//2R~=Q in OP, and 2R+P~=Q in EG (Alburt & Krogius)
#define Q_op Qval
#define Q_eg Qval
#define R_op Rval
#define R_eg (Rval+(Qval-Rval)/10)
//Opening: R+2P<2B; R+2P~<=B+N; R+2P=2N (Alburt & Krogius)
//Endgame:  R+P=2N;  R+P~<=B+N; R+2P=2B (Alburt & Krogius)
#define B_op Bval
#define B_eg (Bval+(R_eg-Rval+2*(P_eg-P_op))/2)
#define N_op Nval
#define N_eg (B_eg-P_eg/2)

//Piece Square Table weights for king
#define K_op_PSTW 3
#define K_eg_PSTW 4

//Endgame King Distance Scores
//Kdist_xside = distance to hostile King weight
//Kdist_side  = distance to friendly King weight
//Eval = Kdist_xside*(distance to hostile King) - Kdist_side*(distance to opponent king)

#define Kdist_side  1/3
#define Kdist_xside 2/3
#define Kdist_halfpasser_side    1/6
#define Kdist_halfpasser_xside   2/6
#define Kdist_passer_side    1/3
#define Kdist_passer_xside   2/3

#define MobB_op 5
#define MobN_op 6
#define MobR_op 4
#define MobQ_op 3

//Pawn Eval
#define doubledPenalty_op 15
#define doubledPenalty_eg 30
#define isolatedPenalty_op 10
#define isolatedPenalty_eg 20
#define unprotectedPenalty_op 4
#define unprotectedPenalty_eg 1
#define backwardPenalty_op 8
#define backwardPenalty_eg 15

//Passer and half-passer bonus by rank is in eval.c
#define passerBonus_op 10  //Static Bonus
#define passerRBonus_op 60 //Bonus at Rank 7
#define passerBonus_eg 20  //Static Bonus
#define passerRBonus_eg 120 //Bonus at Rank 7
#define halfPasserBonus_op 5  //Static Bonus
#define halfPasserRBonus_op 50 //Bonus at Rank 7
#define halfPasserBonus_eg 10  //Static Bonus
#define halfPasserRBonus_eg 100 //Bonus at Rank 7
#define connectedPasser_Bonus_op 20 //Static Bonus added to passer bonus
#define connectedPasser_Bonus_eg 50 //Static Bonus added to passer bonus
#define unstoppablePasser 800

//passer PSTs are in eval.c

//Tempo Eval
#define Tempo_op 10
#define Tempo_eg  0

/*
*Inlined Functions
*/

/*
*Function Prototypes
*/

#ifndef ANDRES_EVAL
int eval(const board* pos, int alpha, int beta);
int fulleval(const board* pos, int alpha, int beta);
#else
void init_AndresEval();
int AndresEval(const board* pos, int alpha, int beta);
#define eval(x,alpha,beta) AndresEval(x,alpha,beta)
#endif
int materialWHITE(const board* pos);
int materialBLACK(const board* pos);
int materialSide(const board* pos);
int materialXSide(const board* pos);
bool symmetryCheck(const board* b);

/*
*Global Data
*/

extern int pieceValue[7];
extern const char EvalWeightNames[][64];

#ifdef ANDRES_EVAL
extern char degbb[4096];
extern int dcache;
#endif

#endif

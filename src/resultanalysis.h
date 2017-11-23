/**
*Buzz Chess Engine
*resultanalysis.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _resultanalysish
#define _resultanalysish

#include "defs.h"
#include <math.h>

/*
*Functions
*/

double erf(double x);
double cdf(double x, double mu, double sigma);
double winProb(double win1, double sd1, double win2, double sd2);
void mean_sd(double W, double D, double L, double* mean, double* sd);
void errorBars(double W, double D, double L, double C, double* smin, double* nominal, double* smax);

/*
*Inlined Functions
*/

//To convert between win probability and centi-pawn value (1centipawn = 1elo)
static INLINE int winToPawn(double win_probability)
{
	double ret = 400*log10(win_probability/(1-win_probability));
	if(ret<0) return (int)(ret-0.5);
	else return (int)(ret+0.5);
}
static INLINE double winToELO(double win_probability)
{
	if(win_probability<=0.0) return -2000;
	if(win_probability>=1.0) return  2000;
	return 400*log10(win_probability/(1-win_probability));
}

static INLINE double pawnToWin(int centipawn_advantage)
{
	return 1/(1+pow(10.0,centipawn_advantage/400.0));
}
static INLINE double ELOToWin(int centipawn_advantage)
{
	return 1/(1+pow(10.0,centipawn_advantage/400.0));
}

#endif
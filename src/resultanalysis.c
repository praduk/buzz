/**
*Buzz Chess Engine
*resultanalysis.c
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include <stdlib.h>
#include "resultanalysis.h"

/*
*Given the mean and standard deviation of a score for two different
*moves, the probability that move 1 is better than move 2 can
*be computed with this function.
*/
double winProb(double win1, double sd1, double win2, double sd2)
{
	return 1.0-cdf(0,win1-win2,sd1+sd2);
}

//approximation of the error function
double erf(double x)
{
	const double pi = acos(-1.0);
	double a = -8.0/(3*pi)*(pi-3.0)/(pi-4.0);
	double x2=x*x;
	return (x<0?-1:1)*sqrt(1 - exp(-x2*(4/pi+a*x2)/(1+a*x2)));
}

//CDF for normal distribution
double cdf(double x, double mu, double sigma)
{
	return 0.5*(1+erf((x-mu)/sigma/sqrt(2)));
}

/*
*Computing mean score and standard deviation given wins, draws, losses
*/
void mean_sd(double W, double D, double L, double* mean, double* sd)
{
	double n = W+D+L;
	double m = (W+0.5*D)/n;

	double x = W*(1-m)*(1-m) + D*(0.5-m)*(0.5-m) + L*m*m;

	if(mean!=NULL) *mean = m;
	if(sd!=NULL) *sd = sqrt(x/(n-1))/sqrt(n-1);
}

/*
*Computing min and max scores given wins, draws, losses, and a confidence
*/
void errorBars(double W, double D, double L, double C, double* smin, double* mean, double* smax)
{
	double n = W+D+L;
	double m = (W+0.5*D)/n;

	double x = W*(1-m)*(1-m) + D*(0.5-m)*(0.5-m) + L*m*m;
	double e;

	if(mean!=NULL) *mean = m;
	if(n<=1)
	{
		*smin = 0;
		*smax = 1;
		return;
	}
	e = C*sqrt(x/(n-1))/sqrt(n-1);
	if(smin!=NULL) {*smin = m-e; if(*smin<0) *smin=0;}
	if(smax!=NULL) {*smax = m+e; if(*smax>1) *smax=1;}
}
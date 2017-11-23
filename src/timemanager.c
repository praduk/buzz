/**
*Buzz Chess Engine
*timemanager.c
*
*Copyright (C) 2007 Pradu Kannan.
**/

#if (defined(_WIN32) || defined(_WIN64))
	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
#endif

#ifdef _POSIX
	#include <sys/time.h>
#endif

#include "timemanager.h"
#include "xboard.h"
#include "log.h"


unsigned int st=0;           /*seconds*/
unsigned int mps=0;          /*moves per TC*/
unsigned int nextTC=0;       /*milliseconds*/
unsigned int inc=0;          /*seconds*/
unsigned int myTime=300000;  /*milliseconds*/
unsigned int opponentTime=5; /*milliseconds*/
time_ms startTime=0;         /*milliseconds*/
time_ms endTime=0;           /*milliseconds*/

//Private Data
bool infTime=false;

time_ms getms()
{
	#if (defined(_WIN32) || defined(_WIN64))
		return GetTickCount();
	#endif
	#ifdef _POSIX
		struct timeval tv;
		gettimeofday(&tv,NULL);
		return (((time_ms) tv.tv_sec)*1000) + (time_ms) tv.tv_usec/1000;
	#endif
}

void infiniteTime()
{
	infTime=true;
	printLogHTML(HTMLLOG_COLOR_GREEN,"Timemanager - Infinite Time\n");
}

void finiteTime()
{
	infTime=false;
	printLogHTML(HTMLLOG_COLOR_GREEN,"Timemanager - Finite Time\n");
}

void setStartTime()
{
	startTime=getms();
}

void setEndTime()
{
	time_ms bufferedTimeLeft;
	time_ms timeToUse;

	if(sd)
	{
		infTime=true;
		return;
	}

	infTime=false;
	if(st) timeToUse=st*1000;
	else
	{
		bufferedTimeLeft=myTime-BUFFER_TIME;
		if(mps)
		{
			unsigned int movesLeft=mps-((moveNumber/2)+(compSide==WHITE && moveNumber%2))%mps+BUFFER_MPS;
			timeToUse=myTime/movesLeft;
		}
		else
			timeToUse=myTime/TIME_DIVIDER;
	}
	if(inc)
		timeToUse += (inc*1000-BUFFER_INC);

	//adjust my time
	if(!timeToUse) timeToUse=1000;
	
	printLogHTML(HTMLLOG_COLOR_GREEN,"Timemanager - time to use %u ms\n",timeToUse);

	setStartTime();
	endTime=startTime+timeToUse;
	
	printLogHTML(HTMLLOG_COLOR_GREEN,"Timemanager - set end time to %u\n",endTime);

	if(!st)
	{
		//Increment
		myTime+=inc*1000;
		if(mps)
		{
			unsigned int moves_played;
			if(compSide==WHITE)
				moves_played = (moveNumber+1)/2;
			else
				moves_played = moveNumber/2;

			if(compSide==gamePosition[moveNumber].side 
				&& ((moves_played + 1)%mps)==0)
			{
				myTime+=nextTC;
				printLogHTML(HTMLLOG_COLOR_GREEN,"Timemanager - Added %u ms for the next time control\n",nextTC);
			}
		}
		//Time used up
		if(((unsigned)timeToUse)<=myTime) myTime-=timeToUse;
		else myTime=0;
	}

	printLogHTML(HTMLLOG_COLOR_GREEN,"Timemanager - estimated myTime after play %u\n",myTime);

}

bool timeUp()
{
	if(infTime) return false;
	return (bool)(timeUsed() >= (endTime - startTime));
}

time_ms timeUsed()
{
	return getms()-startTime;
}

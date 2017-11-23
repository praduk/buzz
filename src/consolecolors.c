/*******************************************************************************
 * CONSOLECOLORS.C                                                             *
 *                                                                             *
 * Lets you output normal, reverse, red, blue, green and intensified text into *
 * into the console.                                                           *
 *                                                                             *
 * Copyright (C) 2007 Pradu Kannan                                            *
 ******************************************************************************/

#if defined(_WIN32) || defined(_WIN64)
	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
#endif

#include <stdio.h>
#include "consolecolors.h"

int isAttachedToConsole()
{
	#if defined(_WIN32) || defined(_WIN64)
		DWORD dwConsoleMode/*, dwAvailable*/;
		HANDLE hInput;
		hInput = GetStdHandle(STD_INPUT_HANDLE);
		if(!GetConsoleMode(hInput,&dwConsoleMode))
     		return /*PeekNamedPipe(hInput,0,0,0,&dwAvailable,0)==0*/ 0;
		else
			return 1;
	#else 
		#if defined(_POSIX)
			return isatty(fileno(stdin));
		#else
			return 0;
		#endif
	#endif
}

//Normal
void CC_Normal()
{
	if(isAttachedToConsole())
	{
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
		#endif

		#ifdef _POSIX
			printf("\033[0m");
		#endif
		;
	}
}

//Reverse Video
void CC_ReverseVideo()
{
    if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE);
		#endif

		#ifdef _POSIX
			printf("\033[7m");
		#endif
		;
	}
}

//Red Text
void CC_Red()
{
    if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED);
		#endif

		#ifdef _POSIX
			printf("\033[31m");
		#endif
		;
	}
}

//Green Text
void CC_Green()
{
    if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN);
		#endif

		#ifdef _POSIX
			printf("\033[32m");
		#endif
		;
	}
}

//Blue Text
void CC_Blue()
{
	if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE);
		#endif

		#ifdef _POSIX
			printf("\033[34m");
		#endif
		;
	}
}


/* INTENSIFIED VERSIONS */

//Normal
void CC_INormal()
{
	if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		#endif

		#ifdef _POSIX
			printf("\033[0;1m");
		#endif
		;
	}
}

//Reverse Video
void CC_IReverseVideo()
{
    if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY);
		#endif

		#ifdef _POSIX
			printf("\033[7;1m");
		#endif
		;
	}
}

//Red Text
void CC_IRed()
{
    if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_RED|FOREGROUND_INTENSITY);
		#endif

		#ifdef _POSIX
			printf("\033[31;1m");
		#endif
		;
	}
}

//Green Text
void CC_IGreen()
{
    if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		#endif

		#ifdef _POSIX
			printf("\033[32;1m");
		#endif
		;
	}
}

//Blue Text
void CC_IBlue()
{
    if(isAttachedToConsole())
	{
		CC_Normal();
		#if defined(_WIN32) || defined(_WIN64)
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		#endif

		#ifdef _POSIX
			printf("\033[34;1m");
		#endif
		;
	}
}


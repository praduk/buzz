/**
*Buzz Chess Engine
*log.c
*
*Logging implementation mainly for the purposes of debugging.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "thread.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

FILE* logfile=NULL;
Mutex logLock;

void startLogHTML()
{
	if(logfile==NULL)
	{
		#ifndef MULTIPLE_LOG_FILES
			logfile=fopen(HTMLLOG_FILENAME,"wt");
		#else
			char filename[512];
			time_t t_t = time(NULL);
			struct tm* t=localtime(&t_t);
			sprintf(filename,"%02d%02d%02d_%02d%02d%02d_" HTMLLOG_FILENAME,
				t->tm_mday,t->tm_mon,t->tm_year%100,
				t->tm_hour,t->tm_min,t->tm_sec);
			logfile=fopen(filename,"wt");	
		#endif
		if(logfile==NULL) return;
	}
	setbuf(logfile, NULL);
	fprintf(logfile,
		"<html>"
		"<head>"
		stringer(<meta HTTP-EQUIV="content-type" CONTENT="text/html; charset=UTF-8">)
		"<title>"PROGRAM_VERSION_STRING" Logfile</title>"
		"</head>"
		"<body>"
		"<font face=\"courier\">"
		);
}

//for the moment unsafe, but usable
void printLogHTML(unsigned char flags, const char* string, ...)
{
	va_list ap;
	LockM(logLock);
	va_start(ap, string);
	vprintLogHTML(flags,string,ap);
	va_end(ap);
	ReleaseM(logLock);
}

void vprintLogHTML(unsigned char flags, const char* string, va_list ap)
{
	char tmp[MAX_CHARS_IN_ONE_PRINT];
	char* tmp2;
	if(logfile==NULL) return;
	//first replace all \n in the string with <br>
	strcpy(tmp,string);
	while((tmp2=strrchr(tmp,0xA)))
	{
		char* nullterm=strchr(tmp,0x0);
		while(nullterm!=tmp2)
		{
			*(nullterm+3)=*nullterm;
			nullterm--;
		}
		tmp2[0]='<';
		tmp2[1]='b';
		tmp2[2]='r';
		tmp2[3]='>';
	}
	//then replace all spaces with &nbsp;
	while((tmp2=strrchr(tmp,' ')))
	{
		char* nullterm=strchr(tmp,0x0);
		while(nullterm!=tmp2)
		{
			*(nullterm+5)=*nullterm;
			nullterm--;
		}
		tmp2[0]='&';
		tmp2[1]='n';
		tmp2[2]='b';
		tmp2[3]='s';
		tmp2[4]='p';
		tmp2[5]=';';
	}
	//print the opening tags
	switch(flags&HTMLLOG_COLOR_MASK)
	{
	case HTMLLOG_COLOR_BLACK: break;
	case HTMLLOG_COLOR_RED: fprintf(logfile,stringer(<font color="red">)); break;
	case HTMLLOG_COLOR_GREEN: fprintf(logfile,stringer(<font color="green">)); break;
	case HTMLLOG_COLOR_BLUE: fprintf(logfile,stringer(<font color="blue">)); break;
	}
	if(flags&HTMLLOG_ITLAIC) fprintf(logfile,"<i>");
	if(flags&HTMLLOG_BOLD) fprintf(logfile,"<b>");
	if(flags&HTMLLOG_UNDERLINE) fprintf(logfile,"<u>");

	//print the contents of the string
	vfprintf(logfile,tmp,ap);

	//print the closing tags
	switch(flags&HTMLLOG_COLOR_MASK)
	{
	case HTMLLOG_COLOR_BLACK: break;
	case HTMLLOG_COLOR_RED:
	case HTMLLOG_COLOR_GREEN:
	case HTMLLOG_COLOR_BLUE: fprintf(logfile,"</font>"); break;
	}
	if(flags&HTMLLOG_ITLAIC) fprintf(logfile,"</i>");
	if(flags&HTMLLOG_BOLD) fprintf(logfile,"</b>");
	if(flags&HTMLLOG_UNDERLINE) fprintf(logfile,"</u>");
}

void stopLogHTML()
{
	if(logfile==NULL) return;
	fprintf(logfile,
		"</body>\n"
		"</html>\n"
		);
	fclose(logfile);
	logfile=NULL;
}

/**
*Buzz Chess Engine
*log.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "defs.h"
#include <stdio.h>
#include <stdarg.h>

/*
*Definitions
*/

#define MULTIPLE_LOG_FILES
#define HTMLLOG_FILENAME "log.html" /*also the suffix for multiple log files*/
#define MAX_CHARS_IN_ONE_PRINT 8192



#define HTMLLOG_COLOR_MASK  3
#define HTMLLOG_COLOR_BLACK 0
#define HTMLLOG_COLOR_RED   1
#define HTMLLOG_COLOR_GREEN 2
#define HTMLLOG_COLOR_BLUE  3

#define HTMLLOG_ITLAIC      4
#define HTMLLOG_BOLD        8
#define HTMLLOG_UNDERLINE  16

/*
*Function Prototypes
*/

void startLogHTML();
void stopLogHTML();
void printLogHTML(unsigned char flags, const char* string, ...);
void vprintLogHTML(unsigned char flags, const char* string, va_list ap);

/*
*Global Data
*/

extern FILE* logfile;

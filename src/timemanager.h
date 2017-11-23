/**
*Buzz Chess Engine
*timemanager.h
*
*/

#include <time.h>
#include "defs.h"

/*
*definitions
*/

#define TIME_DIVIDER 30    /*how many moves do you expect to play*/
#define BUFFER_TIME  1000  /*milli-seconds*/
#define BUFFER_MPS   2     /*buffer extra moves per time control*/
#define BUFFER_INC   100   /*milli-seconds*/

/*
*Global Data
*/

extern unsigned int st;
extern unsigned int mps;
extern unsigned int inc;
extern unsigned int myTime;
extern unsigned int opponentTime;
extern unsigned int nextTC;

/*
*Function Prototypes
*/

typedef unsigned int time_ms;
time_ms getms();
void infiniteTime();
void finiteTime();
void setStartTime();
void setEndTime();
bool timeUp();
time_ms timeUsed();

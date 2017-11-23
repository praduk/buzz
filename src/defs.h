/**
*Buzz Chess Engine
*defs.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _defs
#define _defs

#include <assert.h>

/****************************BUILD CONFIGURATION*******************************/

#ifdef ANDRES_EVAL
	#define PROGRAM_VERSION_STRING "Dirty " __DATE__
#else
//#define PROGRAM_VERSION_STRING "Buzz 1.0"
#define PROGRAM_VERSION_STRING "Buzz " __DATE__ /*Used for beta builds*/
#endif
#define COPYRIGHT_YEAR_STRING "2007"

/*****************************TYPE DEFENITIONS*********************************/

//define boolean types and constants
typedef unsigned char bool;
#define true	1
#define false   0

//defining unsigned integer types

#ifndef __64_BIT_INTEGER_DEFINED__
	#define __64_BIT_INTEGER_DEFINED__
	#if defined(_MSC_VER) && _MSC_VER<1300
		typedef unsigned __int64 U64; //For the old microsoft compilers
	#else
		typedef unsigned long long  U64; //Supported by MSC 13.00+ and C99
	#endif //defined(_MSC_VER) && _MSC_VER<1300
#endif //__64_BIT_INTEGER_DEFINED__

//for 64 bit constants
#ifndef C64
	#if (!defined(_MSC_VER) || _MSC_VER>1300)
		#define C64(constantU64) constantU64##ULL
	#else
		#define C64(constantU64) constantU64
	#endif
#endif

#ifndef INLINE
	#ifdef _MSC_VER
		#define INLINE __forceinline
	#elif defined(__GNUC__)
		#define INLINE __inline__ __attribute__((always_inline))
	#else
		#define INLINE inline
	#endif
#endif

//defining full and empty U64 numbers
#define U64FULL     C64(0xFFFFFFFFFFFFFFFF)
#define U64EMPTY    C64(0x0000000000000000)

//This is used in a number of independant modules
//So it is placed here
#define MAX_SEARCH_DEPTH 64

#define xstringer(x) stringer(x)
#define stringer(x) #x

#ifdef _MSC_VER
	#pragma warning( disable : 4706)
	#pragma warning( disable : 4204)
	#pragma warning( disable : 4100)
#endif

#endif //_defs

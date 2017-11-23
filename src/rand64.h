/**
*Buzz Chess Engine
*rand64.h
*
*64-bit random number generator
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _rand64h
#define _rand64h

#ifndef C64
	#if (!defined(_MSC_VER) || _MSC_VER>1300)
		#define C64(constantU64) constantU64##ULL
	#else
		#define C64(constantU64) constantU64
	#endif
#endif

#ifndef __64_BIT_INTEGER_DEFINED__
#define __64_BIT_INTEGER_DEFINED__
#if defined(_MSC_VER) && _MSC_VER<1300
typedef unsigned __int64 U64; //For the old microsoft compilers
#else
typedef unsigned long long  U64; //Supported by MSC 13.00+ and GCC
#endif //defined(_MSC_VER) && _MSC_VER<1300
#endif //__64_BIT_INTEGER_DEFINED__

/*
*Function Prototypes
*/

void seed();
U64 rand64();


#endif //rand64h

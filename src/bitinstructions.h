/**
*Buzz Chess Engine
*bitinstructions.h
*
*LSB(X)
*MSB(X)
*toBit(X)
*toIndex(X)
*FirstOne(X)
*LastOne(X)
*removeLSB(X)
*popcnt(X)
*iterative_popcount(X)
*non_iterative_popcount(X)
*
*Copyright (C) 2007 Pradu Kannan
**/

#ifndef _bitinstructionsh
#define _bitinstructionsh

/*
*Build Definitions
*/

//define what your machine supports
#define SUPPORTS_TWOS_COMPLEMENT /*Negation (~X+1)==-X*/
#define HAS_FAST_MULTIPLICATION

#ifndef INLINE
	#ifdef _MSC_VER
		#define INLINE __forceinline
	#elif defined(__GNUC__)
		#define INLINE __inline__ __attribute__((always_inline))
	#else
		#define INLINE inline
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


#ifdef _MSC_VER
	#include <intrin.h>
	#pragma message("MSC compatible compiler detected -- turning off warning 4146")
	#pragma warning( disable : 4146)
	#ifdef _WIN64
		#pragma intrinsic(_BitScanForward64)
		#pragma intrinsic(_BitScanReverse64)
		#define USE_PROCESSOR_INSTRUCTIONS
		#define USING_INTRINSICS
	#endif
#elif defined(__GNUC__) && defined(__LP64__)
	static INLINE unsigned char _BitScanForward64(unsigned int* const Index, const U64 Mask)
	{
		U64 Ret;
		__asm__
		(
			"bsfq %[Mask], %[Ret]"
			:[Ret] "=r" (Ret)
			:[Mask] "mr" (Mask)
		);
		*Index = (unsigned int)Ret;
		return Mask?1:0;
	}
	static INLINE unsigned char _BitScanReverse64(unsigned int* const Index, const U64 Mask)
	{
		U64 Ret;
		__asm__
		(
			"bsrq %[Mask], %[Ret]"
			:[Ret] "=r" (Ret)
			:[Mask] "mr" (Mask)
		);
		*Index = (unsigned int)Ret;
		return Mask?1:0;
	}
	#define USING_INTRINSICS
#endif

//default popcnt - iterative_popcount or non_iterative_popcount
#define popcnt(X) iterative_popcount(X)

/*
*Defenitions
*/

#ifndef C64
	#if (!defined(_MSC_VER) || _MSC_VER>1300)
		#define C64(constantU64) constantU64##ULL
	#else
		#define C64(constantU64) constantU64
	#endif
#endif

#ifdef SUPPORTS_TWOS_COMPLEMENT
	#define LSB(X) ((X)&-(X))
#else
	#define LSB(X) ((X)&(~(X)+1))
#endif

#define removeLSB(X) (X&=X-1)
#define toBit(X) toBitDB[X]

#ifdef USING_INTRINSICS
	static INLINE int LastOne(U64 x)
	{
		int ret;
		_BitScanReverse64(&ret,x);
		return ret;
	}
	static INLINE int FirstOne(U64 x)
	{
		int ret;
		_BitScanForward64(&ret,x);
		return ret;
	}
	#define toIndex(X) FirstOne(X)
#else
	#define LastOne(X) (toIndex(MSB(X)))
	#define FirstOne(X) (toIndex(LSB(X)))
	#define toIndex(X) toIndexDB[(X)*C64(0x07EDD5E59A4E28C2)>>58]
#endif


#ifndef USING_INTRINSICS
	#define GetBitAndClear(bb, ret) U64 tmpgbac=(bb&-bb); \
		bb^=tmpgbac; \
		ret=toIndex(tmpgbac)
	#define GetBitAndClearOther(bb, clearbb, ret) U64 tmpgbac=(bb&-bb); \
		clearbb^=tmpgbac; \
		ret=toIndex(tmpgbac)
#else //intrinsic functions
	#define GetBitAndClear(bb, ret) _BitScanForward64(&ret,bb); \
		bb^=toBit(ret);
	#define GetBitAndClearOther(bb, clearbb, ret) _BitScanForward64(&ret,bb); \
		clearbb^=toBit(ret);
#endif

//Default population count
#ifdef HAS_FAST_MULTIPLICATION
	#define non_iterative_popcount(X) fastmul_popcount(X)
#else
	#define non_iterative_popcount(X) slowmul_popcount(X)
#endif

/*
*Global Data
*/

extern const U64 toBitDB[64];
extern const int toIndexDB[64];

/*
*Inlined Functions
*/

static INLINE U64 MSB(U64 x)
{
	x|=x>>1;
	x|=x>>2;
	x|=x>>4;
	x|=x>>8;
	x|=x>>16;
	x|=x>>32;
	x=(x>>1)+1;
	return x;
}

static INLINE int iterative_popcount(U64 b)
{
    int n;
    for (n=0 ; b ; n++, removeLSB(b));
    return n;
}

//Hamming Weights - from Wikipedia
#define m1  C64(0x5555555555555555)
#define m2  C64(0x3333333333333333)
#define m4  C64(0x0F0F0F0F0F0F0F0F)
#define m8  C64(0x00FF00FF00FF00FF)
#define m16 C64(0x0000FFFF0000FFFF)
#define hFF C64(0xFFFFFFFFFFFFFFFF)
#define h01 C64(0x0101010101010101)

//This uses fewer arithmetic operations than any other known  
//implementation on machines with slow multiplication.
//It uses 17 arithmetic operations.
static INLINE int slowmul_popcount(U64 x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
    x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
    x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
    return (int)(x & 0xff);
}

//This uses fewer arithmetic operations than any other known  
//implementation on machines with fast multiplication.
//It uses 12 arithmetic operations, one of which is a multiply.
static INLINE int fastmul_popcount(U64 x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (int)((x * h01)>>56);  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}

//returns a bitboard with all bits above b filled up (discluding b)
static INLINE U64 fillUp(U64 b)
{
	b|=b<<8;
	b|=b<<16;
	return (b|(b<<32))<<8;
}

//returns a bitboard with all bits below b filled down (discluding b)
static INLINE U64 fillDown(U64 b)
{
	b|=b>>8;
	b|=b>>16;
	return (b|(b>>32))>>8;
}

//returns a bitboard with all bits above b filled up (including b)
static INLINE U64 fillUp2(U64 b)
{
	b|=b<<8;
	b|=b<<16;
	return b|(b<<32);
}

//returns a bitboard with all bits below b filled down (including b)
static INLINE U64 fillDown2(U64 b)
{
	b|=b>>8;
	b|=b>>16;
	return b|(b>>32);
}

//Gerd-Isenberg's Carry Propogate Fill
static INLINE U64 fillRight(U64 pieces, U64 occ)
{
	const U64 H  =  C64(0x8080808080808080);
	U64 piecesNH = pieces&~H;
	U64 inclRook =  pieces | H | occ;
	U64 exclRook = (piecesNH) ^ inclRook;
	U64 attacks  = (exclRook - piecesNH) ^ inclRook;
	return attacks|pieces; 
}

//Kogge-Stone Fills from Stefan Wescott
/**
The routine FillUpOccluded() smears the set bits of bitboard g upwards, but only
along set bits of p; a reset bit in p is enough to halt a smear.
*/ 
static INLINE U64 fillUpOccluded(U64 g, U64 p)
{
	g |= p & (g << 8);
	p &=     (p << 8);
	g |= p & (g << 16);
	p &=     (p << 16);
	return g |= p & (g << 32);
}

static INLINE U64 fillDownOccluded(U64 g, U64 p)
{
	g |= p & (g >> 8);
	p &=     (p >> 8);
	g |= p & (g >> 16);
	p &=     (p >> 16);
	return g |= p & (g >> 32);
}

static INLINE U64 fillRightOccluded(U64 g, U64 p)
{
	p &= C64(0xFEFEFEFEFEFEFEFE);
	g |= p & (g << 1);
	p &=     (p << 1);
	g |= p & (g << 2);
	p &=     (p << 2);
	return g |= p & (g << 4);
}

static INLINE U64 fillLeftOccluded(U64 g, U64 p)
{
	p &= C64(0x7F7F7F7F7F7F7F7F);
	g |= p & (g >> 1);
	p &=     (p >> 1);
	g |= p & (g >> 2);
	p &=     (p >> 2);
	return g |= p & (g >> 4);
}

static INLINE U64 fillUpRightOccluded(U64 g, U64 p)
{
	p &= C64(0xFEFEFEFEFEFEFEFE);
	g |= p & (g <<  9);
	p &=     (p <<  9);
	g |= p & (g << 18);
	p &=     (p << 18);
	return g |= p & (g << 36);
}

static INLINE U64 fillUpLeftOccluded(U64 g, U64 p)
{
	p &= C64(0x7F7F7F7F7F7F7F7F);
	g |= p & (g <<  7);
	p &=     (p <<  7);
	g |= p & (g << 14);
	p &=     (p << 14);
	return g |= p & (g << 28);
}

static INLINE U64 fillDownRightOccluded(U64 g, U64 p)
{
	p &= C64(0xFEFEFEFEFEFEFEFE);
	g |= p & (g >>  7);
	p &=     (p >>  7);
	g |= p & (g >> 14);
	p &=     (p >> 14);
	return g |= p & (g >> 28);
}

static INLINE U64 fillDownLeftOccluded(U64 g, U64 p)
{
	p &= C64(0x7F7F7F7F7F7F7F7F);
	g |= p & (g >>  9);
	p &=     (p >>  9);
	g |= p & (g >> 18);
	p &=     (p >> 18);
	return g |= p & (g >> 36);
}

//For convienence during pawn eval
static INLINE U64 fillUpOcc(U64 pieces, U64 occ)
{
	occ=~(occ&~pieces);
	return fillUpOccluded(pieces,occ<<8);
}

//For convienence during pawn eval
static INLINE U64 fillDownOcc(U64 pieces, U64 occ)
{
	occ=~(occ&~pieces);
	return fillDownOccluded(pieces,occ>>8);
}

#endif //_bitinstructionsh


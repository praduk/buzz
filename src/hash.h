/**
*Buzz Chess Engine
*hash.h
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#ifndef _hashh
#define _hashh

#include "defs.h"
#include "board.h"
#include "thread.h"

/*
*Build Configuration
*/

//#define REPLACE_ALWAYS
#define REPLACE_OLDENTRIES_ALWAYS
#define REPLACE_BY_DEPTH
//#define REPLACE_BY_BOUND
#define NUM_PROBES 2

#if !defined(REPLACE_ALWAYS) && !defined(REPLACE_BY_DEPTH)
	#define REPLACE_ALWAYS
#endif

/*
*Defenitions
*/

#define DEFAULT_HASH_TABLE_SIZE (64*1024*1024)
#define INVALID_HASH_KEY C64(0x1234567890ABCDEF)

typedef struct _hashEntry /*16 bytes*/
{
	U64 hashkey;
	unsigned int data; //Format of data will follow
	short score;
	short depth;
}hashEntry;

/*********************Format of hash data**************************
Bits    (27)     0-26     are for the "move data".
Bits     (2)    27-28     are for the "hash flag"
Bits     (3)    29-31     are for the "search id"
******************************************************************/

#define encodeMove(m)       stripMove(m)
#define encodeFlag(f)       (((unsigned int)f)<<27)
#define encodeSearchID(id)  (((unsigned int)id)<<29)

#define extractMove(data)      ((data)&0x7FFFFFF)
#define extractFlag(data)      (((data)>>27)&0x3)
#define extractSearchID(data)  ((data)>>29)

#define MAX_SEARCHID           0x7
#define SEARCH_ID_BITS         3
#define clearSearchID(data)    (data)&=~(MAX_SEARCHID<<29)

#define lockKey(hashkey,data,score,depth) (((U64)(hashkey))^(((U64)(data))<<32)^(((U64)(score))<<16)^(((U64)(depth))))
#define unlockKey(hashkey,data,score,depth) lockKey(hashkey,data,score,depth)

//The flags
#define HASHFLAG_EXACT 0
#define HASHFLAG_LOWER_BOUND 1
#define HASHFLAG_UPPER_BOUND 2
#define HASHFLAG_NONE        3
#define HashFlagToString(HF) HF_to_String[HF]

typedef struct _perftEntry /*16 bytes*/
{
	U64 hashkey;
	U64 data;
}perftEntry;

/*********************Format of hash data**************************
Bits    (56)     0-55     are for the "node count".
Bits     (8)    56-63     are for the "perft depth"
******************************************************************/

#define encodePerftNodes(nodes) (((U64)(nodes))&C64(0xFFFFFFFFFFFFFF))
#define encodePerftDepth(depth) (((U64)(depth))<<56)

#define extractPerftNodes(data) ((data)&C64(0xFFFFFFFFFFFFFF))
#define extractPerftDepth(data) ((data)>>56)

/*
*Function Prototypes
*/

void initHashTable(unsigned int bytes);
void resetHashTable();

void newSearchID();
void storeRootHash(U64 hashKey, move m, int alpha, int beta, int score, int depth, int dfr);
void storeHash(U64 hashkey, move m, int alpha, int beta, int score, int depth, int dfr);
bool probeHash(U64 hashKey, hashEntry* copy);

void storePerftHash(const U64 hashKey, const U64 nodes, int depth);
perftEntry* probePerftHash(const U64 hashKey);

double HashTableUsage();

/*
*Global Data
*/

extern volatile hashEntry* hashTable;
extern Mutex rootEntryLock;
extern const char HF_to_String[][12];

#endif

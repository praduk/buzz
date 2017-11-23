/**
*Buzz Chess Engine
*hash.c
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "hash.h"
#include "eval.h"
#include "xboard.h"
#include "board.h"
#include "bitinstructions.h"
#include <stdlib.h>
#include <assert.h>

volatile hashEntry* hashTable=NULL;
volatile hashEntry rootEntry;
Mutex rootEntryLock;
unsigned int HASH_SIZE; //number of hash-table entries
unsigned int searchID=0;
const char HF_to_String[][12] = {"Exact","Lower Bound", "Upper Bound","None"}; 

void initHashTable(unsigned int bytes)
{
	rootEntry.data=0;
	rootEntry.score=0;
	rootEntry.hashkey=U64EMPTY;
	rootEntry.depth=-1;
	if(hashTable!=NULL)
	{
		free((void*)hashTable);
		hashTable=NULL;
	}
	//make a hashtable of a prime size to get the best hashing distribution
	{
		bool notdone=true;
		HASH_SIZE=bytes/sizeof(hashEntry);
		if(HASH_SIZE<=2)
		{
			HASH_SIZE=0;
			return;
		}
		//reduce HASH_SIZE to a prime number
		{
			if(HASH_SIZE%2==0) HASH_SIZE--;
			while(notdone && HASH_SIZE>2)
			{
				unsigned int i;
				unsigned int end;
				//find the square-root of HASH_SIZE with unit precision
				{
					double root=HASH_SIZE/2.0;
					double root2=HASH_SIZE;
					while((unsigned int)(root2)!=(unsigned int)(root))
					{
						root2=root;
						root=(root+(HASH_SIZE/root))/2.0;
					}
					end=(unsigned int)(root);
				}
				notdone=false;
				for(i=3;i<end;i+=2)
					if(HASH_SIZE%i==0)
					{
						notdone=true;
						break;
					}
				if(notdone) HASH_SIZE-=2;
			}
		}
	}
	hashTable=(volatile hashEntry*)malloc(HASH_SIZE*sizeof(hashEntry));
	resetHashTable();
}

void resetHashTable()
{
	unsigned int i;
	searchID=0;
	assert(hashTable!=NULL);
	for(i=0;i<HASH_SIZE;i++)
	{
		hashTable[i].data=0;
		hashTable[i].score=0;
		hashTable[i].hashkey=INVALID_HASH_KEY;
		hashTable[i].depth=-1;
	}
	rootEntry.data=0;
	rootEntry.score=0;
	rootEntry.hashkey=INVALID_HASH_KEY;
	rootEntry.depth=-1;
}

void newSearchID()
{
	if(searchID>=MAX_SEARCHID) searchID=0;
	else searchID++;
}

//Storing the root entry so that the hash table will always have a root entry
void storeRootHash(U64 hashKey, move m, int alpha, int beta, int score, int depth, int dfr)
{
	//Store in the regular hash-table as well
	storeHash(hashKey, m, alpha, beta, score, depth, dfr);

	//Storing for the root entry
	LockM(rootEntryLock);
	{
		unsigned int flag;

		if(score>=beta) flag = HASHFLAG_LOWER_BOUND;
		else if(score<=alpha) flag = HASHFLAG_UPPER_BOUND;
		else flag = HASHFLAG_EXACT;

		if(score>=MATE_MIN) score+=dfr;
		else if(score<=-MATE_MIN) score-=dfr;

		if(rootEntry.hashkey == hashKey
			&& rootEntry.depth == (short)depth
			&& flag == ((unsigned int)HASHFLAG_LOWER_BOUND)
			&& ((short)score) < rootEntry.score)
		{
			ReleaseM(rootEntryLock);
			return;
		}

		rootEntry.hashkey=hashKey;
		rootEntry.depth=((short)depth);
		rootEntry.score=((short)score);
		rootEntry.data=encodeMove(m)|encodeSearchID(searchID)|encodeFlag(flag);
	}
	ReleaseM(rootEntryLock);
}

//Bigger index means better hash entry
unsigned int hashEntryValue(short depth, unsigned int currID, unsigned int flag)
{
	unsigned int index = 0;
	#ifdef REPLACE_OLDENTRIES_ALWAYS
		index<<=SEARCH_ID_BITS;
		index|= MAX_SEARCHID-(searchID<=currID?currID-searchID:MAX_SEARCHID-(searchID-currID));
	#endif
	#ifdef REPLACE_BY_DEPTH
		index<<=16;
		if(depth>0)
			index|=(unsigned int)(depth);
	#endif
	#ifdef REPLACE_BY_BOUND
		index<<=2;
		index|=2-flag;
	#endif
	return index;
}

void storeHash(U64 hashKey, move m, int alpha, int beta, int score, int depth, int dfr)
{
	unsigned int i;

	volatile hashEntry* entry[NUM_PROBES]; //pointers to entries in the hash table
	hashEntry copy[NUM_PROBES];
	U64 unlockedHashKey[NUM_PROBES];
	unsigned int flag;

	assert(hashTable!=NULL);
	assert(m!=NULLMOVE);
	assert(alpha<beta);
	assert(depth>0);

	if(score>=beta) flag = HASHFLAG_LOWER_BOUND;
	else if(score<=alpha) flag = HASHFLAG_UPPER_BOUND;
	else flag = HASHFLAG_EXACT;

	if(score>=MATE_MIN) score+=dfr;
	else if(score<=-MATE_MIN) score-=dfr;
	
	assert(depth>=-32768 && depth<=32767);
	assert(score>=-32768 && score<=32767);


	//Initialize data and search for the same position
	for(i=0;i<NUM_PROBES;i++)
	{
		entry[i]=hashTable+(hashKey+i)%HASH_SIZE;
		copy[i] = *entry[i];
		unlockedHashKey[i] = unlockKey(copy[i].hashkey,copy[i].data,copy[i].score,copy[i].depth);
		if(unlockedHashKey[i]==hashKey) //Replace the same position always
		{
			/*switch(extractFlag(copy[i].data))
			{
			case HASHFLAG_LOWER_BOUND:
				if(!(depth!=copy[i].depth || flag!=HASHFLAG_UPPER_BOUND || score>=copy[i].score))
				{
					print("Depth = %d\n",depth);
					print("Flag  = %d\n",flag);
					print("score >= copy[i].score ==> %d >= %d\n",score,copy[i].score);
					MilliSleep(1000);
				}
				break;
			case HASHFLAG_UPPER_BOUND:
				if(!(depth!=copy[i].depth || flag!=HASHFLAG_LOWER_BOUND || score<=copy[i].score))
				{
					print("Depth = %d (%d)\n",depth,copy[i].depth);
					print("Flag  = %s (%s)\n",HashFlagToString(flag),HashFlagToString(extractFlag(copy[i].data)));
					print("score <= copy[i].score ==> %d <= %d\n",score,copy[i].score);
					MilliSleep(1000);
				}
				//assert(depth!=copy[i].depth || flag!=HASHFLAG_LOWER_BOUND || score<=copy[i].score);
				break;
			}*/
			{
				unsigned int data = encodeMove(m)|encodeSearchID(searchID)|encodeFlag(flag);
				entry[i]->hashkey=lockKey(hashKey,data,((short)score),((short)depth));
				entry[i]->depth=((short)depth);
				entry[i]->score=((short)score);
				entry[i]->data=data;
				return;
			}
		}
	}

	{
		unsigned int replacementWeight = (unsigned int)(-1);
		#ifdef REPLACE_ALWAYS
		unsigned int replacementIndex = 0;
		#else
		unsigned int replacementIndex = NUM_PROBES;
		#endif

		for(i=0;i<NUM_PROBES;i++)
		{
			//Corrupt Entry
			bool corrupt_entry = false;
			const U64 keyIndex_Min = (unlockedHashKey[i]%HASH_SIZE);
			const U64 keyIndex_Max = ((unlockedHashKey[i]+NUM_PROBES)%HASH_SIZE);
			const U64 realIndex = (hashKey+i)%HASH_SIZE;
			if(keyIndex_Min<keyIndex_Max)
				corrupt_entry = (bool)(realIndex < keyIndex_Min || realIndex >= keyIndex_Max);
			else
				corrupt_entry = (bool)(realIndex < keyIndex_Min && realIndex >= keyIndex_Max);
			if(corrupt_entry) 
			{
				replacementWeight = 0;
				replacementIndex = i;
				break;	
			}

			{
				unsigned int tempWeight;
				tempWeight = hashEntryValue((short)copy[i].depth,
					(unsigned int)extractSearchID(copy[i].data),(unsigned int)extractFlag(copy[i].data));
				if(tempWeight<replacementWeight)
				{
					replacementWeight=tempWeight;
					replacementIndex = i;
				}
			}
		}

		#ifndef REPLACE_ALWAYS
		if(replacementIndex!=NUM_PROBES && hashEntryValue((short)depth,searchID,flag)>=replacementWeight)
		#endif
		{
			unsigned int data = encodeMove(m)|encodeSearchID(searchID)|encodeFlag(flag);
			entry[replacementIndex]->hashkey=lockKey(hashKey,data,((short)score),((short)depth));
			entry[replacementIndex]->depth=((short)depth);
			entry[replacementIndex]->score=((short)score);
			entry[replacementIndex]->data=data;
		}
	}
		
}

//returns true on a hit, false for a no hit
bool probeHash(U64 hashKey, hashEntry* copy)
{
	
	U64 i;
	assert(hashTable!=NULL);
	
	if(rootEntry.hashkey==hashKey && extractMove(rootEntry.data)!=NULLMOVE)
	{
		*copy=rootEntry;
		return true;
	}

	for(i=0;i<NUM_PROBES;i++)
	{
		*copy=hashTable[(hashKey+i)%HASH_SIZE];
		if(unlockKey(copy->hashkey,copy->data,copy->score,copy->depth)==hashKey && extractMove(copy->data)!=NULLMOVE)
		{
			assert(copy->depth>0);
			return true;
		}
	}
	return false;
}

void storePerftHash(const U64 hashKey, const U64 nodes, int depth)
{
	perftEntry* entry;
	assert(hashTable!=NULL);
	entry=(perftEntry*)(hashTable+hashKey%HASH_SIZE);

	//if(depth>extractPerftDepth(entry->data))
	{
		entry->hashkey=hashKey;
		entry->data=encodePerftDepth(depth)|encodePerftNodes(nodes);
	}
}


perftEntry* probePerftHash(const U64 hashKey)
{
	perftEntry* entry;
	assert(hashTable!=NULL);
	entry=(perftEntry*)(hashTable+hashKey%HASH_SIZE);

	if(entry->hashkey==hashKey)
		return entry;
	return NULL;
}

double HashTableUsage()
{
	U64 count = 0;
	U64 i;
	assert(hashTable!=NULL);
	for(i=0;i<HASH_SIZE;i++)
	{
		volatile hashEntry* entry=hashTable+i;
		if(extractSearchID(entry->data)==searchID)
			count++;
	}
	return ((double)count)/((double)HASH_SIZE);
}


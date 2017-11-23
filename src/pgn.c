/**
*Buzz Chess Engine
*pgn.c
*
*A generic PGN reader that reads a "clean" PGN file.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "pgn.h"

#define PGN_DELIMITERS   " .\n" /*when reading moves*/

void resetPGNgame(PGNgame* const pgn)
{
    *pgn->event = '\0';
    *pgn->site  = '\0';
    *pgn->date  = '\0';
    *pgn->round = '\0';
    *pgn->white = '\0';
    *pgn->black = '\0';
    pgn->whiteRating = -1;
    pgn->blackRating = -1;
    pgn->result = RESULT_NONE;
	pgn->moves[0] = NULLMOVE;
	pgn->nummoves = 0;
}

//returns false on failure
bool getNextPGNGame(FILE* const f, PGNgame* const pgn)
{
	board pos = startPos;
	char line[PGN_LINE_LENGTH];
	
	//Clear the PGN
	resetPGNgame(pgn);
	
	//If the file pointer is not good, return failure
	if(f==NULL) return false;
	
	//Go to a header
	for(;;)
	{
		if(!fgets(line,PGN_LINE_LENGTH,f)) //if error in reading
			return false;
        if(*line=='[') break;
	}

	//read the header
	for(;;)
	{
		char identifier[PGN_LINE_LENGTH];
		if(!sscanf(line,"[%s \"%*[^\n\"]\"]",identifier)) return false;
		if(!strcmp(identifier,"Result"))
		{
			char result_string[PGN_LINE_LENGTH];
            if(!sscanf(line,"[%*s \"%[^\n\"]\"]",result_string)) return false;
            if(!strcmp(result_string,"1-0"))
                pgn->result = RESULT_WHITE_WINS;
			else if(!strcmp(result_string,"0-1"))
			    pgn->result = RESULT_BLACK_WINS;
            else if(!strcmp(result_string,"1/2-1/2"))
			    pgn->result = RESULT_DRAW;
			else
			    pgn->result = RESULT_NONE;
		}
		else if(!strcmp(identifier,"Event"))
			sscanf(line,"[%*s \"%[^\n\"]\"]",pgn->event);
        else if(!strcmp(identifier,"Site"))
			sscanf(line,"[%*s \"%[^\n\"]\"]",pgn->site);
        else if(!strcmp(identifier,"Date"))
			sscanf(line,"[%*s \"%[^\n\"]\"]",pgn->date);
        else if(!strcmp(identifier,"Round"))
			sscanf(line,"[%*s \"%[^\n\"]\"]",pgn->round);
        else if(!strcmp(identifier,"White"))
			sscanf(line,"[%*s \"%[^\n\"]\"]",pgn->white);
        else if(!strcmp(identifier,"Black"))
			sscanf(line,"[%*s \"%[^\n\"]\"]",pgn->black);
		else if(!strcmp(identifier,"WhiteElo") || !strcmp(identifier,"WhiteELO"))
		    sscanf(line,"[%*s \"%d\"]",&pgn->whiteRating);
        else if(!strcmp(identifier,"BlackElo") || !strcmp(identifier,"BlackELO"))
		    sscanf(line,"[%*s \"%d\"]",&pgn->blackRating);
		    
        if(!fgets(line,PGN_LINE_LENGTH,f)) //if error in reading
			return false;
		if(*line=='\n') break;
	}
	
	//Now read the movelist
	while(fgets(line,PGN_LINE_LENGTH,f))
	{
		const char* token = (const char*)strtok(line,PGN_DELIMITERS);
		for(;token!=NULL;token=(const char*)strtok(NULL,PGN_DELIMITERS))
		{
			if(*token=='\n' || !strcmp(token,"1-0") || !strcmp(token,"0-1") ||
				!strcmp(token,"1/2-1/2") || *token=='{' || *token=='(')
			    return true;
			else if(*token=='$' || isdigit(*token)) //NAGs
				continue;
			else if(*token==';' || *token==0x25) //line comment
				break;
			else if(isalpha(*token)) //move
			{
				move m=stringToMove(&pos,token);
				if(m==NULLMOVE) {printf("Illegal Move ply %d; %s\n",pgn->nummoves+1,token); return false;} //illegal move
				makemove(&pos,m);
				pgn->moves[pgn->nummoves] = m;
                pgn->nummoves++;
				pgn->moves[pgn->nummoves] = NULLMOVE;
			}
		}
	}

	return true; //All went well
}
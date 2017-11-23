/**
*Buzz Chess Engine
*book.c
*
*Book code for Buzz.
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "board.h"
#include "movegen.h"
#include "consolecolors.h"
#include "book.h"
#include "rand64.h"
#include "resultanalysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//PGN parsing states
#define BOOK_PGN_STATE_SEARCH 0 /*searching the pgn file for a game*/
#define BOOK_PGN_STATE_BLACK  1 /*the game being read is won by black*/
#define BOOK_PGN_STATE_WHITE  2 /*the game being read is drawn*/
#define BOOK_PGN_STATE_DRAW   3 /*the game being read is won by white*/
#define BOOK_PGN_STATE_NORES  4 /*the game being read has no result*/
#define BOOK_PGN_DELIMITERS   " .\n"

char bookPath[2048]=DEFAULT_BOOK_PATH;
FILE* bookFile=NULL;
FILE* pgnFile=NULL;
int pgnstate;
unsigned int gamenum;
unsigned int maxplys;

//retuns NULLMOVE if no book move was found
move getBookMove(const board* pos, char* info)
{
	if(info!=NULL)
		*info='\0';
	bookFile=fopen(bookPath,"rb");
	if(bookFile==NULL)
	{
		if(info!=NULL)
			sprintf(info,"%s not found",bookPath);
		return NULLMOVE;
	}
	
	fseek(bookFile,0,SEEK_END);
	{
		long numentries=ftell(bookFile)/sizeof(bookPos);
		moveList ml;
		bookPos bookDat[MAX_MOVECHOICES];
		double metric[MAX_MOVECHOICES];
		unsigned int i;
		bool have_book_move = false;

		genMoves(pos,&ml);
		if(ml.moveCount<2)
		{
			if(info!=NULL) strcpy(info,"book move unnecessary");
			return NULLMOVE;
		}
		
		//get data for each move from the book file using a binary search
		for(i=0;i<ml.moveCount;i++)
		{
			long left=0;
			long right=numentries-1;
			long mid=(left+right)/2;
			board tempPos=*pos;

			bookDat[i].numdraws = U64EMPTY;
			bookDat[i].numwins  = U64EMPTY;
			bookDat[i].numgames = U64EMPTY;

			makemove(&tempPos,ml.moves[i].m);
			while(left<=right)
			{
				bookPos bp;
				fseek(bookFile,mid*sizeof(bookPos),SEEK_SET);

				fread(&bp,sizeof(bp),1,bookFile);
				if(tempPos.hashkey>bp.hashkey)
					left=mid+1;
				else if(tempPos.hashkey<bp.hashkey)
					right=mid-1;
				else if(bp.hashkey==tempPos.hashkey)
				{
					bookDat[i]=bp;
					have_book_move = true;
					break;
				}
				mid=(left+right)/2;
			}
		}
		fclose(bookFile);

		if(!have_book_move)
		{
			if(info!=NULL) strcpy(info,"no book move");
			return NULLMOVE;
		}

		/************************** Create the Metric **************************/
		//This is the core of the move choosing mechanism
		//A higher metric value means that the move is more likely to be chosen
		{
			for(i=0;i<ml.moveCount;i++) metric[i]=0; //reset the metric

			#ifdef BOOK_METRIC_NORM
			for(i=0;i<ml.moveCount;i++)
				if(bookDat[i].numgames)
					metric[i]=((double)bookDat[i].numwins+0.5*bookDat[i].numdraws);
			#endif
		}

		/*for(i=0;i<ml.moveCount;i++)
		{
			sanString s;
			moveToSAN(pos,ml.moves[i].m,s);
			printf("%s (%f)\n",s,metric[i]);
		}
		printf("\n");*/

		/***********************************************************************/

		//normalize the metric (the one with the highest gets the highest probability)
		{
			double met_tot = 0;
			for(i=0;i<ml.moveCount;i++) met_tot+=metric[i];
			for(i=0;i<ml.moveCount;i++) metric[i]/=met_tot;
		}
		

		//sort move list
		for(i=ml.moveCount-1;i>0;i--)
		{
			unsigned int j;
			for(j=1;j<=i;j++)
				if(metric[j-1]<metric[j])
				{
					//swap the actual move, the book data, and the metric
					smove temp = ml.moves[j-1];
					bookPos temp2 = bookDat[j-1];
					double temp3 = metric[j-1];

					ml.moves[j-1]=ml.moves[j];
					ml.moves[j]=temp;

					bookDat[j-1]=bookDat[j];
					bookDat[j]=temp2;

					metric[j-1]=metric[j];
					metric[j]=temp3;
				}
		}

		//pick a move
		{
			double randomval=rand()/((double)RAND_MAX+1);
			double sum=0;

			for(i=0;i<ml.moveCount;i++)
			{
				sum+=metric[i];
				if(sum>=randomval)
					break;
			}
			if(info!=NULL)
			{
				unsigned int j;
				//Print Win, Draw, Loss
				for(j=0;j<ml.moveCount && bookDat[j].numgames;j++)
				{
					sanString str;
					char temp[2048];
					if(j) strcat(info," ");
					moveToSAN(pos,ml.moves[j].m,str);
					sprintf(temp,"%s(%0.f%%)",str,100*metric[j]);
					strcat(info,temp);
				}
			}
			return ml.moves[i].m;
		}
	}
}

//Buzz makebook <bookname> <plys> <razor %%> <pgnFile1> <pgnFile2> ... <pgnFileN>
void makebook(int argc, char* argv[])
{
	int i;
	int minGames;
	float razorPercent;
	
	argc-=6;
	if(argc<1)
	{
		printf("Not enough parameters\n");
		return;
	}

	//Book file path is arg[2]

	if(sscanf(argv[3],"%d",&maxplys)!=1)
	{
		printf("Bad ply parameter\n");
		return;
	}
	if(sscanf(argv[4],"%d",&minGames)!=1)
	{
		printf("Bad ply parameter\n");
		return;
	}
	if(sscanf(argv[5],"%f",&razorPercent)!=1)
	{
		printf("Bad razor parameter\n");
		return;
	}
	for(i=6; argc; argc--, i++)
	{
		char* pgnFilePath=argv[i];
		printf("\nReading %s\n",pgnFilePath);
		gamenum=0;
		pgnFile=fopen(pgnFilePath,"rt");
		if(pgnFile==NULL)
		{
			printf("Error: could not open %s\n",argv[i]);
			continue;
		}
		else
		{
			bookPosGen bkpos;
			//Parse PGN here
			pgnstate=BOOK_PGN_STATE_SEARCH;
			gamenum=0;
			while(getPGNBookPosition(&bkpos))
				rb_insert(&bkpos);
			fclose(pgnFile);
			pgnFile=NULL;
		}
	}
	//write to book
	
	{
		bookPosGen bkpos;
		printf("\nWriting to %s\n",argv[2]);
		bookFile=fopen(argv[2],"wb");
		if(bookFile==NULL)
		{
			printf("Error: could not open %s for writing\n",argv[2]);
			return;
		}
		{
			U64 positions=0;
			U64 razor=0;
			while(rb_getleast(&bkpos))
			{
				float scorepercent=(2*(float)bkpos.numwins+(float)bkpos.numdraws)/2/(float)bkpos.numgames*100;
				positions++;
				if(scorepercent>=razorPercent && bkpos.numgames>=((U64)minGames))
				{
					bookPos pos;
					pos.hashkey=bkpos.hashkey;
					pos.numgames=bkpos.numgames;
					pos.numdraws=bkpos.numdraws;
					pos.numwins=bkpos.numwins;
					fwrite(&pos,sizeof(pos),1,bookFile);
				}
				else
					razor++;
			}
			printf("Finished writing\n");
			printf("\nBook Statistics\n");
			printf("Number of Positions Read      = ");
			printU64(positions);
			printf("\n");
			printf("Number of Positions Discarded = ");
			printU64(razor);
			printf("\n");
			printf("Number of Positions in Book   = ");
			printU64(positions-razor);
			printf("\n");
			
		}
		fclose(bookFile);
		bookFile=NULL;
	}
}

bool getPGNBookPosition(bookPosGen* pos)
{
	static unsigned int plys;
	static board currentPos;
	static char line[2048];
	char* token;

	assert(pgnFile!=NULL);
	
	//searching for a new game
searchPGN:
	if(pgnstate==BOOK_PGN_STATE_SEARCH)
	{
		while(pgnstate==BOOK_PGN_STATE_SEARCH)
		{
			if(!fgets(line,2048,pgnFile)) //if error in reading
				return false;
			if(!strncmp(line,"[Result \"0-1\"]",14))
				pgnstate=BOOK_PGN_STATE_BLACK;
			else if(!strncmp(line,"[Result \"1-0\"]",14))
				pgnstate=BOOK_PGN_STATE_WHITE;
			else if(!strncmp(line,"[Result \"1/2-1/2\"]",18))
				pgnstate=BOOK_PGN_STATE_DRAW;
			else if(!strncmp(line,"[Result \"*\"]",18))
				pgnstate=BOOK_PGN_STATE_NORES;
		}
		//ignore all other tags and newlines
		for(;;)
		{
			if(!fgets(line,2048,pgnFile)) //if error in reading
				return false;
			if(*line!='[' && *line!='\n' && *line!=0x25)
				break;
		}
		currentPos=startPos;
		plys=0;
		gamenum++;
		token=strtok(line,BOOK_PGN_DELIMITERS);
		goto tokenize;
	}
	
	token=strtok(NULL,BOOK_PGN_DELIMITERS);
tokenize:
	for(;token!=NULL;token=strtok(NULL,BOOK_PGN_DELIMITERS))
	{
		if(*token=='\n' || !strcmp(token,"1-0") || !strcmp(token,"0-1") || !strcmp(token,"1/2-1/2") ||
			*token=='{' || *token=='(')
		{
			pgnstate=BOOK_PGN_STATE_SEARCH;
			goto searchPGN;
		}
		else if(*token=='$' || isdigit(*token))
			continue;
		else if(*token==';' || *token==0x25) //line comment
			break;
		else if(isalpha(*token)) //move
		{
			move m=stringToMove(&currentPos,token);
			plys++;
			if(m==NULLMOVE)
			{
				if(currentPos.side==WHITE)
					printf("Illegal Move: %d.%s in game %d\n",plys/2+1,token,gamenum);
				else
					printf("Illegal Move: %d...%s in game %d\n",plys/2,token,gamenum);
				pgnstate=BOOK_PGN_STATE_SEARCH;
				goto searchPGN;
			}
			makemove(&currentPos,m);
			pos->hashkey=currentPos.hashkey;
			pos->numgames=1;
			switch(pgnstate)
			{
			case BOOK_PGN_STATE_BLACK:
				pos->numdraws=0;
				pos->numwins=currentPos.side!=BLACK;
				break;
			case BOOK_PGN_STATE_WHITE:
				pos->numdraws=0;
				pos->numwins=currentPos.side!=WHITE;
				break;
			case BOOK_PGN_STATE_DRAW:
			case BOOK_PGN_STATE_NORES:
				pos->numdraws=1;
				pos->numwins=0;
			}
			if(plys>=maxplys) pgnstate=BOOK_PGN_STATE_SEARCH;
			return true;
		}
	}
	readagain:
	if(!fgets(line,2048,pgnFile)) //if error in reading
		return false;
	if(*line==0x25)
		goto readagain;
	token=strtok(line,BOOK_PGN_DELIMITERS);
	goto tokenize;
}

/***************************Red-black trees for book generation***************************/
//defines for the red-black trees for creating the book
typedef struct _rbNode
{
	bookPosGen bkpos;   //the book position data
	struct _rbNode* left;   //nodes with a key less than the current node
	struct _rbNode* right;  //nodes with a key greater than the current node
	struct _rbNode* parent; //the parent node
	bool color;             //the color, either red or black
}rbNode;

//the red-black tree
struct _rbTree
{
	rbNode* root;  //pointer to the root node
	int size;       //ponter to the number of elements
}rbTree;

//BLACK is already define, define RED
#define RED WHITE

//some macros used in the insertion code
#define grandparent(n) ((n)->parent->parent)
#define uncle(n) (((n)->parent==grandparent(n)->left)?grandparent(n)->right:grandparent(n)->left)

//gets the key of a red-black node
#define rbkey(n) ((n)->bkpos.hashkey)

//some prototypes
bool bianaryInsert(rbNode* root, rbNode* node);
void rbVerify(rbNode* n);
void rb_rotateLeft(rbNode* n);
void rb_rotateRight(rbNode* n);
bool rbGetLeast(rbNode* n, bookPosGen* bkpos);

/*rb_insert()
 *inserts a book position into the red-black tree
 */
void rb_insert(bookPosGen* bkpos)
{
	rbNode* newNode;

	newNode=malloc(sizeof(rbNode));
	newNode->bkpos=*bkpos;
	newNode->left=NULL;
	newNode->right=NULL;
	newNode->parent=NULL;
	newNode->color=RED;

	if(rbTree.root==NULL)
	{
		rbTree.root=newNode;
		rbTree.size++;
	}
	//if the node was found through traversal, no need to verify the tree
	else if(bianaryInsert(rbTree.root, newNode)) return;

	//verify the tree
	rbVerify(newNode);
}

/*rb_getleast()
 *encapsulator for rbGetLeast()
 */
bool rb_getleast(bookPosGen* bkpos)
{
	return rbGetLeast(rbTree.root,bkpos);
}

/*binaryInsert()
 *a binaray tree insertion of a node
 *NOTE: the red-black tree cannot be empty
 *
 *@paran root - the relative root node
 *@param node - the new node
 *
 *@returns true if the position was found in the tree through traversal
 */
bool bianaryInsert(rbNode* root, rbNode* node)
{
	if(root==NULL)
	{
		root=node;
		if(rbkey(root->parent)>rbkey(root))
			root->parent->left=root;
		else
			root->parent->right=root;
		rbTree.size++;
		return false;
	}
	if(rbkey(node)==rbkey(root))
	{
		root->bkpos.numgames+=node->bkpos.numgames;
		root->bkpos.numwins+=node->bkpos.numwins;
		root->bkpos.numdraws+=node->bkpos.numdraws;
		free(node);
		return true;
	}
	if(rbkey(node)<rbkey(root))
	{
		node->parent=root;
		return bianaryInsert(root->left,node);
	}
	node->parent=root;
	return bianaryInsert(root->right,node);
}

/*rbVerify()
 *verify's the red-black tree
 *
 *@param n - the node that was just inserted
 */
void rbVerify(rbNode* n)
{
	//The new node is at the root of the tree
	if(n->parent==NULL)
	{
		n->color=BLACK;
		return;
	}

	//The new node's parent is black
	if(n->parent->color==BLACK) return;

	//If both the parent and the uncle are red,
	//repaint them both black and repaint the grandparent red
	//treat the grandparent as the new node now
	if(uncle(n)!=NULL && uncle(n)->color==RED)
	{
		n->parent->color=BLACK;
		uncle(n)->color=BLACK;
		grandparent(n)->color=RED;
		rbVerify(grandparent(n));
		return;
	}

	if(n==n->parent->right && n->parent==grandparent(n)->left)
	{
		rb_rotateLeft(n->parent);
		if(n->left==NULL)
			printf("Error Emminent Stage 2\n");
		n=n->left;
	}
	else if(n==n->parent->left && n->parent==grandparent(n)->right)
	{
		rb_rotateRight(n->parent);
		if(n->right==NULL)
			printf("Error Emminent Stage 2\n");
		n=n->right;
	}

	n->parent->color=BLACK;
	grandparent(n)->color=RED;
	if(n==n->parent->left && n->parent==grandparent(n)->left)
		rb_rotateRight(grandparent(n));
	else rb_rotateLeft(grandparent(n));
}

/*rb_rotateLeft()
 *rotates the red-black tree at a node left
 *
 *    n               C
 *   / \             / \
 *  /   \           /   \
 * A     C   --->  n     D
 *      / \       / \
 *     B   D     A   B
 *
 *@param n - the pivot of rotation
 */
void rb_rotateLeft(rbNode* n)
{
	n->right->parent=n->parent;
	if(n->parent!=NULL)
	{
		if(n->parent->right==n)
			n->parent->right=n->right;
		else n->parent->left=n->right;
	}
	else
		rbTree.root=n->right;
	n->parent=n->right;
	n->right=n->parent->left;
	if(n->right!=NULL) n->right->parent=n;
	n->parent->left=n;
	//printf("RotL\n");
}

/*rb_rotateRight()
 *rotates the red-black tree at a node right
 *
 *      n            B
 *     / \          / \
 *    /   \        /   \
 *   B     D ---> A     n
 *  / \                / \
 * A   C              C   D
 *
 *@param n - the pivot of rotation
 */
void rb_rotateRight(rbNode* n)
{
	n->left->parent=n->parent;
	if(n->parent!=NULL)
	{
		if(n->parent->left==n)
			n->parent->left=n->left;
		else n->parent->right=n->left;
	}
	else
		rbTree.root=n->left;
	n->parent=n->left;
	n->left=n->parent->right;
	if(n->left!=NULL) n->left->parent=n;
	n->parent->right=n;
	//printf("RotR\n");
}

/*rbGetLeast()
 *get the smallest bookPosGen in the red-black tree and deletes it
 *from the tree without verification
 *
 *@param n       - should be called with rbTree.root
 *@param bookPos - the pointer to the book Position
 *
 *@returns false if the tree is empty
 */
bool rbGetLeast(rbNode* n, bookPosGen* bkpos)
{
	if(n==NULL) return false;
	while(n->left!=NULL) n=n->left;
	*bkpos=n->bkpos;
	if(n->right!=NULL)
	{
		//if the node is the root node
		if(n->parent==NULL)
		{
			rbTree.root=n->right;
			rbTree.root->parent=NULL;
		}
		else
		{
			n->parent->left=n->right;
			n->right->parent=n->parent;
		}
	}
	else
	{
		if(n->parent==NULL) rbTree.root=NULL;
		else n->parent->left=NULL;
	}
	//free the pointer
	free(n);
	//n=NULL;
	return true;
}

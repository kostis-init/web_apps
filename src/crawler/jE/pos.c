#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "defs.h"

void PRecDel(struct PosNode* node)
{
	if(node->next!=NULL)
	{
		
		PRecDel(node->next);
	}
	free(node->pathname);free(node->lines);
	free(node);
	
}

void PosDelete(struct PosList* pos)
{
	PRecDel(pos->first);
	free(pos);
}

int PosInsert(struct PosList* pos,int line,char* pathname)
{
	struct PosNode* cur=pos->first;
	struct PosNode* previous=NULL;
	while(cur!=NULL)
	{
		if(!strcmp(pathname,cur->pathname))//found
		{
			cur->lines = realloc(cur->lines,sizeof(int)*(cur->lsz+1));
			cur->lines[cur->lsz]=line;
			cur->lsz++;
			//printf("ALREADY HERE tf=%d ",cur->tf);
			break;
		}
		previous=cur;//useful later, if docid not founds
		cur=cur->next;
	}
	if(cur==NULL)///not found, add pathname
	{
		cur=previous;///go back to add new node
		if((cur->next=malloc(sizeof(struct PosNode)))==NULL)
		{
			printf("PosInsert: malloc failure, exiting...\n");
			return -1;
		}
		cur->next->pathname=malloc((strlen(pathname)+1)*sizeof(char));
		strcpy(cur->next->pathname,pathname);
		cur->next->lines=malloc(sizeof(int));
		cur->next->lines[0]=line;
		cur->next->lsz=1;
		cur->next->next=NULL;
	}
	pos->wordfreq++;
	return 0;
}

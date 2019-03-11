#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "structs.h"
#include "defs.h"

int map_to_index(char** map,int sz,struct TrieRoot* trie,char* filename)
{
	/**INSERT TO TRIE*/
	for(int i=0; i<sz; i++)		//for every line
	{
		//printf("inserting line: %s\n",map[i]);
		char* temp=malloc((strlen(map[i])+1)*sizeof(char));
		strcpy(temp,map[i]);//make a copy of doc for strtok
		for(char* word=strtok(temp," \t"); word!=NULL; word=strtok(NULL, " \t"))//for every word
		{
			//printf("inserting word: %s\n",word);
			TrieInsert(trie,word,i,filename);
		}
		//printf("\r%d%%",(i+1)*100/sz);fflush(stdout);//show progress dynamically
		free(temp);
	}
	
	return 0;
}

int create_index(struct filelist* fl,struct TrieRoot* trie)
{
	/**INIT TRIE*/
	if(TrieInit(trie)<0) return -1;
	
	while(fl->next!=NULL)
	{
		//printf("%d::inserting pathname:%s:::map has %d lines\n",getpid(),fl->pathname,fl->nlines);
		map_to_index(fl->map,fl->nlines,trie,fl->pathname);
		fl=fl->next;
	}
}

void TriePrintAllDF(struct TrieNode* node,char* word,int depth)
{
	char* newword=malloc((depth+2)*sizeof(char));
	strcpy(newword,word);//keep current word
	
	while(node!=NULL)
	{
		newword[depth]=node->c;newword[depth+1]='\0';
		if(node->pos!=NULL)//we have a word, print it
			printf("%s %d\n",newword,node->pos->wordfreq);
		
		TriePrintAllDF(node->child,newword,depth+1);//go to next level
		
		newword[depth]='\0';//prepare the word for next char
		node=node->next;
	}
	free(newword);
}

void TRecDel(struct TrieNode* node)
{
	if(node->pos!=NULL)PosDelete(node->pos);
	if(node->child!=NULL || node->next!=NULL)
	{
		TRecDel(node->child);
		TRecDel(node->next);
	}
	free(node);
}

void TrieDelete(struct TrieRoot trie)
{
	TRecDel(trie.first);
}


int TrieInsert(struct TrieRoot* trie,char* word,int line,char* filename)
{
	struct TrieNode* cur = trie->first;//cur:	current node
	char c;
	int i=0;
	while(word[i]!='\0' && cur!=NULL)//for every letter
	{
		while(cur->next!=NULL)//search nodes of current level for word[i]
		{
			if(cur->c == word[i])//found letter
				break;
			cur=cur->next;
		}
		if(cur->next==NULL)//if not found
		{
			cur->c=word[i];
			TrieNodeInit(&(cur->next));
			TrieNodeInit(&(cur->child));
		}
		//printf("    loop %d , cur->c=%c\n",i,cur->c);
		if(word[i+1]=='\0')//finished
			break;
		cur=cur->child;
		i++;
	}
	
	///cur points to the leaf of word
	///edit the postings list
	if(cur->pos==NULL)///initialize pos list if needed
	{
		if((cur->pos=malloc(sizeof(struct PosList)))==NULL)
		{
			printf("TrieInsert: malloc failure, exiting...\n");
			exit(-1);
		}
		cur->pos->wordfreq=1;
		if((cur->pos->first=malloc(sizeof(struct PosNode)))==NULL)
		{
			printf("TrieInsert: malloc failure, exiting...\n");
			exit(-1);
		}
		cur->pos->first->lines=malloc(sizeof(int));
		cur->pos->first->lines[0]=line;
		cur->pos->first->lsz=1;
		cur->pos->first->pathname=malloc((strlen(filename)+1)*sizeof(char));
		strcpy(cur->pos->first->pathname,filename);
		cur->pos->first->next=NULL;
		//printf("docid=%d,pos->df=%d\n",docid,cur->pos->df);
	}
	else PosInsert(cur->pos,line,filename);
	
	return 0;
}

int TrieNodeInit(struct TrieNode** node)
{
	if(((*node)=malloc(sizeof(struct TrieNode)))==NULL)
	{
		printf("TrieNodeInit: malloc failure, exiting...\n");
		return -1;
	}
	(*node)->c     = 0;
	(*node)->child = NULL;
	(*node)->next  = NULL;
	(*node)->pos   = NULL;
	
	return 0;
}

int TrieInit(struct TrieRoot* trie)
{
	if((trie->first=malloc(sizeof(struct TrieNode)))==NULL)
	{
		printf("TrieInit: malloc failure, exiting...\n");
		return -1;
	}
	trie->first->c     = 0;
	trie->first->child = NULL;
	trie->first->next  = NULL;
	trie->first->pos   = NULL;
	
	return 0;
}

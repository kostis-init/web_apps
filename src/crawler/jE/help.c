#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "structs.h"
#include "defs.h"

int digits(int n)
{
	int counter=1;
	while(n/10)
	{
		n/=10;
		counter++;
	}
	return counter;
}

int fcount_lines(FILE* docfile)
{
	///returns docfile's number of lines
	char c;
	int count=0;
	while((c=fgetc(docfile))!=EOF)
		if(c=='\n')count++;
	return count;
}

int count_lines(char* buf)
{
	///returns buf's number of lines
	int count=0;
	for(int i=0;buf[i]!='\0';i++)
		if(buf[i]=='\n')count++;
	return count;
}

int exists(char* retbuf,char* name,int line)
{
	///returns true if line and name already exists in formatted retbuf
	int i=0,j;
	if(retbuf[i]=='\0')return 0;
	while(retbuf[i]!='\0')
	{
		char exs=1;//flag
		i+=5;//see formatted buffer in worker.c
		
		for (j = 0; retbuf[i]!=' ' ; i++,j++)
		{
			if(retbuf[i]!=name[j])
				exs=0;
		}
		i+=3;j=0;
		char buf[20];//buf for line number
		while(retbuf[i]!=' ')
		{
			buf[j]=retbuf[i];
			i++;j++;
		}
		buf[j]='\0';
		if(atoi(buf)!=line)exs=0;
		if(exs)return 1;
		while(retbuf[i]!='\n')i++;
		i++;
	}
	return 0;
}

long long timeInMilliseconds(void) //taken from stakcoverflow
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

int allset(char* flags,int numWor)
{
	for (int i = 0; i < numWor; i++)
	{
		if(flags[i]==0)return 0;
	}
	return 1;
	
}

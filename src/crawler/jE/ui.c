#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <limits.h>
#include "structs.h"
#include "defs.h"

extern int numWor;
//check for dead children
int* deadp;
int there_is_dead(int* deadp);

int user_input(int* fdw,int* fdr)
{
	while(1)
	{
		char allbuf[MAXBUFF];
		memset(allbuf,0,MAXBUFF);
		char buf[MAXBUFF];
		memset(buf,0,MAXBUFF);
		
		char buffer[128];
		char bufcp[128];
		char* token;
		
		do{
		printf("> ");
		fgets(buffer,sizeof(buffer),stdin);
		strcpy(bufcp,buffer);
		token=strtok(buffer," \t\n");
		}while(token==NULL);//while input is empty
		
		if(there_is_dead(deadp))replacew();//check for dead child
		
		if(!strcmp(token,"/search"))		///SEARCH CASE
		{
			char* query[MAXQ];
			int sz=0;	///sz holds the number of words
			while(token!=NULL && sz<MAXQ)
			{
				token=strtok(NULL," \t\n");
				if(token!=NULL)
				{
					query[sz]=malloc((strlen(token)+1) * sizeof(char));//allocate memory for every word of query
					strcpy(query[sz++],token);//keep every word
				}
			}
			if(sz<2 || strcmp(query[sz-2],"-d"))
			{
				for (int i = 0; i < sz; i++)
					free(query[i]);
				printf("Wrong Input, give deadline parameter (-d)\n");
				continue;
			}
			int deadline = atoi(query[sz-1]);
			if(deadline<=0 || deadline > 1000)
			{
				for (int i = 0; i < sz; i++)
					free(query[i]);
				printf("Please give a proper deadline (0 < deadline < 1000)\n");
				continue;
			}
			
			//write
			for (int i = 0; i < numWor; i++)
			{
				int n=strlen(bufcp);
				if(there_is_dead(deadp))replacew();//check for dead child
				if(write(fdw[i],bufcp,n)!=n)perror("write error");
			}
			
			struct pollfd* fds = malloc(numWor*sizeof(struct pollfd));
			for (int k = 0; k < numWor; k++)
			{
				fds[k].fd = fdr[k];
				fds[k].events = 0 | POLLIN;
			}
			char* flags=malloc(numWor*sizeof(char));
			memset(flags,0,numWor);
			
			int n;
			long long int rem_time=deadline*1000;
			long long int prev_time=timeInMilliseconds();
			while(!allset(flags,numWor) && rem_time > 0)
			{
				poll(fds,numWor,rem_time);
				rem_time -= timeInMilliseconds() - prev_time;
				prev_time = timeInMilliseconds();
				int ret=0;
				for (int k = 0; k < numWor; k++)
					if(fds[k].revents & POLLIN)
					{
						ret=1;flags[k]=1;
						char temp[MAXBUFF];
						memset(temp,0,MAXBUFF);
						n=read(fdr[k],temp,MAXBUFF);
						strcat(allbuf,temp);
					}
				if(!ret)break;
			}
			
			int ans=numWor;
			for (int k = 0; k < numWor; k++)if(!flags[k])//send END message to the still-working processes
			{ans--;if(write(fdw[k],"END",3)!=3)perror("write error");}
			
			printf("\nAnswered %d from %d workers\n",ans,numWor);
			printf("--------------------------------\nPATHNAME|NUMBER OF LINE|TEXT OF LINE\n%s\n",allbuf);
			
			free(fds);free(flags);
			for (int i = 0; i < sz; i++)
				free(query[i]);
		}
		else if(!strcmp(token,"/maxcount"))	///MAXCOUNT CASE
		{
			//write
			for (int i = 0; i < numWor; i++)
			{
				int n=strlen(bufcp);
				if(there_is_dead(deadp))replacew();//check for dead child
				if(write(fdw[i],bufcp,n)!=n)perror("write error");
			}
			
			//read
			for (int i = 0; i < numWor; i++)
			{
				int n=read(fdr[i],buf,MAXBUFF);
				if(n)buf[n]='\0';
				strcat(allbuf,buf);
			}
			//keep maximum
			memset(buf,0,MAXBUFF);
			int max=-1;
			for (int i = 0; allbuf[i]!='\0'; i++)
			{
				char num[15];
				int j=0;
				while(allbuf[i]!='|')num[j++]=allbuf[i++];
				num[j]='\0';
				int n = atoi(num);
				j=0;i++;
				while(allbuf[i]!='\n')num[j++]=allbuf[i++];
				num[j]='\0';
				
				if(n>max)
				{
					sprintf(buf,"%d|%s",n,num);
					max=n;
				}
				
			}
			if(max==0)printf("word not found\n");
			else printf("COUNT|PATHNAME\n%s\n",buf);
		}
		else if(!strcmp(token,"/mincount"))	///MINCOUNT CASE
		{
			//write
			for (int i = 0; i < numWor; i++)
			{
				int n=strlen(bufcp);
				if(there_is_dead(deadp))replacew();//check for dead child
				if(write(fdw[i],bufcp,n)!=n)perror("write error");
			}
			
			//read
			for (int i = 0; i < numWor; i++)
			{
				int n=read(fdr[i],buf,MAXBUFF);
				if(n)buf[n]='\0';
				strcat(allbuf,buf);
			}
			//keep minimum
			memset(buf,0,MAXBUFF);
			int min=INT_MAX;
			for (int i = 0; allbuf[i]!='\0'; i++)
			{
				char num[15];
				int j=0;
				while(allbuf[i]!='|')num[j++]=allbuf[i++];
				num[j]='\0';
				int n = atoi(num);
				j=0;i++;
				while(allbuf[i]!='\n')num[j++]=allbuf[i++];
				num[j]='\0';
				
				if(n<min && n!=0)
				{
					sprintf(buf,"%d|%s",n,num);
					min=n;
				}
				
			}
			if(min==INT_MAX)printf("word not found\n");
			else printf("COUNT|PATHNAME\n%s\n",buf);
		}
		else if(!strcmp(token,"/wc"))		///WC CASE
		{
			int nc=0,nw=0,nl=0;
			//write
			for (int i = 0; i < numWor; i++)
			{
				int n=strlen(bufcp);
				if(there_is_dead(deadp))replacew();//check for dead child
				if(write(fdw[i],bufcp,n)!=n)perror("write error");
			}
			
			
			//read
			for (int i = 0; i < numWor; i++)
			{
				int n=read(fdr[i],buf,MAXBUFF);
				buf[n]='\0';
				nc+=atoi(strtok(buf,"|"));
				nw+=atoi(strtok(NULL,"|"));
				nl+=atoi(strtok(NULL,"|"));
			}
			printf("%d %d %d\n",nc,nw,nl);
		}
		else if(!strcmp(token,"/exit"))		///EXIT CASE
		{
			//write
			for (int i = 0; i < numWor; i++)
			{
				int n=strlen(bufcp);
				if(there_is_dead(deadp))replacew();//check for dead child
				if(write(fdw[i],bufcp,n)!=n)perror("write error");
			}
			break;
		}
		else 								///DEFAULT CASE
			printf("command not found\n");
		
	}
	return 0;
}

int there_is_dead(int* deadp)
{
	for (int i = 0; i < numWor; i++)
		if(deadp[i]!=0)return 1;
	return 0;
	
}

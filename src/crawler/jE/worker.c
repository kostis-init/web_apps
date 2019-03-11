#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <poll.h>
#include <limits.h>
#include <time.h>
#include "structs.h"
#include "defs.h"

void worker(char* file1,char* file2)
{
	//open fifo
	int fdr=open(file1,O_RDONLY);if(fdr<0)perror("open read fifo error");
	int fdw=open(file2,O_WRONLY);if(fdw<0)perror("open write fifo error");
	
	//read the dirs
	char buf[MAXBUFF];int n;	//buf holds the directories (separated by '\n')
	if((n=read(fdr,buf,MAXBUFF))<=0)perror("read error");
	buf[n]='\0';
	
	//save files in memory
	struct filelist* fl = malloc(sizeof(struct filelist));
	fl->next=NULL;
	struct filelist* cur=fl;
	for(char* dir=strtok(buf,"\n"); dir!=NULL; dir=strtok(NULL, "\n"))//for every dir
	{
		DIR* d;
		struct dirent* sdir;
		d = opendir(dir);
		if (d)
		{
			while ((sdir = readdir(d)) != NULL)//for every file in dir
				if(strcmp(sdir->d_name,".") && strcmp(sdir->d_name,".."))
				{
					//add in list
					//pathname
					cur->pathname=malloc((strlen(dir)+strlen(sdir->d_name)+2)*sizeof(char));
					cur->pathname[0]='\0';
					strcat(cur->pathname,dir);strcat(cur->pathname,"/");strcat(cur->pathname,sdir->d_name);
					//map
					FILE* f;
					if((f = fopen(cur->pathname,"r")) == NULL)perror("open file error");
					int lines=fcount_lines(f);
					cur->nlines=lines;
					cur->map=malloc((lines+1)*sizeof(char*));
					parse_file(f,cur->map);
					fclose(f);
					//printf("++++\n%d:lines=%d:name=%s:text:\n",getpid(),lines,cur->pathname);
					
					cur->next=malloc(sizeof(struct filelist));
					cur->next->next=NULL;
					cur=cur->next;
				}
			closedir(d);
		}
		else perror("open dir error");
	}
	
	//construct trie
	struct TrieRoot trie;	///trie:	root of the trie
	create_index(fl,&trie);
	
	//give response OK
	if(write(fdw,"OK",2)!=2)perror("write error");
	
	
	//create files for log
	char* lb = malloc(sizeof(char)*(15+digits(getpid())));
	lb[0]='\0';
	sprintf(lb,"log/Worker_%d",(int)getpid());
	int logfd = open(lb,O_CREAT|O_RDWR|O_APPEND,PERMS);
	free(lb);
	if(logfd<0)perror("logfd error");
	char filebuf[MAXBUFF];memset(filebuf,0,MAXBUFF);
	char s[64];//time
	int keys=0;//total keywords found
	//user input
	while(1)
	{
		if((n=read(fdr,buf,MAXBUFF))<=0)perror("read error");
		buf[n]='\0';
		
		char* token=strtok(buf," \t\n");
		if(!strcmp(token,"/search"))		///SEARCH CASE
		{
			char* retbuf=malloc(sizeof(char));retbuf[0]='\0';
			char* tempbuf=malloc(sizeof(char));tempbuf[0]='\0';
			
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
			
			for(int t=0;t<sz-2;t++)//for every word
			{
				token=query[t];
				//log
				time_t t = time(NULL);
				struct tm *tm = localtime(&t);
				strftime(s, sizeof(s), "%c", tm);
				sprintf(filebuf,"%s | search | %s |",s,token);
				
				struct TrieNode* node=trie.first;
				struct PosList* pos=node->pos;
				int i=0;
				while(token[i]!='\0')///search trie character-character
				{
					while(node!=NULL)
					{
						if(token[i] == node->c)
							break;
						node=node->next;
					}
					if(node==NULL)///if not found
						break;
					
					i++;
					pos=node->pos;
					node=node->child;
				}
				
				if(node==NULL || pos ==NULL)///if not found
				{
					//log
					strcat(filebuf,"\n");
					write(logfd,filebuf,strlen(filebuf));
					continue;
				}
				struct PosNode* cur = pos->first;
				int first=1;
				while(cur!=NULL)///parse pos
				{
					//keywords
					if(first)keys++;
					first=0;
					for (int j = 0; j < cur->lsz; j++)
						if(!exists(retbuf,cur->pathname,cur->lines[j]))//if name and line not already in the return buffer
						{
							char* text;
							struct filelist* now=fl;
							while(now->next!=NULL)
							{
								if(!strcmp(now->pathname,cur->pathname))
								{
									text=now->map[cur->lines[j]];
								}
								now=now->next;
							}
							int size=strlen(cur->pathname)+digits(cur->lines[j])+strlen(text)+20;
							int len=strlen(retbuf) +size;
							retbuf= realloc(retbuf,sizeof(char)*(len+1));
							
							free(tempbuf);
							tempbuf=malloc((size+1)*sizeof(char));
							snprintf(tempbuf,size+1, "---> %s | %d | %s\n",cur->pathname,cur->lines[j],text);
							strcat(retbuf,tempbuf);
							
						}
					//log
					free(tempbuf);
					tempbuf=malloc((4+strlen(cur->pathname))*sizeof(char));
					sprintf(tempbuf," %s |",cur->pathname);
					strcat(filebuf,tempbuf);
					
					cur=cur->next;
				}
				//log
				strcat(filebuf,"\n");
				write(logfd,filebuf,strlen(filebuf));
			}
			for (int i = 0; i < sz; i++)
				free(query[i]);
			
			struct pollfd fds[1];
			fds[0].fd=fdr;
			fds[0].events=0|POLLIN;
			
			if(poll(fds,1,0))read(fdr,retbuf,3);//if timeout has already come, read it and do nothing
			else	write(fdw,retbuf,strlen(retbuf)+1);//else write normally
			
			free(tempbuf);free(retbuf);
		}
		else if(!strcmp(token,"/maxcount"))		///MAXCOUNT CASE
		{
			token=strtok(NULL," \t\n");
			if(token!=NULL)
			{
				//log
				time_t t = time(NULL);
				struct tm *tm = localtime(&t);
				strftime(s, sizeof(s), "%c", tm);
				sprintf(filebuf,"%s | maxcount | %s |",s,token);
				
				struct TrieNode* node=trie.first;
				struct PosList* pos=node->pos;
				int i=0;
				while(token[i]!='\0')///search trie character-character
				{
					while(node!=NULL)
					{
						if(token[i] == node->c)//found
							break;
						node=node->next;
					}
					if(node==NULL)///if not found
						break;
						
					i++;
					pos=node->pos;
					node=node->child;
				}
				
				if(node==NULL || pos ==NULL)///if not found
					write(fdw,"0|\n",3);
				else
				{
					char retbuf[MAXBUFF];retbuf[0]='\0';
					memset(retbuf,0,MAXBUFF);
					struct PosNode* cur = pos->first;
					int max=-1;
					while(cur!=NULL)///parse pos
					{
						if(cur->lsz > max)
						{
							//printf("writing %d|%s\n",cur->lsz,cur->pathname);
							sprintf(retbuf,"%d|%s\n",cur->lsz,cur->pathname);
							max=cur->lsz;
						}
						cur=cur->next;
					}
					write(fdw,retbuf,MAXBUFF);
				}
				strcat(filebuf,"\n");
				write(logfd,filebuf,strlen(filebuf));
			}
			else
			write(fdw,"0|\n",3);
			
		}
		else if(!strcmp(token,"/mincount"))	///MINCOUNT CASE
		{
			token=strtok(NULL," \t\n");
			if(token!=NULL)
			{
				//log
				time_t t = time(NULL);
				struct tm *tm = localtime(&t);
				strftime(s, sizeof(s), "%c", tm);
				sprintf(filebuf,"%s | mincount | %s |",s,token);
				
				struct TrieNode* node=trie.first;
				struct PosList* pos=node->pos;
				int i=0;
				while(token[i]!='\0')///search trie character-character
				{
					while(node!=NULL)
					{
						if(token[i] == node->c)//found
							break;
						node=node->next;
					}
					if(node==NULL)///if not found
						break;
						
					i++;
					pos=node->pos;
					node=node->child;
				}
				
				if(node==NULL || pos ==NULL)///if not found
					write(fdw,"0|\n",3);
				else
				{
					char retbuf[MAXBUFF];retbuf[0]='\0';
					memset(retbuf,0,MAXBUFF);
					struct PosNode* cur = pos->first;
					int min=INT_MAX;
					while(cur!=NULL)///parse pos
					{
						if(cur->lsz < min)
						{
							//printf("writing %d|%s\n",cur->lsz,cur->pathname);
							sprintf(retbuf,"%d|%s\n",cur->lsz,cur->pathname);
							min=cur->lsz;
						}
						cur=cur->next;
					}
					write(fdw,retbuf,MAXBUFF);
				}
				strcat(filebuf,"\n");
				write(logfd,filebuf,strlen(filebuf));
			}
			else
			write(fdw,"0|\n",3);
		}
		else if(!strcmp(token,"/wc"))		///WC CASE
		{
			char retbuf[MAXBUFF];retbuf[0]='\0';
			memset(retbuf,0,MAXBUFF);
			
			//log
			time_t t = time(NULL);
			struct tm *tm = localtime(&t);
			strftime(s, sizeof(s), "%c", tm);
			sprintf(filebuf,"%s | wc |\n",s);
			write(logfd,filebuf,strlen(filebuf));
			
			int nc=0,nw=0,nl=0;
			//parse filelist, count
			struct filelist* cur= fl;
			while(cur->next!=NULL)//for every file
			{
				nl+=cur->nlines;//nl
				int i=0,j=0;
				for (i = 0; i < cur->nlines; i++)//for every line
					for (j = 0; cur->map[i][j]!='\0' ; j++)
					{
						nc++;//nc
						if(cur->map[i][j]!=' ' && cur->map[i][j]!='\t')
						{
							nw++;
							while(cur->map[i][j] != ' ' && cur->map[i][j]!='\t' && cur->map[i][j]!='\0')
							{j++;nc++;}
							if(cur->map[i][j]=='\0')break;
						}
					}
				cur=cur->next;
			}
			sprintf(retbuf,"%d|%d|%d\n",nc,nw,nl);
			write(fdw,retbuf,MAXBUFF);
			
		}
		else if(!strcmp(token,"/exit"))
			break;
	}
	
	//give some statistics
	char temp[MAXBUFF];
	sprintf(temp,"Total keywords found: %d",keys);
	if(write(fdw,temp,strlen(temp))<0)perror("write error");
	
	//free filelist
	clear_fl(fl);
	//free trie
	TrieDelete(trie);
	
	close(fdw);
	close(fdr);
	close(logfd);
}


void clear_fl(struct filelist* fl)
{
	
	if(fl->next!=NULL)
	{
		free(fl->pathname);
		for (int i = 0; i < fl->nlines; i++)
			free(fl->map[i]);
		free(fl->map);
		clear_fl(fl->next);
	}
	
	free(fl);
	
}


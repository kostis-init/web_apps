#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "structs.h"
#include "defs.h"

int numWor;
char** pipes;
char** dirs;
int* fdw;
int* fdr;
int* pids;
extern int* deadp;

void handle_child(int signum);
void pass_dirs(char**,int*,int*,int);

void start(char** ds,int w)
{
	dirs=ds;
	numWor=w;
	//for dead children handling
	deadp=malloc(numWor*sizeof(int));
	for (int i = 0; i < numWor; i++)deadp[i]=0;
	signal(SIGCHLD,handle_child);//handle dead child
	
	printf("Starting workers, please wait...\n");
	//create array of pids
	pids = malloc(numWor*sizeof(int));
	//create arrays of file descriptors
	fdw = malloc(numWor*sizeof(int));
	fdr = malloc(numWor*sizeof(int));
	//create array of pipefiles
	pipes=malloc(2*numWor*sizeof(char*));
	for (int i = 0; i < numWor; i++)
	{
		pipes[2*i]	 = malloc((FLEN + digits(i) + 1)*sizeof(char));
		pipes[2*i+1] = malloc((FLEN + digits(i) + 1)*sizeof(char));
		snprintf(pipes[2*i]  ,FLEN+digits(i)+1,"%s%d",FIFO1,i);
		snprintf(pipes[2*i+1],FLEN+digits(i)+1,"%s%d",FIFO2,i);
	}
	
	for (int i = 0; i < numWor; i++)//start workers
	{
		//create fifo
		if(mkfifo(pipes[2*i]  ,PERMS)<0 && errno!=EEXIST)perror("mkfifo error");
		if(mkfifo(pipes[2*i+1],PERMS)<0 && errno!=EEXIST){unlink(pipes[2*i]);perror("mkfifo error");}
		
		int pid=fork();
		if(pid<0)perror("fork failed");
		else if(pid==0) //child/worker
		{
			//clean
			int j=0;
			while(dirs[j])free(dirs[j++]);
			free(dirs);free(fdw);free(fdr);
			free(pids);free(deadp);
			
			worker(pipes[2*i],pipes[2*i+1]);
			//clean
			for (int j = 0; j < 2*numWor; j++)free(pipes[j]);free(pipes);
			exit(0);
		}
		pids[i]=pid;
		//open fifo
		if((fdw[i]=open(pipes[2*i],O_WRONLY))<0)perror("open write fifo error");
		if((fdr[i]=open(pipes[2*i+1],O_RDONLY))<0)perror("open read fifo error");
	}
	//pass the directories to workers
	pass_dirs(dirs,fdw,fdr,numWor);
	printf("\nWorkers are ready\n");
	
	user_input(fdw,fdr);//user
	
	//read some statistics
	char temp[MAXBUFF];int n;
	for (int i = 0; i < numWor; i++)
	{
		if((n=read(fdr[i],temp,MAXBUFF))<0)perror("read error");
		temp[n]='\0';
		printf("Worker_%d: %s\n",pids[i],temp);
	}
	
	//clean
	int a;	wait(&a);
	for (int i = 0; i < 2*numWor; i++)unlink(pipes[i]);
	for (int i = 0; i < 2*numWor; i++)free(pipes[i]);
	free(pipes);
	for (int i = 0; i < numWor; i++)
	{close(fdw[i]);close(fdr[i]);}
	free(fdw);free(fdr);
	int j=0;
	while(dirs[j])free(dirs[j++]);
	free(dirs);free(pids);
	free(deadp);
	//DONT FORGET WAIT,UNLINK,CLEAN
	
}

void pass_dirs(char** dirs,int* fdw,int* fdr,int numWor)
{
	char** bufs=malloc(numWor*sizeof(char*));
	for (int i = 0; i < numWor; i++)
	{
		if((bufs[i]=malloc(MAXBUFF*sizeof(char)))==NULL)perror("malloc error");
		bufs[i][0]='\0';
	}
	
	//divide directories
	int i=0;
	while (dirs[i]!=NULL)
	{
		strcat(bufs[i%numWor],dirs[i]);
		strcat(bufs[i%numWor],"\n");
		i++;
	}
	
	for (int i = 0; i < numWor; i++)
	{
		//give the bufs
		int n=strlen(bufs[i]);
		if(write(fdw[i],bufs[i],n)!=n)perror("write error");
	}
	
	for (int i = 0; i < numWor; i++)
	{
		//wait for response
		char temp[MAXBUFF];int n;
		if((n=read(fdr[i],temp,MAXBUFF))<0)perror("read error");
	}
	
	for (int i = 0; i < numWor; i++)
		free(bufs[i]);
	free(bufs);
}

void replacew()
{
	for(int r = 0; r < numWor; r++)
		if(deadp[r]!=0)
		{
			for (int i = 0; i < numWor; i++)
			{
				if (pids[i]==deadp[r])//find the one that exited to make it
				{
					unlink(pipes[2*i]);
					unlink(pipes[2*i+1]);
					close(fdw[i]);close(fdr[i]);
					//create fifo
					if(mkfifo(pipes[2*i]  ,PERMS)<0 && errno!=EEXIST)perror("mkfifo error");
					if(mkfifo(pipes[2*i+1],PERMS)<0 && errno!=EEXIST){unlink(pipes[2*i]);perror("mkfifo error");}
					
					int pid=fork();
					if(pid<0)perror("fork failed");
					else if(pid==0) //child/worker
					{
						//clean
						int j=0;
						while(dirs[j])free(dirs[j++]);
						free(dirs);free(fdw);free(fdr);
						free(pids);free(deadp);
						
						worker(pipes[2*i],pipes[2*i+1]);
						//clean
						for (int j = 0; j < 2*numWor; j++)free(pipes[j]);free(pipes);
						exit(0);
					}
					pids[i]=pid;
					//open fifo
					if((fdw[i]=open(pipes[2*i],O_WRONLY))<0)perror("open write fifo error");
					if((fdr[i]=open(pipes[2*i+1],O_RDONLY))<0)perror("open read fifo error");
					
					//pass the dirs
					char buf[MAXBUFF];
					memset(buf,0,MAXBUFF);
					int j=0;
					while (dirs[j]!=NULL)
					{
						if(j==i)
						{
							strcat(buf,dirs[j]);
							strcat(buf,"\n");
						}
						j++;
					}
					//give the dirs
					int n=strlen(buf);
					if(write(fdw[i],buf,n)!=n)perror("write error");
					//wait for response
					if((n=read(fdr[i],buf,MAXBUFF))<0)perror("read error");
					
					
				}
			}
			printf("\n");
			deadp[r]=0;//not dead anymore
		}
}

void handle_child(int signum)
{
	signal(SIGCHLD,handle_child);
	
	int status;
	pid_t pid;
	
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		if ( WIFSIGNALED(status) )
			for (int i = 0; i < numWor; i++)//find an empty spot for the dead
				if(deadp[i]==0)
				{
					deadp[i] = (int)pid;
					break;
				}
}

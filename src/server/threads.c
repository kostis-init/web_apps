#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#include "defs.h"

pthread_t *ths;// Array of threads.
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pool_t pool;

int quit = 0;

extern unsigned int npages;
extern long unsigned int nbytes;


void* start(void*);// Thread's start function.

void threads(void)
{
	
	ths = malloc(nthreads * sizeof(pthread_t));
	
	// Initialize.
	initialize_pool();
	pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);
	
	// Start threads.
	for (int i = 0; i < nthreads; i++)
		pthread_create(&ths[i],0,start,0);
	
	
}

void* start(void * ptr)
{
	
	while(1)
	{
		int socket;
		pthread_mutex_lock(&mtx);// LOCK
		
		while (pool.count <= 0)
		{
			//printf("%d just startt wait\n",(int)pthread_self());
			pthread_cond_wait(&cond_nonempty, &mtx);
			//printf("%d just end wait, quit=%d\n",(int)pthread_self(),quit);
			if(quit){pthread_mutex_unlock(&mtx);return NULL;}
		}
		if(quit){pthread_mutex_unlock(&mtx);return NULL;}
		
		//printf("%d take socket\n",(int)pthread_self());
		
		socket = pool.data[pool.start];
		pool.start = (pool.start + 1) % POOL_SIZE;
		pool.count--;
		
		pthread_mutex_unlock(&mtx);//UNLOCK
		
		pthread_cond_broadcast(&cond_nonfull);
		//printf("%d unlocked, broadcast\n",(int)pthread_self());
		
		
		char buf[128];memset(buf,'\0',128);
		
		//only GET
		if((read(socket, buf ,3))<=0){close(socket);continue;}
		if(strcmp(buf,"GET")){close(socket);continue;}
		
		// message holds the whole reading
		char* message=malloc(4*sizeof(char));memset(message,'\0',4);
		strcpy(message,"GET");
		int olds=4;int n;
		
		while( (n = read(socket,buf,127)) > 0) // Save it in message
		{
			message = realloc(message,olds + (n+1)*sizeof(char));
			strcat(message,buf);
			olds += (n+1)*sizeof(char);
			
			//check for end (LFLF)
			int last4 = -4;
			for(int i=0;buf[i]!='\0';i++)
				last4 = i - 4;
			if(last4>=-1)if(buf[last4+1]=='\r' && buf[last4+2]=='\n' && buf[last4+3]=='\r' && buf[last4+4]=='\n')
				break;
			
			memset(buf,'\0',128);
		}
		if(n<0)perror_exit("read");
		memset(buf,'\0',128);
		
		//printf("%d  %s\n",(int)pthread_self(),message);
		
		
		char* token;
		token = strtok(message," ");//GET
		if(!strcmp(token,"GET"))
		{
			token = strtok(NULL," ");
			strcpy(buf,dir);
			//if last letter is '/', remove it
			int j=0,last=0;
			while(buf[j]!='\0')
				last=j++;
			if(buf[last]=='/')
				buf[last]='\0';
			
			strcat(buf,token);//buf holds the file(absolute path)
			//printf("%d  %s\n",(int)pthread_self(),buf);
			
			//exists, perms
			int exists=0;int perm=0;
			if(file_exists(buf))exists=1;
			if(  file_perm(buf))  perm=1;
			
			if(exists && perm)
			{
				long unsigned int total = response_ok(socket,buf);
				pthread_mutex_lock(&mtx);
				npages++;
				nbytes+=total;
				pthread_mutex_unlock(&mtx);
			}
			else if(!exists)
				response_ne(socket);
			else if(!perm)
				response_np(socket);
			
		}
		//printf("%d ends!\n",(int)pthread_self());
		free(message);
		close(socket);
		
	}
	return NULL;
}

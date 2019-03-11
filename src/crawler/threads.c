#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"

pthread_t *ths;// Array of threads.
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pool_t pool;

int in_progress=1;

extern unsigned int npages;
extern long unsigned int nbytes;

int quit=0;

void* start(void*);

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
	{
		pthread_create(&ths[i],0,start,0);
	}
	
	
	//put first url
	pthread_mutex_lock(&mtx);
	
	pool.end = (pool.end + 1) % POOL_SIZE;
	pool.data[pool.end] = malloc((strlen(s_url)+1)*sizeof(char));
	strcpy(pool.data[pool.end],s_url);
	pool.count++;
	
	pthread_mutex_unlock(&mtx);//UNLOCK
	
	pthread_cond_broadcast(&cond_nonempty);
}
int ts=0;
void* start(void * ptr)
{
	int counter=0;
	while(1)
	{
		if(counter==20)break;
		
		char* url=NULL;
		pthread_mutex_lock(&mtx);// LOCK
		
		while (pool.count <= 0)
		{
			pthread_cond_wait(&cond_nonempty, &mtx);
		}
		if((url = malloc((strlen(pool.data[pool.start])+1) * sizeof(char)))==NULL)perror_exit("malloc");
		
		strcpy(url,pool.data[pool.start]);
		free(pool.data[pool.start]);
		pool.start = (pool.start + 1) % POOL_SIZE;
		pool.count--;
		
		pthread_mutex_unlock(&mtx);//UNLOCK
		
		pthread_cond_broadcast(&cond_nonfull);
		
		//printf("url=%s\n",url);
		
		int sfd;
		struct sockaddr_in server;
		struct hostent *hent;
		if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)perror_exit("socket");
		if((hent = gethostbyname(host))==NULL){printf("gethostbyname error\n");exit(1);}
		
		server.sin_family = AF_INET;
		memcpy(&server.sin_addr, hent->h_addr, hent->h_length);
		server.sin_port = htons(port);
		
		if (connect(sfd, (struct sockaddr*)&server, sizeof(server)) < 0)perror_exit("connect");
		
		char getmsg[1024];memset(getmsg,'\0',1024);
		strcat(getmsg,"GET ");
		strcat(getmsg,url);
		strcat(getmsg," HTTP/1.1\r\nUser-Agent: mycrawler\r\nHost: mycrawler\r\nAccept-Language: en-us\r\nAccept-Encoding: gzip, deflate\r\nConnection: Closed\r\n\r\n");
		
		//send get request
		if(write(sfd,getmsg,strlen(getmsg)+1)<0)perror_exit("write");
		
		// Save to file
		int fdw;char path[128];memset(path,'\0',128);
		strcat(path,save_dir);
		strcat(path,url);
		
		//create directory if it doesn't exist
		char dir[50];memset(dir,'\0',50);
		int c=0;int i=0;
		while(c<2)//two '/'
		{
			if(path[i]=='/')c++;
			if(c==2)break;
			dir[i]=path[i];
			i++;
		}
		struct stat st = {0};
		if (stat(dir, &st) == -1)
			mkdir(dir, 0744);
		
		if((fdw = open(path, O_RDWR | O_APPEND | O_CREAT | O_TRUNC, 0644))<0)perror_exit("open");
		
		//read header
		char buf[1];memset(buf,'\0',1);
		
		while(1)
		{
			if(read(sfd,buf,1)<0)perror_exit("read");
			if(buf[0]=='\r')
			{
				if(read(sfd,buf,1)<0)perror_exit("read");
				if(buf[0]=='\n')
				{
					if(read(sfd,buf,1)<0)perror_exit("read");
					if(buf[0]=='\r')
					{
						if(read(sfd,buf,1)<0)perror_exit("read");
						if(buf[0]=='\n')break;
					}
				}
			}
		}
		
		
		//save text to fdw
		int total=0;
		char response[1024];memset(response,'\0',1024);
		while(1)
		{
			int n;
			if((n=read(sfd,response,1023))<0)perror_exit("read");
			total+=n;
			if(n==0)break;
			if(write(fdw,response,1024)<0)perror_exit("write");
			memset(response,'\0',1024);
		}
		
		pthread_mutex_lock(&mtx);
		npages++;
		nbytes+=total;
		
		pthread_mutex_unlock(&mtx);
		
		
		// Analyze file to place urls
		lseek(fdw,0,SEEK_SET);
		
		response[1]='\0';
		char* text = malloc(1000000);
		if(text==NULL)perror_exit("malloc");
		memset(text,'\0',1000000);
		memset(response,'\0',1024);
		while(1)
		{
			int n;
			if((n=read(fdw,response,50))<0)perror_exit("read");
			if(n==0)break;
			strcat(text,response);
			memset(response,'\0',51);
		}
		
		char* ptr=text;
		while(1)
		{
			ptr=strstr(ptr,"<a href=\"/");
			if(ptr==NULL)break;
			ptr+=9;
			
			int last2=0;int last1=0;
			int cur=0;
			while(ptr[cur]!='"')
			{
				if(ptr[cur]=='/')
				{
					last2=last1;
					last1=cur;
				}
				cur++;
			}
			//create new path, add it to queue
			char newpath[50];memset(newpath,'\0',50);
			cur=last2;int i=0;
			while(ptr[cur]!='"')
				newpath[i++]=ptr[cur++];
			if(newpath[i-1]!='l' || newpath[i-2]!='m'|| newpath[i-3]!='t' || newpath[i-4]!='h' || newpath[i-5]!='.' ){continue;}
			if(newpath[0]!='/' || newpath[1]!='s' || newpath[2]!='i' || newpath[3]!='t' || newpath[4]!='e'){continue;} 
			
			pthread_mutex_lock(&mtx);
			//printf("adding!!!! %s\n",newpath);
			pool.end = (pool.end + 1) % POOL_SIZE;
			pool.data[pool.end] = malloc((strlen(newpath)+1)*sizeof(char));
			strcpy(pool.data[pool.end],newpath);
			pool.count++;
			
			pthread_mutex_unlock(&mtx);//UNLOCK
			
			pthread_cond_broadcast(&cond_nonempty);
			
			
		}
		close(sfd);
		close(fdw);
		counter++;
		free(text);
		free(url);
	}
	ts++;
	if(ts==nthreads)in_progress=0;
	
	return NULL;
}

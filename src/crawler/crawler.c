#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>
#include <dirent.h>

#include "defs.h"

unsigned int npages=0;
long unsigned int nbytes=0;

extern int in_progress;

void crawler(void)
{
	struct timeval  start, end;
	gettimeofday(&start, NULL);
	
	// If client closes connection, do not exit.
	signal(SIGPIPE, SIG_IGN);
	
	// Create socket.
	int c_sockfd;
	if ((c_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) perror_exit("socket");
	
	// Set option. Re-bind to port, even if a previous connection is still in TIME_WAIT state.
	int reuse_addr = 1;
	setsockopt(c_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
	
	// Bind sockets.
	struct sockaddr_in c_socket;
	c_socket.sin_family      = AF_INET;
	c_socket.sin_addr.s_addr = htonl(INADDR_ANY);
	c_socket.sin_port        = htons(c_port);
	if (bind(c_sockfd, (struct sockaddr*) &c_socket, sizeof(c_socket)) < 0) perror_exit("bind");
	
	// Listen for connections.
	if (listen(c_sockfd, 5) < 0) perror_exit("listen");
	printf("Listening for connections to command_port: %d\n", c_port);
	
	// Accept connections.
	struct pollfd* fds = malloc(sizeof(struct pollfd));
	fds[0].fd = c_sockfd; fds[0].events = 0 | POLLIN; fds[0].revents = 0;
	while (1)
	{
		// Block, until someone connects.
		if(poll(fds,1,-1) < 0) perror_exit("poll");
		
		//printf("just unblecked by poll\n");
		// Check if there is dead thread.
		
		
		//COMMAND PORT ALERT
		if(fds[0].revents & POLLIN)
		{
			int client_sockfd;
			struct sockaddr_in client;
			socklen_t clientlen = sizeof(client); 
			if ((client_sockfd = accept(c_sockfd, (struct sockaddr *)&client, &clientlen)) < 0) perror_exit("accept");
			
			char buf[128];memset(buf,'\0',128);
			if(read(client_sockfd,buf,128) < 0)perror_exit("read");
			
			// Clean input
			int i=0;
			while(1)
				if((buf[i]<'a' || buf[i]>'z') && (buf[i]<'A' || buf[i]>'Z'))
					{buf[i]='\0';break;}
				else i++;
			
			if(!strcmp(buf,"STATS"))			// STATS
			{
				gettimeofday(&end, NULL);
				pthread_mutex_lock(&mtx);// LOCK
				char buf[1024];memset(buf,'\0',1024);
				sprintf(buf,"Crawler running for %.0f seconds, downloaded %d pages, %ld bytes\n", (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec),npages,nbytes);
				write(client_sockfd,buf,strlen(buf)+1);
				pthread_mutex_unlock(&mtx);
		
			}
			else if(!strcmp(buf,"SHUTDOWN"))	// SHUTDOWN
			{
				close(client_sockfd);
				break;
			}
			else if(!strcmp(buf,"SEARCH"))
			{
				if(in_progress)
					write(client_sockfd,"Work in progress, please wait.\n",31);
				else
				{
					write(client_sockfd,"Search is not yet implemented!\n",31); 
				}
			}
			else								// DEFAULT
			{
				write(client_sockfd,"give a proper command\n",22);
			}
			
			close(client_sockfd);
		}
		fds[0].revents = 0;
	}
	
	pthread_mutex_lock(&mtx);//LOCK
	quit = 1;
	pthread_mutex_unlock(&mtx);//UNLOCK
	pthread_cond_broadcast(&cond_nonempty);// Wake threads, to kill them
	
	sleep(1);
	for (int i = 0; i < nthreads; i++)
		pthread_kill(ths[i],SIGINT);
	
	pthread_cond_destroy(&cond_nonempty);
	pthread_cond_destroy(&cond_nonfull);
	pthread_mutex_destroy(&mtx);
	
	free(ths);
	
	close(c_sockfd);
	free(fds);
	
}

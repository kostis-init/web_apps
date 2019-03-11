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

//returns number of bytes sent
long unsigned int response_ok(int socket,char *file)
{
	char buf[1024];
	int fd;
	if((fd = open(file,O_RDONLY))<0)perror_exit("open");
	//content-length
	lseek(fd, 0, SEEK_END);
	int len = (int)lseek(fd, 0, SEEK_CUR);
	lseek(fd, 0, SEEK_SET);//rewind
	
	//date
	char date[256];memset(date,'\0',256);
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	
	
	sprintf(buf, "HTTP/1.1 200 OK\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Date: %s\r\n",date);
	write(socket, buf, strlen(buf));
	sprintf(buf, "Server: myhttpd/1.0.0 (Ubuntu64)\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Content-Length: %d\r\n",len);
	write(socket, buf, strlen(buf));
	sprintf(buf, "Content-Type: text/html\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Connection: Closed\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "\r\n");
	write(socket, buf, strlen(buf));
	memset(buf,'\0',1024);
	long unsigned int total=0;int n;
	while((n=read(fd,buf,1023))>0)
	{
		total+=n;
		//printf("%s",buf);
		write(socket,buf,strlen(buf));
		memset(buf,'\0',1024);
	}
	return total;
}

void response_ne(int socket)
{
	char buf[1024];
	//date
	char date[256];memset(date,'\0',256);
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	

	sprintf(buf, "HTTP/1.1 404 Not Found\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Date: %s\r\n",date);
	write(socket, buf, strlen(buf));
	sprintf(buf, "Server: myhttpd/1.0.0 (Ubuntu64)\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Content-Length: 200\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Content-Type: text/html\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Connection: Closed\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "<html><head><title>error</title></head>\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "<body><p>Sorry dude, couldn't find this file.</body></html>\r\n");
	write(socket, buf, strlen(buf));
}

void response_np(int socket)
{
 char buf[1024];
 //date
	char date[256];
	time_t now = time(0);
	struct tm tm = *gmtime(&now);
	strftime(date, sizeof date, "%a, %d %b %Y %H:%M:%S %Z", &tm);
	

	sprintf(buf, "HTTP/1.1 403 Forbidden\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Date: %s\r\n",date);
	write(socket, buf, strlen(buf));
	sprintf(buf, "Server: myhttpd/1.0.0 (Ubuntu64)\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Content-Length: 100\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Content-Type: text/html\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "Connection: Closed\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "<html><head><title>error</title></head>\r\n");
	write(socket, buf, strlen(buf));
	sprintf(buf, "<body><p>Trying to access this file but don’t think I can make it.</body></html>\r\n");
	write(socket, buf, strlen(buf));
}

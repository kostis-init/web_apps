#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "defs.h"

///initializes pool
void initialize_pool(void)
{
	pool.start = 0;
	pool.end = -1;
	pool.count = 0;
}

///checks if filename exists
int file_exists(char* filename)
{
	struct stat buffer;
	return (stat (filename, &buffer) == 0);
}

///checks if filename permissions are ok
int file_perm(char* filename)
{
	struct stat buffer;
	if (stat (filename, &buffer) == -1)return 0;
	return (buffer.st_mode & S_IRUSR);
}

///checks if str represents a directory 
int is_dir(char *str)
{
	struct stat statbuf;
	if ( stat(str, &statbuf) != 0 ) return 0;
	return S_ISDIR(statbuf.st_mode);
}

///returns how many digits n has
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

///returns buf's number of lines
int count_lines(char* buf)
{
	int count=0;
	for(int i=0;buf[i]!='\0';i++)
		if(buf[i]=='\n')count++;
	return count;
}

void perror_exit(char* message)
{
	perror(message);
	exit(1);
}

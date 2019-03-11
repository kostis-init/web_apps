#include <stdio.h>
#include <stdlib.h>

#include "defs.h"

int s_port = 0;
int c_port = 0;
int nthreads;
char* dir = NULL;

int main(int argc,char* argv[])
{
	///PARSE ARGUMENTS
	parse_args(argc,argv);
	
	///SET UP THREADS
	threads();
	
	///START SERVER
	server();
	
	
	
	exit(0);
}

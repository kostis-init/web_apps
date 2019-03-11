#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "defs.h"

char* host;
int port;
int c_port;
int nthreads;
char* save_dir;
char* s_url;

int main(int argc,char* argv[])
{
	///PARSE ARGUMENTS
	parse_args(argc,argv);
	
	/// start threads
	threads();
	
	/// crawler
	crawler();
	
	exit(0);
}

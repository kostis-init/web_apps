#include <stdio.h>
#include <stdlib.h>
#include "structs.h"
#include "defs.h"

int main(int argc,char* argv[])
{
	
	/**PARSE ARGUMENTS*/
	char** dirs;		///dirs:		array of directories
	int  numWor;		///numWor:		number of workers
	parse_args(argc,argv,&dirs,&numWor);
	
	printf("Welcome to jobExecutor!\n[NOTE: Maximum number of keywords for search query is set to %d]\n\n",MAXQ-2);
	
	/**START EXECUTOR*/
	start(dirs,numWor);
	
	printf("Exiting...\n");
	
	return 0;
}

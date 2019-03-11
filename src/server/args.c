#include <stdio.h>
#include <stdlib.h>

#include "defs.h"

///parse command-line
///arguments in memory
void parse_args(int argc,char** argv)
{
	//error_message
	char* message = "Usage:\n$ ./myhttpd -p serving_port -c command_port -t num_of_threads -d root_dir";
	
	if( argc != 9) { fprintf(stderr,"ERROR (wrong usage)\n%s\n",message); exit(1); }
	
	//flags, to check that args are unique
	bool p,c,t,d; p = c = t = d = false;
	
	for (int i = 1; i < argc; i+=2)//scan flags
	{
		if(argv[i][0] != '-') { fprintf(stderr,"ERROR (wrong usage)\n%s\n",message); exit(1); }
		switch (argv[i][1])
		{
			case 'p':
				if(p == true) { fprintf(stderr,"ERROR (flag given twice: -p)\n%s\n",message); exit(1); }
				s_port = atoi(argv[i+1]);
				if( s_port <= 0 ) { fprintf(stderr,"ERROR (bad serving_port)\n"); exit(1); }
				p = true;break;
			case 'c':
				if(c == true) { fprintf(stderr,"ERROR (flag given twice: -c)\n%s\n",message); exit(1); }
				c_port = atoi(argv[i+1]);
				if( c_port <= 0 ) { fprintf(stderr,"ERROR (bad command_port)\n"); exit(1); }
				c = true;break;
			case 't':
				if(t == true) { fprintf(stderr,"ERROR (flag given twice: -t)\n%s\n",message); exit(1); }
				nthreads = atoi(argv[i+1]);
				if( nthreads <= 0 || nthreads > MAX_THREADS ) { fprintf(stderr,"ERROR (bad num_of_threads)\n"); exit(1); }
				t = true;break;
			case 'd':
				if(d == true) { fprintf(stderr,"ERROR (flag given twice: -d)\n%s\n",message); exit(1); }
				dir = argv[i+1];
				if( ! is_dir(dir) ) { fprintf(stderr,"ERROR (root_dir not a directory)\n"); exit(1); }
				d = true;break;
			default:
				fprintf(stderr,"ERROR (non-existent flag given: -%c)\n%s\n",argv[i][1],message);
				exit(1);
		}
	}
	printf("--------------------------------\n-----This is my Web Server!-----\n--------------------------------\n");
	printf("serving_port   set to %d\ncommand_port   set to %d\nnum_of_threads set to %d\nroot_dir       set to %s\n",s_port,c_port,nthreads,dir);
	printf("--------------------------------\n");
	printf("Do you want to continue? (y/n)\n");
	char ch = getchar();
	if( ch == 'n' || ch == 'N' ) { printf("Exiting...\n"); exit(1); }
	printf("Continuing...\n");
	fseek(stdin,0,SEEK_END);//ignore the rest
}

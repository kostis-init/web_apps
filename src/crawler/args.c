#include <stdio.h>
#include <stdlib.h>

#include "defs.h"

///parse command-line
///arguments in memory
void parse_args(int argc,char** argv)
{
	//error_message
	char* message = "Usage:\n$ ./mycrawler -h host_or_IP -p port -c command_port -t num_of_threads -d save_dir starting_URL";
	
	if( argc != 12) { fprintf(stderr,"ERROR (wrong usage)\n%s\n",message); exit(1); }
	
	//flags, to check that args are unique
	bool h,p,c,t,d; h = p = c = t = d = false;
	
	for (int i = 1; i < argc-2; i+=2)//scan flags
	{
		if(argv[i][0] != '-') { fprintf(stderr,"ERROR (wrong usage)\n%s\n",message); exit(1); }
		switch (argv[i][1])
		{
			case 'h':
				if(h == true) { fprintf(stderr,"ERROR (flag given twice: -h)\n%s\n",message); exit(1); }
				host = argv[i+1];
				h = true;break;
			case 'p':
				if(p == true) { fprintf(stderr,"ERROR (flag given twice: -p)\n%s\n",message); exit(1); }
				port = atoi(argv[i+1]);
				if( port <= 0 ) { fprintf(stderr,"ERROR (bad port)\n"); exit(1); }
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
				save_dir = argv[i+1];
				if( ! is_dir(save_dir) ) { fprintf(stderr,"ERROR (root_dir not a directory)\n"); exit(1); }
				d = true;break;
			default:
				fprintf(stderr,"ERROR (non-existent flag given: -%c)\n%s\n",argv[i][1],message);
				exit(1);
		}
	}
	//starting url
	s_url=argv[11];
	
	printf("--------------------------------\n-----This is my Web Crawler!-----\n--------------------------------\n");
	printf("host        set to %s\nport        set to %d\ncommand_port   set to %d\nnum_of_threads set to %d\nsave_dir       set to %s\nstarting_url set to %s\n",host,port,c_port,nthreads,save_dir,s_url);
	printf("--------------------------------\n");
	printf("Do you want to continue? (y/n)\n");
	char ch = getchar();
	if( ch == 'n' || ch == 'N' ) { printf("Exiting...\n"); exit(1); }
	printf("Continuing...\n");
	fseek(stdin,0,SEEK_END);//ignore the rest
}

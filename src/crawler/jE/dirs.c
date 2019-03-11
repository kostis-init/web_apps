#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "defs.h"

void parse_file(FILE* docfile,char** map)
{
	///save docfile in memory
	///scans file 2 times. 1 to measure every line, 2 to save every line.(to save memory)
	char c;
	int i=0;//iteration number
	fseek(docfile, 0, SEEK_SET);//rewind to the beginning of file
	while(1)//first scanning to measure every line
	{
		c=fgetc(docfile);
		if(c==EOF)break;
		
		int len=0;
		while(c!='\n' && c!=EOF)//count length of line
		{
			len++;
			c=fgetc(docfile);
		}
		
		if((map[i]=malloc((len+1) * sizeof(char)))==NULL)//allocate as much memory, as needed, for every line
		{
			printf("create_map: malloc failure, exiting...\n");
			exit(-1);
		}
		i++;
	}
	i=0;
	fseek(docfile, 0, SEEK_SET);//rewind to the beginning of file
	while(1)//second scanning to save dirs
	{
		c=fgetc(docfile);
		if(c==EOF)break;
		
		int j=0;//counter
		while(c!='\n' && c!=EOF)//save dir
		{
			map[i][j]=c;
			j++;
			c=fgetc(docfile);
		}
		map[i][j]='\0';
		i++;
	}
	map[i]=NULL;//end of dirs
}

void parse_args(int argc,char** argv,char*** dirs,int* numWor)
{
	///parse arguments from command line into memory
	FILE* docfile;
	char* error_msg = "Usage: ./jobExecutor –d docfile –w numWorkers\n";
	if(argc==5)
	{
		if(!strcmp(argv[1],"-d"))
		{
			if(strcmp(argv[3],"-w"))
			{
				printf("%s-Give the numWorkers parameter (-w)\n",error_msg);
				exit(-1);
			}
			if((docfile = fopen(argv[2],"r")) == NULL)	//opening docfile
			{
				printf("%s-Wrong input file\n",error_msg);
				exit(-1);
			}
			if((*numWor = atoi(argv[4])) <= 0)			//parsing numWor
			{
				printf("%s-Wrong numWorkers number\n",error_msg);
				exit(-1);
			}
		}	
		else if(!strcmp(argv[3],"-d"))
		{
			if(strcmp(argv[1],"-w"))
			{
				printf("%s-Give the numWorkers parameter (-w)\n",error_msg);
				exit(-1);
			}
			if((docfile = fopen(argv[4],"r")) == NULL)	//opening docfile
			{
				printf("%s-Wrong input file\n",error_msg);
				exit(-1);
			}
			if((*numWor = atoi(argv[2])) <= 0)			//parsing numWor
			{
				printf("%s-Wrong numWorkers number\n",error_msg);
				exit(-1);
			}
		}		
		else	
		{
			printf("%s-Give the docfile parameter (-i)\n",error_msg);
			exit(-1);
		}
	}
	else if(argc==3)//default numWor
	{
		if(!strcmp(argv[1],"-d"))
		{
			if((docfile = fopen(argv[2],"r")) == NULL)	//opening docfile
			{
				printf("%s-Wrong input file\n",error_msg);
				exit(-1);
			}
			*numWor = DEFAULT_NUMWOR;
		}
		else
		{
			printf("%s-Give the docfile parameter (-d)\n",error_msg);
			exit(-1);
		}
	}
	else
	{
		printf("%s-Wrong number of arguments\n",error_msg);
		exit(-1);
	}
	
	//load dirs from docfile
	int lines = fcount_lines(docfile);
	if((*dirs = malloc((lines+1) * sizeof(char *)))==NULL){printf("malloc error");exit(-1);}
	parse_file(docfile,*dirs);
	
	if(*numWor>lines){printf("numWorkers too big, resetting to %d\n",lines);*numWor=lines;}
	
	fclose(docfile);
}

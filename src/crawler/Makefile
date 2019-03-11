target: main.o args.o help.o threads.o crawler.o
	gcc -o mycrawler main.o args.o help.o threads.o crawler.o -pthread

main.o: main.c
	gcc -c main.c

args.o: args.c
	gcc -c args.c

help.o: help.c
	gcc -c help.c

threads.o: threads.c
	gcc -c threads.c

crawler.o: crawler.c
	gcc -c crawler.c

clean:
	rm -f mycrawler main.o args.o help.o threads.o crawler.o

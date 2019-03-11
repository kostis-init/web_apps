target: main.o help.o args.o server.o threads.o net.o
	gcc -o myhttpd main.o help.o args.o server.o threads.o net.o -pthread

main.o: server/main.c
	gcc -c server/main.c

args.o: server/args.c
	gcc -c server/args.c

server.o: server/server.c
	gcc -c server/server.c

threads.o: server/threads.c
	gcc -c server/threads.c

help.o: server/help.c
	gcc -c server/help.c

net.o: server/net.c
	gcc -c server/net.c

clean:
	rm -f myhttpd main.o help.o args.o server.o threads.o net.o

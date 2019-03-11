target: main.o ui.o dirs.o worker.o executor.o help.o trie.o pos.o
	gcc -o jobExecutor main.o ui.o dirs.o worker.o executor.o help.o trie.o pos.o

main.o: main.c
	gcc -c main.c

ui.o: ui.c
	gcc -c ui.c

dirs.o: dirs.c
	gcc -c dirs.c

worker.o: worker.c
	gcc -c worker.c

executor.o: executor.c
	gcc -c executor.c

help.o: help.c
	gcc -c help.c

trie.o: trie.c
	gcc -c trie.c

pos.o: pos.c
	gcc -c pos.c

clean:
	rm -f jobExecutor main.o ui.o dirs.o worker.o executor.o help.o trie.o pos.o

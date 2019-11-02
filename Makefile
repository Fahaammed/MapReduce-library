wc: threadpool.o mapreduce.o distwc.o
	g++ -std=c++11 -pthread -Wall -g -o wordcount threadpool.o mapreduce.o distwc.o

compile: threadpool.h threadpool.cpp mapreduce.h mapreduce.cpp distwc.c
	g++ -std=c++11 -ggdb -c threadpool.cpp
	g++ -std=c++11 -ggdb -c mapreduce.cpp 
	gcc -std=c99 -ggdb -c distwc.c

clean:
	rm -f wordcount.o *.o

compress:
	tar -czf mapreduce.tar.gz distwc.c mapreduce.cpp threadpool.cpp threadpool.h mapreduce.h Makefile readme.md
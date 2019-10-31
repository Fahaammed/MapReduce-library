wc: compile
	g++ -Wall -Wextra -g -std=c++11 -o wordcount distwc.cpp threadpool.o mapreduce.o -lpthread

compile:
	g++ -Wall -Wextra -g -std=c++11 -c threadpool.cpp -lpthread
	g++ -Wall -Wextra -g -std=c++11 -c mapreduce.cpp -lpthread

clean:
	rm *.o
	rm wc

compress:
	tar -czf mapreduce.tar.gz distwc.cpp mapreduce.cpp threadpool.cpp threadpool.h mapreduce.h Makefile readme.md
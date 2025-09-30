main:main.cc ./PageCache/PageCache.cpp ./ThreadCache/ThreadCache.cpp ./CentralCache/CentralCache.cpp
	g++ -o $@ $^ -std=c++11 -lpthread -g
.PHONY:clean
clean:
	rm -f main
test:UnitTest.cc
	g++ -o $@ $^ -std=c++11 -pthread
.PHONY:clean
clean:
	rm -f test 
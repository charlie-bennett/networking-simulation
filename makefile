CC:=g++
CFLAGS +=g++ -std=c++14 -Wall -Wextra -pedantic-errors

all: 
	g++ -std=c++11 serverA.cpp -o A
	g++ -std=c++11 serverB.cpp -o B
	g++ -std=c++11 client.cpp -o client
	g++ -std=c++11 AWS.cpp -o aws

clean: 
	$(RM) aws
	$(RM) client
	$(RM) A
	$(RM) B
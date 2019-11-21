CC:=g++
CFLAGS +=g++ -std=c++14 -Wall -Wextra -pedantic-errors -pthread
LFLAGS = -pthread
all: 
	g++ -std=c++11 serverA.cpp -o A
	g++ -std=c++11 serverB.cpp -o B
	g++ -std=c++11 client.cpp -o client
	g++ -lpthread -std=c++11 AWS.cpp -o aws 

clean: 
	$(RM) aws
	$(RM) client
	$(RM) A
	$(RM) B
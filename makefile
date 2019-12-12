CC:=g++
CFLAGS += g++ -std=c++11 -Wall

objects = serverA.o serverB.o aws.o client.o

all: 
	$(CFLAGS) serverA.cpp -o serverA
	$(CFLAGS) serverB.cpp -o serverB
	$(CFLAGS) AWS.cpp -o AWS
	$(CFLAGS) client.cpp -o client

serverA:
	$(CFLAGS) serverA.cpp -o serverA

serverB:
	$(CFLAGS) serverB.cpp -o serverB

AWS:
	$(CFLAGS) AWS.cpp -o AWS

client:
	$(CFLAGS) client.cpp -o client

clean: 
	$(RM) client
	$(RM) AWS
	$(RM) serverA
	$(RM) serverB

CC:=g++
CFLAGS += g++ -std=c++11 -Wall

objects = serverA.o serverB.o aws.o client.o

all: 
	$(CFLAGS) serverA.cpp -o serverA
	$(CFLAGS) serverB.cpp -o serverB
	$(CFLAGS) aws.cpp -o aws 
	$(CFLAGS) client.cpp -o client

serverA:
	$(CFLAGS) serverA.cpp -o serverA

serverB:
	$(CFLAGS) serverB.cpp -o serverB

aws:
	$(CFLAGS) aws.cpp -o aws

client:
	$(CFLAGS) client.cpp -o client

clean: 
	$(RM) client
	$(RM) aws
	$(RM) serverA
	$(RM) serverB
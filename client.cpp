/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include<iostream>
#include <vector>
#include <sstream>
#include <iomanip>

#include <arpa/inet.h>

#define PORT "23095" // the port client will be connecting to 
#define MAXDATASIZE 1000 // max number of bytes we can get at once 
using namespace std;
#define RELINF 1000000


string from_cstring(char* input)
{
	string output;
	output += input;
	return output;
}

vector<string> delimit(string input, char delimiter, int max_size = RELINF)
{
	vector<string> output;
	string::iterator head = input.begin();
	for (string::iterator tail = input.begin(); tail != input.end(); ++tail)
	{
		char word = *tail;
		if (word == delimiter)
		{
			if (head != tail) output.push_back(string(head, tail));
			head = next(tail, 1);
			if (output.size() == max_size)
			{
				return output;
			}
		}
	}
	//add last element
	if (*prev(input.end(), 1) != ' ')
	{
		output.push_back(string(head, input.end()));
	}
	return output;
}

char* to_cstring(string input)
{
	char* output = new char[input.size()];
	for (int i = 0; i < input.size(); i++)
	{
		output[i] = input[i];
	}
	return output;
}

void print_matrix(vector<string> data, vector<int> widths)
{
	vector<string>::iterator iter = data.begin();
	while (iter != data.end())
	{
		for (auto width : widths)
		{
			if (iter->size() > width)
			{
				//asume number
				long double number = stold(to_cstring(*iter));
				std::ostringstream mystream;
				mystream << std::setprecision(5) << number;
				string new_number = mystream.str();
				cout << std::setw(width) << std::setfill(' ') << std::left << new_number;
			}
			else cout << std::setw(width) << std::setfill(' ') << std::left << *iter;
			std::advance(iter, 1);
		}
		cout << endl;
	}
}

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int get_port(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return (((struct sockaddr_in*)sa)->sin_port);
	}

	return (((struct sockaddr_in6*)sa)->sin6_port);
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("Error: Expexted 3 arguements, recieved %i", argc);
		return 1;
	}
	cout << "The Client is up and running." << endl;
	int sockfd, numbytes ;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("127.0.0.1", to_cstring(PORT), &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		                     p->ai_protocol)) == -1)
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}


	if (p == NULL)
	{
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr),
	          s, sizeof s);
	auto getsock_check = getsockname(sockfd, (struct sockaddr*)get_in_addr((struct sockaddr*)p->ai_addr), (socklen_t*)&p->ai_addrlen);
	//cout << (((struct sockaddr_in*)p->ai_addr)->sin_port) << endl;


	freeaddrinfo(servinfo); // all done with this structure
//TODO
	/*
	/*Retrieve     the  locally-bound  name of   the  specified
	 socket and store it in the sockaddr structure
	getsock_check = getsockname(TCP_Connect_Sock, (struct sockaddr
	                            *)&my_addr, (socklen_t*)&addrlen);
	//Error checking
	if (getsock_check == -1)
	{
		perror("getsockname"); exit(1);
	}
	//use CONNECT MAYBE NOT BIND
	*/


	if (getsock_check == -1)
	{
		perror("getsockname"); exit(1);
	}
	string to_send;
	for (int i = 1; i < argc; i++)
	{
		to_send += argv[i];
		to_send += ' ';
	}
	to_send += '\0';

	if (send(sockfd, to_cstring(to_send), to_send.size(), 0) == -1)
		perror("send");
	printf("The client has sent query to AWS using TCP over \n port %i: start vertex %s; map %s; file size %s. \n",
	       get_port(p->ai_addr), argv[2], argv[1], argv[3]);
	while (1)
	{
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
		{
			perror("recv");
			exit(1);
		}

		else if (numbytes > 1)
		{
			printf("listener: packet is %d bytes long\n", numbytes);
			break;
		}
	}

	buf[numbytes] = '\0';
	cout << buf << endl;
	vector<string> output = delimit(from_cstring(buf), ' ');
	for (auto entry : output)
	{
		cout << entry << endl;
	}
	output = vector<string>(output.begin() + 5, output.end());
	cout << "The client has recieved the results form AWS:" << endl;
	cout << "-----------------------------------------------------------" << endl;
	cout << "Destination    Min Length    Tt        Tp       Delay      " << endl;
	cout << "-----------------------------------------------------------" << endl;
	vector<int> widths;
	widths.push_back(string("Destination    ").size());
	widths.push_back(string("Min Length    ").size());
	widths.push_back(string("Tt        ").size());
	widths.push_back(string("Tp       ").size());
	widths.push_back(string("Delay      ").size());
	print_matrix(output, widths);
	cout << "-----------------------------------------------------------" << endl;



	close(sockfd);

	return 0;
}
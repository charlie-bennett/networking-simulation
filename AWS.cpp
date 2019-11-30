#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sstream>
#include <sys/wait.h>
#define MYPORT "23095"
#define SERVERA "21095"    // the port users will be connecting to
#define SERVERB "22095"
#define MAXBUFLEN 10000
using namespace std;
#define RELINF 1000000
#define MYIPADDRESS "127.0.0.1"
#define BACKLOG 2   // how many pending connections queue will hold
//TODO: Make helper funciton.h
//TODO: CHILD SOCKETS

#define MAXDATASIZE 1000 // max number of bytes we can get at once
char buf[MAXBUFLEN];

int new_fd;
bool do_better = 1;

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
struct file_descriptor
{
	int tcp_parent;
	int tcp_child;
	int udp;
};
file_descriptor fd;

string from_cstring(char* input)
{
	string output;
	output += input;
	return output;
}

char* to_cstring(string input)
{
	char* output = new char[input.size()];
	for (int i = 0; i < input.size(); i++)
	{
		output[i] = input[i];
		if (input[i] == '/0') return output;
	}
	return output;
}

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while (waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}
//the following is from Beej guid
//************
// get sockaddr, IPv4 or IPv6:
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
				cout << *iter << endl;
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


	return;
}
void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

class TCP
{
public:
	int sock_fd_child, sock_fd_parent, numbytes, init_rv = 0;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN];
	int rv, yes = 1;
	TCP()
	{
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE; // use my IP
		if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			init_rv = 1;
		}
		// loop through all the results and bind to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next)
		{
			if ((sock_fd_parent = socket(p->ai_family, p->ai_socktype,
			                             p->ai_protocol)) == -1)
			{
				perror("server: socket");
				continue;
			}

			if (setsockopt(sock_fd_parent, SOL_SOCKET, SO_REUSEADDR, &yes,
			               sizeof(int)) == -1)
			{
				perror("setsockopt");
				exit(1);
			}

			if (::bind(sock_fd_parent, p->ai_addr, p->ai_addrlen) == -1)
			{
				close(sock_fd_parent);
				perror("server: bind");
				continue;
			}

			break;
		}

		freeaddrinfo(servinfo); // all done with this structure

		if (p == NULL)
		{
			fprintf(stderr, "server: failed to bind\n");
			exit(1);
		}

		if (::listen(sock_fd_parent, BACKLOG) == -1)
		{
			perror("listen");
			exit(1);
		}

		sa.sa_handler = sigchld_handler; // reap all dead processes
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1)
		{
			perror("sigaction");
			exit(1);
		}
	}
	int listen()
	{
		printf("server: waiting for connections...\n");

		while (1)   // main accept() loop
		{
			sin_size = sizeof their_addr;
			sock_fd_child = accept(sock_fd_parent, (struct sockaddr*)&their_addr, &sin_size);
			if (sock_fd_child == -1)
			{
				perror("accept");
				continue;
			}

			inet_ntop(their_addr.ss_family,
			          get_in_addr((struct sockaddr*)&their_addr),
			          s, sizeof s);
			printf("server: got connection from %s\n", s);
			/*
			if (!fork())   // this is the child process
			{
				//close(sock_fd_parent); // child doesn't need the listener
				//added
				cout << "inside fork" << endl;
				if ((numbytes = recv(sock_fd_child, buf, MAXDATASIZE - 1, 0)) == -1)
				{
					perror("recv");
					exit(1);
					return 1;
				}
				buf[numbytes] = '\0';
				printf("host: received '%s'\n", buf);

				return numbytes;
			}
			*/
			//close(sock_fd_parent); // child doesn't need the listener
			//added
			cout << "inside fork" << endl;
			if ((numbytes = recv(sock_fd_child, buf, MAXDATASIZE - 1, 0)) == -1)
			{
				perror("recv");
				exit(1);
				return 1;
			}
			buf[numbytes] = '\0';
			printf("host: received '%s'\n", buf);

			return numbytes;
			close(sock_fd_child);  // parent doesn't need this

		}

		return 0;
	}

	int talk(int message_size)
	{
		buf[message_size] = '\0';
		while (1)
		{
			/*
			if (!fork())
			{
				if (::send(sock_fd_child, buf, 100, 0) == -1)
					perror("send");
				close(sock_fd_child);
				return 0;
			}
			*/
			if (::send(sock_fd_child, buf, message_size, 0) == -1)
				perror("send");
			close(sock_fd_child);
			return 0;
		}
		return 0;
		close(sock_fd_child);
	}
	void block()
	{
		while (true)
		{
			if ((numbytes = recv(sock_fd_child, buf, MAXDATASIZE - 1, 0)) == -1)
			{
				break;
			}
		}
	}
	~TCP()
	{
		close(sock_fd_parent);
	}

};
class UDP
{
public:
	struct addrinfo hints, *my_address, *servinfo, *A_address, *B_address;
	int sock_fd, init_rv = 0;
	map<string, addrinfo*> addresses;
	UDP()
	{
		cout << "enter UDP init" << endl;
		memset(&this->hints, 0, sizeof this->hints);
		this->hints.ai_family = AF_INET;
		this->hints.ai_socktype = SOCK_DGRAM;
		int rv;
		if ((rv = getaddrinfo(to_cstring(MYIPADDRESS), to_cstring(MYPORT), &(this->hints), &(this->servinfo))) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			init_rv = 1;
		}
		for (my_address = servinfo; my_address != NULL; my_address = my_address->ai_next)
		{

			if ((sock_fd = socket(my_address->ai_family, my_address->ai_socktype,
			                      my_address->ai_protocol)) == -1)
			{
				perror("listener: socket");
				continue;
			}

			if (::bind(sock_fd, my_address->ai_addr, my_address->ai_addrlen) == -1)
			{
				close(sock_fd);
				perror("listener: bind");
				continue;
			}


			break;
		}
		if (my_address == NULL)
		{
			fprintf(stderr, "listener: failed to bind socket\n");
			init_rv = 2;
		}
		if ((rv = getaddrinfo(to_cstring(MYIPADDRESS), to_cstring(SERVERA), &hints, &A_address)) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			init_rv = 1;
		}
		if ((rv = getaddrinfo(to_cstring(MYIPADDRESS), to_cstring(SERVERB), &hints, &B_address)) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			init_rv = 1;
		}
		addresses.insert({"A", A_address});
		addresses.insert({"B", B_address});
		addresses.insert({"me", my_address});
		cout << "exit udp_init" << endl;

	}
	int send(string server_ID, int message_size)
	{
		buf[message_size] = '\0';
		int numbytes;
		struct addrinfo* p = addresses[server_ID];
		if ((numbytes = sendto(this->sock_fd, buf, strlen(buf), 0,
		                       p->ai_addr, p->ai_addrlen)) == -1)
		{
			perror("talker: sendto");
			exit(1);
			return 1;
		}


		printf("talker: sent %d bytes to %s\n", numbytes, MYIPADDRESS);
		return 0;
	}
	int recieve()
	{
		int rv;
		int numbytes;
		struct sockaddr_storage their_addr;
		//char buf[MAXBUFLEN];
		socklen_t addr_len;
		char s[INET6_ADDRSTRLEN];
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sock_fd, buf, MAXBUFLEN - 1 , 0,
		                         (struct sockaddr*)&their_addr, &addr_len)) == -1)
		{
			perror("recvfrom");
			exit(1);
			return 1;
		}


		printf("listener: got packet from %s\n",
		       inet_ntop(their_addr.ss_family,
		                 get_in_addr((struct sockaddr*)&their_addr),
		                 s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);
		cout << "exit udp listen" << endl;
		return numbytes;
	}

	~UDP()
	{
		for (auto address : addresses)
		{
			freeaddrinfo(address.second);
		}
		close(sock_fd);
	}
};



int main()
{
	int message_size = 0;
	TCP tcp;
	UDP udp;
	int count = 0;
	vector<int> widths;
	cout << "The AWS is up and running." << endl;
	while (true)
	{
		message_size = tcp.listen();
		vector<string> input = delimit(from_cstring(buf), ' ');
		printf("The AWS has recieved MAP ID %s, start vertex %s /n", to_cstring(input[0]), to_cstring(input[1]));
		printf("and file size %s from the client using TCP \n", to_cstring(input[2]));
		printf("over port %s \n", to_cstring(MYPORT));
		if (udp.send("A", message_size)) {}
		printf("The AWS has sent map ID and starting vertex to serer A using UPD over port %s\n",
		       to_cstring(SERVERA));

		message_size = udp.recieve();
		cout << "The AWS has has recieved the shortest path from server A: " << endl;
		cout << "---------------------------------" << endl;
		cout << "Destination      Min Length      " << endl;
		cout << "---------------------------------" << endl;
		widths.clear();
		widths.push_back(string("Destination      ").size());
		widths.push_back(string("Min Length      ").size());
		input = delimit(from_cstring(buf), ' ');
		print_matrix(vector<string>(input.begin() + 6, input.end()), widths);
		cout << "---------------------------------" << endl;
		if (udp.send("B", message_size)) {}
		cout << "The AWS has sent path length, propagatin speed and transmission speed" << endl;
		cout << "to server B using UDP over port " << SERVERB << endl;
		message_size = udp.recieve();
		cout << "The AWS has recieved delays from server B" << endl;
		cout << "-----------------------------------------" << endl;
		cout << "Destination      Tt      Tp      Delay   " << endl;
		cout << "-----------------------------------------" << endl;
		widths.clear();
		widths.push_back(string("Destination      ").size());
		widths.push_back(string("Tt      ").size());
		widths.push_back(string("Tp      ").size());
		widths.push_back(string("Delay   ").size());
		input = delimit(from_cstring(buf), ' ');
		input = vector<string>(input.begin() + 6, input.end());
		count = 0;
		for (vector<string>::iterator it = input.begin(); it != input.end(); ++it)
		{
			if (count % 5 == 1) input.erase(it);
			count++;
		}
		print_matrix(input, widths);
		cout << "-----------------------------------------" << endl;
		cout << "The AWS has sent calculated dleay to client using TCP over port %s\n" << MYPORT << endl;

		if (tcp.talk(message_size)) {}
		cout << "The AWS has sent calculated dleay to client using TCP over port %s\n" << MYPORT << endl;
		tcp.block();
	}

	return 0;
}
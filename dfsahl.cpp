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
#include <sys/wait.h>
#define MYPORT "23095"
#define SERVERA "21095"    // the port users will be connecting to
#define SERVERB "22095"
#define MAXBUFLEN 100000
using namespace std;
#define RELINF 1000000
#define MYIPADDRESS "127.0.0.1"
#define BACKLOG 2   // how many pending connections queue will hold
//TODO: Make helper funciton.h
//TODO: CHILD SOCKETS

#define MAXDATASIZE 100 // max number of bytes we can get at once
char buf[MAXBUFLEN];

int new_fd;

struct file_descriptor
{
	int tcp_parent;
	int tcp_child;
	int udp;
};
file_descriptor fd;



char* to_cstring(string input)
{
	char* output = new char[input.size()];
	for (int i = 0; i < input.size(); i++)
	{
		output[i] = input[i];
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
void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// getcpt sockaddr, IPv4 or IPv6:

/*
int tcp_init()
{

}*/
/*
int tcp_recieve()
{

	//TCP Variables
	int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];


	struct sigaction sa;
	int yes = 1;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, to_cstring(MYPORT), &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		                     p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
		               sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
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

	if (listen(sockfd, BACKLOG) == -1)
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

	printf("server: waiting for connections...\n");
	return 0;
	while (1)   // main accept() loop
	{
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
		if (new_fd == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
		          get_in_addr((struct sockaddr*)&their_addr),
		          s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork())   // this is the child process
		{
			close(sockfd); // child doesn't need the listener
			//added

			if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1)
			{
				perror("recv");
				exit(1);
			}
			buf[numbytes] = '\0';

			printf("host: received '%s'\n", buf);
			/*

			if (send(new_fd, buf, 13, 0) == -1)
				perror("send");


			return 0;
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
*/

int tcp_recieve()
{


	int numbytes;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	//char buf[MAXDATASIZE];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((fd.tcp_parent = socket(p->ai_family, p->ai_socktype,
		                            p->ai_protocol)) == -1)
		{
			perror("server: socket");
			continue;
		}

		if (setsockopt(fd.tcp_parent, SOL_SOCKET, SO_REUSEADDR, &yes,
		               sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(1);
		}

		if (::bind(fd.tcp_parent, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(fd.tcp_parent);
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

	if (listen(fd.tcp_parent, BACKLOG) == -1)
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

	printf("server: waiting for connections...\n");

	while (1)   // main accept() loop
	{
		sin_size = sizeof their_addr;
		fd.tcp_child = accept(fd.tcp_parent, (struct sockaddr*)&their_addr, &sin_size);
		if (fd.tcp_child == -1)
		{
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
		          get_in_addr((struct sockaddr*)&their_addr),
		          s, sizeof s);
		printf("server: got connection from %s\n", s);
		if (!fork())   // this is the child process
		{
			close(fd.tcp_parent); // child doesn't need the listener
			//added
			cout << "inside fork" << endl;
			if ((numbytes = recv(fd.tcp_child, buf, MAXDATASIZE - 1, 0)) == -1)
			{
				perror("recv");
				exit(1);
			}
			buf[numbytes] = '\0';
			printf("host: received '%s'\n", buf);

			//close(new_fd);
			//exit(0);

			return 0;
		}
		close(fd.tcp_child);  // parent doesn't need this
	}

	return 0;


}

int tcp_send()
{
	while (1)
	{
		if (!fork())
		{
			if (send(new_fd, buf, 100, 0) == -1)
				perror("send");
			close(new_fd);
			return 0;
		}
	}
	close(new_fd);
	return 0;
}


//****************

int udp_send(char* message, char* port) //please dont forget terminating char
{

	cout << "enter udp talk" << endl;
	//int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	if (0) //(argc != 3)
	{
		fprintf(stderr, "usage: talker hostname message\n");
		exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(MYIPADDRESS, port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((fd.udp = socket(p->ai_family, p->ai_socktype,
		                     p->ai_protocol)) == -1)
		{
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	if ((numbytes = sendto(fd.udp, message, strlen(message), 0,
	                       p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, MYIPADDRESS);
	close(fd.udp);
	cout << "exit udp talk" << endl;
	return 0;
}

int udp_listen()
{
	cout << "enter udp listen" << endl;
	//from beej
	//********************************
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	//char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, to_cstring(MYPORT), &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		                     p->ai_protocol)) == -1)
		{
			perror("listener: socket");
			continue;
		}
		if (::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("listener: bind");
			continue;
		}


		break;
	}

	if (p == NULL)
	{
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1 , 0,
	                         (struct sockaddr*)&their_addr, &addr_len)) == -1)
	{
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n",
	       inet_ntop(their_addr.ss_family,
	                 get_in_addr((struct sockaddr*)&their_addr),
	                 s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);
	close(sockfd);
	//********************************
	cout << "exit udp listen" << endl;
	return 0;

}

int main()
{
	while (true)
	{
		//if (tcp_init()) {}
		//if (tcp_recieve()) {} //whoopss
		//if (udp_send(buf, SERVERA)) {} //
		if (udp_send(to_cstring("A 2 20"), SERVERA)) {} //whoops
		if (udp_listen()) {} //whoops
		cout << "inside main" << endl;
		if (udp_send(buf, SERVERB)) {} //whoops
		if (udp_listen()) {} //whoops
		//if (tcp_send()) {}
	}

	return 0;
}
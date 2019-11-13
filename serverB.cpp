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
#include <math.h>
#include <sstream>
#define MYPORT "22095"
#define AWS "23095"   // the port users will be connecting to
#define MAXBUFLEN 100
using namespace std;
#define RELINF 1000000
#define MYIPADDRESS "127.0.0.1"

struct request_params
{
	int file_size;
	char mapID;
	int num_v;
	int num_e;
	double prop_speed;
	double trans_speed;
	int* distances[2];
};

class Request
{
public:
	char* response;
	void dump()
	{
		string output = "";
		output += to_string(file_size);
		output += "||";
		output += mapID;
		output += "||";
		output += to_string(num_v);
		output += "||";
		output += to_string(num_e);
		output += "||";
		output += to_string(prop_speed);
		output += "||";
		output += to_string(trans_speed);
		output += "||";
		std::ostringstream change_percision;

		for (map<string, vector<double> >::iterator it = delay.begin(); it != delay.end(); ++it)
		{
			for (vector<double>::iterator iter = it->second.begin(); iter != it->second.end(); ++iter)
			{
				//change_percision.clear();
				change_percision.str("");
				change_percision.precision(3);
				change_percision << fixed << *iter;
				output += change_percision.str();
				//output += to_string(*iter);
				output += "||";
			}
		}
		response = new char[output.size()];
		for (int i = 0; i < output.size(); i++)
		{
			response[i] = output[i];
		}
		return;
	}
	Request(request_params* incoming_request)
	{

		this->file_size = incoming_request->file_size;
		this->mapID = incoming_request->mapID;
		this->num_v = incoming_request->num_v;
		this-> num_e = incoming_request->num_e;
		this-> prop_speed = incoming_request->prop_speed;
		this->trans_speed = incoming_request->trans_speed;
		this->distances = incoming_request->distances;
		delay["Transmission"] = vector<double>(num_v);
		delay["Propagation"] = vector<double>(num_v);
		delay["Total"] = vector<double>(num_v);
		delay["Destination"] = vector<double>(num_v);
		for (int i = 0; i < num_v; i++)
		{
			delay["Transmission"][i] = (double) trans_speed * file_size;
			delay["Propagation"][i] = (double) prop_speed * distances[1][i];
			delay["Total"][i] = delay["Propagation"][i] + delay["Transmission"][i];
			delay["Destination"][i] = distances[0][i];
		}
		dump();


	}
	void print()
	{
		cout << endl;
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		cout << "Destination Delay" << endl;
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		for (int i = 0; i < num_v; i++)
		{
			cout << delay["Destination"][i] << " ";
			cout << setw(strlen("Destination")) << delay["Total"][i] << " ";
			cout << endl;
		}
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;

	}

	int* prop;
	map<string, vector<double> > delay;
	int file_size;
	char mapID;
	int num_v;
	int num_e;
	double prop_speed;
	double trans_speed;
	int** distances;
	char buffer[3000];


};



int next_index(char* buf, int start, int size)
{
	for (int i = start; i < size; i++)
	{
		if (buf[i] == '|')
		{
			return i + 2;
		}
	}
	return -1;
}

void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int udp_listen(request_params* incoming_request, bool boot_up)
{
	//from beej
	//********************************
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0)
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

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
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

	if (boot_up)
	{
		printf("The Server B is up and running using UDP on port %d", MYPORT);
	}
	else {} //do something here

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
	incoming_request->file_size = atoi(buf);
	int start = 0;
	start = next_index(buf, start, numbytes);
	incoming_request->mapID = buf[start];
	start = next_index(buf, start, numbytes);
	incoming_request->num_v = atoi(buf + start);
	start = next_index(buf, start, numbytes);
	incoming_request->num_e = atoi(buf + start);
	start = next_index(buf, start, numbytes);
	incoming_request->prop_speed = atoi(buf + start);
	start = next_index(buf, start, numbytes);
	incoming_request->trans_speed = atoi(buf + start);
	start = next_index(buf, start, numbytes);
	int node = 0;
	int count = 0;


	incoming_request->distances[0] = new int[incoming_request->num_v];
	incoming_request->distances[1] = new int[incoming_request->num_v];
	while (start < numbytes)
	{

		incoming_request->distances[node][count] = atoi(buf + start);
		if (node) count++;
		node = !(node);
		start = next_index(buf, start, numbytes);
	}

	return 0;

}

int udp_send(char* message, char* port) //please dont forget terminating char
{

	int sockfd;
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

	if ((rv = getaddrinfo(MYIPADDRESS, port, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
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

	if ((numbytes = sendto(sockfd, message, strlen(message), 0,
	                       p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("talker: sendto");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("talker: sent %d bytes to %s\n", numbytes, MYIPADDRESS);
	close(sockfd);

	return 0;
}



int main()
{
	request_params* incoming_request = new request_params;
	udp_listen(incoming_request, true);
	//recieved data
	cout << endl << "The Server B has recieved data for calculation:" << cout << endl;
	printf("*Propagation speed: %f km / s\n", incoming_request->prop_speed);
	printf("*Transmiission speed: %f Bytes / s\n", incoming_request->trans_speed);
	for (int i = 0; i < incoming_request->num_v; i++)
	{
		printf("*Path length for destination %d : %d\n", incoming_request->distances[0][i],
		       incoming_request->distances[1][i]);
	}

	Request* processed_request = new Request(incoming_request);
	processed_request->print();
	if (udp_send(processed_request->response, AWS)) {} //whoops
	cout << endl << "The Server B has finished sending the output to AWS" << endl;





}

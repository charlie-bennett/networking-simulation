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
int string_to_int(string me)
{
	char* output = new char[me.size()];
	for ( int i = 0; i < me.size(); i++)
	{
		output[i] =  me[i];
	}

	return atoi(output);
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

			output.push_back(string(head, tail));
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

string from_cstring(char* input)
{
	string output;
	output += input;
	return output;
}
template<class T>
class request_params
{
public:
	void set_info(vector<string> input)
	{
		copy(input.begin(), input.end(), back_inserter(this->info));
		return;
	}
	void print_input()
	{
		printf("*Propagation speed: %f km / s\n", this->prop_speed);
		printf("*Transmiission speed: %f Bytes / s\n", this->trans_speed);
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		for (auto node : this->nodes)
		{
			printf("*Path length for destination %s : %f\n", to_cstring(node->vertexID), node->dist);
		}
		cout << endl;
		return;

	}
	string get_output()
	{
		for (auto entry : this->info)
		{
			output += entry;
			output += ' ';
		}
		for (auto node : this->nodes)
		{
			for (auto entry : node->info)
			{
				output += entry;
				output += ' ';
			}
		}
		return output;
	}
	void print_output()
	{
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		cout << "Destination Delay" << endl;
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		for (auto node : this->nodes)
		{
			cout << node->vertexID << std::setw(strlen("Destination")) << node->total_delay << endl;
		}
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		return;
	}
	int file_size;
	string mapID;
	int num_v;
	int num_e;
	double prop_speed;
	double trans_speed;
	vector<T> nodes;
	vector<string> info;
	string output;

};
class Node
{
public:
	Node(string vertexID, double dist, request_params<Node*>* parent_request) : dist(dist), vertexID(vertexID), parent_request(parent_request)
	{
		this->trans_delay = parent_request->trans_speed * parent_request->file_size;
		this->prop_delay = parent_request->prop_speed * this->dist;
		this->total_delay = this->trans_delay * this->prop_delay;
		this->info.push_back(vertexID);
		this->info.push_back(to_string(trans_delay));
		this->info.push_back(to_string(prop_delay));
		this->info.push_back(to_string(total_delay));
	}
	double dist;
	double trans_delay;
	double prop_delay;
	double total_delay;
	vector<string> info;
	request_params<Node*>* parent_request;
	string vertexID;
};

/*
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
	//map<string, vector<double> > delay;
	int file_size;
	char mapID;
	int num_v;
	int num_e;
	double prop_speed;
	double trans_speed;
	//int** distances;
	//map < string, this->Delay > nodes;
	char buffer[3000];


};
*/





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

int udp_listen(request_params<Node*>* incoming_request, bool boot_up)
{
	bool debug = 1;
	if (debug)
	{
		char* buf = "20 A 6 5 1.000000 2.000000 0 24 1 31 2 0 3 15 4 20 5 22 ";

		vector<string> output = delimit(from_cstring(buf), ' ');
		cout << "OUTPUT" << endl;
		for (auto entry : output)
		{
			cout << entry << endl;
		}
		incoming_request->file_size = string_to_int(output[0]);
		incoming_request->mapID = output[1];
		incoming_request->num_v = string_to_int(output[2]);
		incoming_request->num_e = string_to_int(output[3]);
		incoming_request->prop_speed = stod(output[4]);

		incoming_request->trans_speed = stod(output[5]);

		incoming_request->set_info(vector<string>(output.begin(), next(output.begin(), 5)));

		for (std::pair<vector<string>::iterator, vector<string>::iterator>
		        it(next(output.begin(), 5), next(output.begin(), 6));
		        it.first != prev(output.end(), 1);
		        it.first = next(it.first, 2), it.second = next(it.second, 2))
			//advance(it.first, 2), advance(it.second, 2))
		{
			incoming_request->nodes.push_back(new Node(*it.first, stod(*it.second), incoming_request));



		}

		return 0;
	}
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

	if (boot_up)
	{
		printf("The Server B is up and running using UDP on port %c", to_cstring(MYPORT));
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
	vector<string> output = delimit(from_cstring(buf), ' ');
	incoming_request->file_size = string_to_int(output[0]);
	incoming_request->mapID = output[1];
	incoming_request->num_v = string_to_int(output[2]);
	incoming_request->num_e = string_to_int(output[3]);
	incoming_request->prop_speed = stod(output[4]);
	incoming_request->trans_speed = stod(output[5]);
	incoming_request->set_info(vector<string>(output.begin(), next(output.begin(), 5)));

	for (std::pair<vector<string>::iterator, vector<string>::iterator>
	        it(next(output.begin(), 5), next(output.begin(), 6));
	        it.first != prev(output.end(), 1);
	        it.first = next(it.first, 2), it.second = next(it.second, 2))
	{
		incoming_request->nodes.push_back(new Node(*it.first, stod(*it.second), incoming_request));
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

	request_params<Node*>* incoming_request = new request_params<Node*>;
	udp_listen(incoming_request, true); //is this blocking?
	//recieved data
	cout << endl << "The Server B has recieved data for calculation:" << endl;
	incoming_request->print_input();
	incoming_request->print_output();
	if (udp_send(to_cstring(incoming_request->get_output()), AWS)) {} //whoops
	cout << endl << "The Server B has finished sending the output to AWS" << endl;


}

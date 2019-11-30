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
#define MAXBUFLEN 1000
#define PRECISION 3
using namespace std;
#define RELINF 1000000
#define MYIPADDRESS "127.0.0.1"
char buf[MAXBUFLEN];
int long long string_to_int(string me)
{
	char* output = new char[me.size()];
	for ( int i = 0; i < me.size(); i++)
	{
		output[i] =  me[i];
	}

	return atoll(output);
}

string double_to_string(double input)
{
	ostringstream stream;
	stream << std::fixed;
	stream << setprecision(PRECISION);
	stream << input;
	return stream.str();

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

string from_cstring(char* input)
{
	string output;
	output += input;
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
//template<class T>
class request_params
{
private:
	vector<string> info;
	string output;
public:
	int long long file_size;
	string mapID;
	int num_v;
	int num_e;
	double prop_speed;
	double trans_speed;
	//vector<T> nodes;
	class Node
	{
	public:
		Node(string vertexID, double dist, request_params* parent_request) : dist(dist), vertexID(vertexID), parent_request(parent_request)
		{
			this->trans_delay = double(parent_request->file_size) / (parent_request->trans_speed * 8);
			this->prop_delay = this->dist / parent_request->prop_speed;
			this->total_delay = this->trans_delay + this->prop_delay;
			this->info.push_back(vertexID);
			this->info.push_back(double_to_string(this->dist)); //newly added
			this->info.push_back(double_to_string(trans_delay));
			this->info.push_back(double_to_string(prop_delay));
			this->info.push_back(double_to_string(total_delay));
		}
		double dist;
		double trans_delay;
		double prop_delay;
		double total_delay;
		vector<string> info;
		request_params* parent_request;
		string vertexID;
	};
	vector<Node*> nodes;
	request_params()
	{
		vector<string> output = delimit(from_cstring(buf), ' ');
		this->file_size = string_to_int(output[0]);
		this->mapID = output[1];
		this->num_v = string_to_int(output[2]);
		this->num_e = string_to_int(output[3]);
		this->prop_speed = stod(output[4]);
		this->trans_speed = stod(output[5]);
		this->set_info(vector<string>(output.begin(), next(output.begin(), 5)));

		for (std::pair<vector<string>::iterator, vector<string>::iterator>
		        it(next(output.begin(), 6), next(output.begin(), 7));
		        it.first != prev(output.end(), 2);
		        it.first = next(it.first, 2), it.second = next(it.second, 2))
		{
			this->nodes.push_back(new Node(*it.first, stod(*it.second), this));
		}

	}
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
		output += '\0';
		return output;
	}
	void print_output()
	{
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		cout << "Destination     Delay    " << endl;
		vector<int> widths;
		widths.push_back(string("Destination     ").size());
		widths.push_back(string("Delay    ").size());
		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		vector<string> for_grid;
		for (auto node : this->nodes)
		{
			for_grid.push_back(node->vertexID);
			for_grid.push_back(double_to_string(node->total_delay));
			//cout << node->vertexID << std::setw(strlen("Destination")) << node->total_delay << endl;
		}
		print_matrix(for_grid, widths);

		for (int i = 0; i < 50; i++) cout << "-";
		cout << endl;
		return;
	}


};


void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
class UDP
{
public:
	struct addrinfo hints, *my_address, *servinfo, *AWS_address;
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
		if ((rv = getaddrinfo(to_cstring(MYIPADDRESS), to_cstring(AWS), &hints, &AWS_address)) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			init_rv = 1;
		}
		addresses.insert({"me", my_address});
		addresses.insert({"aws", AWS_address});
		cout << "exit udp_init" << endl;

	}
	int send(string server_ID, char* message, int message_size)
	{
		message[message_size] = '\0';
		int numbytes;
		struct addrinfo* p = addresses[server_ID];
		if ((numbytes = sendto(this->sock_fd, message, message_size, 0,
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
		return 0;
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
	UDP udp;
	cout << "The Server B is up and running using UDP on port " << MYPORT << endl;
	while (1)
	{
		udp.recieve();
		request_params* incoming_request = new request_params();
		//recieved data
		cout << endl << "The Server B has recieved data for calculation:" << endl;
		incoming_request->print_input();
		cout << "The Server B has finished the calculation of the delays: " << endl;
		incoming_request->print_output();
		if (udp.send("aws", to_cstring(incoming_request->get_output()), incoming_request->get_output().size())) {} //whoops

		cout << endl << "The Server B has finished sending the output to AWS" << endl;
	}
}

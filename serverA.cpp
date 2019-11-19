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
#include <tuple>
#include <cmath>
#define MYPORT "21095"
#define AWS "23095"   // the port users will be connecting to
#define MAXBUFLEN 100
using namespace std;
#define RELINF 1000000
#define MYIPADDRESS "127.0.0.1"

//the following is from Beej guid
//************
// get sockaddr, IPv4 or IPv6:
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

void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
//****************

struct Request
{
	string mapID;
	string src;
	int file_size;
};


int udp_send(char* message) //please dont forget terminating char
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

	if ((rv = getaddrinfo(MYIPADDRESS, AWS, &hints, &servinfo)) != 0)
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

int udp_listen(Request* incoming_request, bool boot_up)
{
	//from beej
	//********************************
	bool debug = 1;
	if (debug)
	{
		vector<string> delimited = delimit(from_cstring("A 2 2000"), ' ', 3);
		incoming_request->mapID = delimited[0];
		incoming_request->src = delimited[1];
		incoming_request->file_size = string_to_int(delimited[2]);
		return 0;
	}
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

		if ((::bind(sockfd, p->ai_addr, p->ai_addrlen) == -1))
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


	if (boot_up)
	{
		printf("The Server A is up and running using UDP on port %s.", MYPORT);
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
	vector<string> delimited = delimit(from_cstring(buf), ' ', 3);
	incoming_request->mapID = delimited[0];
	incoming_request->src = delimited[1];
	incoming_request->file_size = string_to_int(delimited[2]);



	return 0;

}
class map_info
{
public:
	map<string, int> aliases;
	void dijkstra(int** graph, int* output, int numVert, int src)
	{
		//initialize
		bool* inPath = new bool[numVert];
		int min_v, index_v;
		for ( int i = 0; i < numVert; i++)
		{
			output[i] = RELINF;
			inPath[i] = 0;
		}
		output[src] = 0;
		inPath[src] = 0; //distance from source to self is 0

		for ( int i = 0; i < (numVert); i++)
		{
			min_v = RELINF;
			index_v = 0;
			for (int j = 0; j < (numVert); j++)
			{
				if ((output[j] <= min_v) && !(inPath[j]) && (output[j] != RELINF))
				{
					min_v = output[j];
					index_v = j;

				}

			}
			inPath[index_v] = 1;
			if (output[index_v] >= RELINF) continue; //try w and w/o this


			for (int k = 0; k < numVert; k++)
			{
				if ((!inPath[k]) && (graph[index_v][k] > 0))
				{
					if (output[k] > (graph[index_v][k] + output[index_v]))
					{
						output[k] = graph[index_v][k] + output[index_v];
					}
				}
			}

		}

		delete [] inPath;
		return;
	}
	vector<int> get_dijkstra(string src)
	{
		int src_prime = aliases[src];
		vector<int> output(num_v);
		int* shortest_paths = new int[num_v]; //allocate
		this->dijkstra(graph, shortest_paths, num_v, src_prime);
		for (int i = 0; i < num_v; i++) output[i] = shortest_paths[i];
		delete [] shortest_paths;
		return output;
	}
	string get_mapID() {return this->mapID;}
	int get_v() {return this->num_v;}
	int get_e() {return this->num_e;}
	double get_prop_speed() {return this->prop_speed;}
	double get_trans_speed() {return this->trans_speed;}


	map_info(vector<string> buffer, string mapID): buffer(buffer), mapID(mapID)
	{
		this->num_v = 0;
		//this->mapID = buffer[0][0];
		this->prop_speed = (double) stoi(buffer[1]);
		this->trans_speed = (double) stoi(buffer[2]);
		this->num_e = buffer.size() - 3;

		//test
		string to, from;
		int cost;
		//vector<vector<string> > nodes;
		string dummy;
		int running = 0;
		vector<vector<string>> nodes; //to from cost
		for (auto line : buffer)
		{

			nodes.push_back(delimit(line, ' ', 3));
			to = nodes.back()[0];
			from = nodes.back()[1];
			if (aliases.find(to) == aliases.end())
			{
				aliases.insert({to, running});
				running++;
			}
			if (aliases.find(from) == aliases.end())
			{
				aliases.insert({from, running});
				running++;
			}
		}
		this->num_v = running + 1;
		graph = new int* [num_v];
		for (int i = 0; i < num_v; i++)
		{
			graph[i] = new int[num_v];
			for (int j = 0; j < num_v; j++) graph[i][j] = 0;
		}

		for (auto node : nodes)
		{
			to = node[0];
			from = node[1];
			cost = string_to_int(node[2]);
			graph[aliases[to]][aliases[from]] = cost;
		}



		/*
		vector<string> dum;

		for (int i = 0; i < nodes.size(); i++)
		{
			to = nodes[i][0];
			from = nodes[i][1];
			cost = string_to_int(nodes[i][2]);
		}
		*/
		for (int i = 0; i < num_v; i++)
		{
			for (int j = 0; j < num_v; j++) cout << " " << graph[i][j];
			cout << endl;
		}
	}



private:
	int num_v;
	int** graph;
	double prop_speed;
	double trans_speed;
	string mapID;
	vector<string> buffer;
	int num_e;

};

vector<map_info*> read_file(string file_name)
{
	ifstream mapFile;
	mapFile.open(file_name);
	if (!mapFile) cout << endl << "FILE DNE" << endl;
	vector<string> buffer;
	string line;
	while (getline(mapFile, line))
	{
		buffer.push_back(line);
	}

	//isalpha
	//vector<string> map_dump;
	vector<vector<string> > map_dumps;
	vector<string>::const_iterator first = buffer.begin();
	vector<string>::const_iterator last = buffer.begin();
	int begin_value = 0;
	vector<map_info*> maps;
	for (int i = 0; i < buffer.size(); i++)
	{
		//check if new map
		if (((isalpha(buffer[i][0]) && (buffer[i].size() == 1))
		        || ( i == (buffer.size() - 1) ) ) && begin_value != i)
		{
			//new map
			first = buffer.begin() + begin_value + 3; //see if this doesntt cause problems
			last = buffer.begin() + i;
			map_dumps.push_back(vector<string>(first, last));
			//get map ID

			maps.push_back(new map_info(map_dumps[map_dumps.size() - 1], buffer[begin_value]));
			begin_value = i;
		}
	}
	//int num_v = buffer.size() -2;
	mapFile.close();
	return maps;
}


int main()
{

	//udp_send("test");
	vector<char*> Responses;
	vector<map_info*> maps = read_file("map.txt");
	//maps[0]->get_dijkstra(0);
	map<string, map_info*> map_of_maps;
	vector<vector<int> > shortest_paths;

	//construct map
	printf("\nThe Server A has constructed a list of %lu maps:\n", maps.size());
	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl << "Map ID Num Vertices Num Edges" << endl;
	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl;
	for (vector<map_info*>::iterator it = maps.begin(); it != maps.end(); ++it)
	{

		map_of_maps[(*it)->get_mapID()] = (*it);
		cout << setw(strlen("Map ID")) << (*it)->get_mapID();
		cout << setw(strlen("Num Vertices")) << (*it)->get_v();
		cout << setw(strlen("Num Edges")) << (*it)->get_e();
		cout << endl;
	}
	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl;

	Request* incoming_request = new Request;
	//while loop here
	string response;
	response = "";
	//listen

	if (udp_listen(incoming_request, true))
	{
		//whoops we got an error
	}





	string message = "The Server A has recieved input for finding the shortest paths: starting vertext %c of map %c.\n";
	//printf(message, incoming_request->src, incoming_request->mapID);
	printf(to_cstring(message), to_cstring(incoming_request->src), to_cstring(incoming_request->mapID));
	cout << incoming_request->src << " " << incoming_request->mapID;
	//Populate first values of response
	response += to_string(incoming_request->file_size);
	response += "||";
	response += map_of_maps[incoming_request->mapID]->get_mapID();
	response += "||";
	response += to_string(map_of_maps[incoming_request->mapID]->get_v());
	response += "||";
	response += to_string(map_of_maps[incoming_request->mapID]->get_e());
	response += "||";
	response += to_string(map_of_maps[incoming_request->mapID]->get_prop_speed());
	response += "||";
	response += to_string(map_of_maps[incoming_request->mapID]->get_trans_speed());
	response += "||";



	//print shortest path
	cout << "The Server A has identified the following shortest paths:" << endl;
	cout << endl;
	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl;
	cout << "Destination Min Length" << endl;
	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl;
	shortest_paths.push_back(map_of_maps[incoming_request->mapID]->get_dijkstra(string(incoming_request->src)));
	for (std::pair<vector<int>::iterator, map<string, int>::iterator>
	        it(shortest_paths.back().begin(),
	           map_of_maps[incoming_request->mapID]->aliases.begin());
	        it.first != shortest_paths.back().end();
	        ++it.first, ++it.second)
	{
		cout << (it.second->first) << " " << (*it.first) << endl;
		response += string(it.second->first);
		response += "||";
		response += to_string(*it.first);
		response += "||";
	}


	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl;

	//send
	Responses.push_back(new char[response.size()]);
	for (int i = 0; i < response.size(); i++)
	{
		Responses[Responses.size() - 1][i] = response[i];
	}

	//wait to send
	for (int i = 0; i < 100000000; i++) {}
	udp_send(Responses.back());

	delete [] Responses.back();





	return 0;
}

//TODO garbage collection
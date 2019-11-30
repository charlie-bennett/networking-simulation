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
#include <sstream>
#define MYPORT "21095"
#define AWS "23095"   // the port users will be connecting to
#define MAXBUFLEN 1000
using namespace std;
#define RELINF 1000000
#define MYIPADDRESS "127.0.0.1"
#define PRECISION 5
char buf[MAXBUFLEN];


//TODO: EXCEPTION HANDLING

//the following is from Beej guid
//************
// get sockaddr, IPv4 or IPv6:
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
string double_to_string(double input)
{
	ostringstream stream;
	stream << std::fixed;
	stream << setprecision(PRECISION);
	stream << input;
	return stream.str();

}
int string_to_int(string me)
{
	char* output = new char[me.size()];
	for ( int i = 0; i < me.size(); i++)
	{
		output[i] =  me[i];
	}
	return atoi(output);
}

struct src_comparator //organize sources in order
{
	bool operator()(const string& rhs, const string& lhs) const
	{
		return stod(rhs) < stod(lhs);
	}
};
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


void copy_to_cstring(string input, char* copy)
{
	for (int i = 0; i < input.size(); i++)
	{
		copy[i] = input[i];
	}
	return;
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

class Request
{
public:
	string mapID;
	string src;
	string file_size;
	Request()
	{
		vector<string> delimited = delimit(from_cstring(buf), ' ', 3);
		mapID = delimited[0];
		src = delimited[1];
		cout << from_cstring(buf) << endl;
		file_size = delimited[2];
	}
};

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
		cout << to_cstring(MYPORT) << endl;
		cout << to_cstring(MYIPADDRESS) << endl;

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
class map_info
{
public:
	map<string, int, src_comparator> aliases;
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



	map_info(vector<string> buffer, string mapID, string prop_speed, string trans_speed): buffer(buffer), mapID(mapID),
		prop_speed(prop_speed), trans_speed(trans_speed)
	{
		this->num_v = 0;
		//this->mapID = buffer[0][0];

		this->num_e = buffer.size();
		string to, from;
		int cost;
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
		this->num_v = running;
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
			graph[aliases[from]][aliases[to]] = cost;
		}
		for (int i = 0; i < num_v; i++)
		{
			for (int j = 0; j < num_v; j++) cout << " " << graph[i][j];
			cout << endl;
		}
	}
	string get_response(Request* incoming_request)
	{
		string response = "";
		response += incoming_request->file_size;
		response += " ";
		response += mapID;
		response += " ";
		response += to_string(num_v);
		response += " ";
		response += to_string(num_e);
		response += " ";
		response += prop_speed;
		response += " ";
		response += trans_speed;
		response += " ";
		cout << "this is a ttest " << response << endl;
		//print shortest path
		cout << "The Server A has identified the following shortest paths:" << endl;
		cout << endl;
		for (int i = 0; i < 45; i++) cout << "-";
		cout << endl;
		cout << "Destination    Min Length  " << endl;
		for (int i = 0; i < 45; i++) cout << "-";
		cout << endl;
		vector<int> shortest_path_list = get_dijkstra(incoming_request->src);
		/*
		for (std::pair<vector<int>::iterator, map<string, int>::iterator>
		        it(shortest_path_list.begin(),
		           aliases.begin());
		        it.first != shortest_path_list.end();
		        ++it.first, ++it.second)
		{
			cout << (it.second->first) << " " << (*it.first) << endl;
			response += string(it.second->first);
			response += ' ';
			response += to_string(*it.first);
			response += ' ';
		}
		*/
		vector<string> for_grid;
		vector<int> widths;
		widths.push_back(string("Destination    ").size());
		widths.push_back(string("Min Length  ").size());



		for (auto src_name : aliases)
		{
			for_grid.push_back(src_name.first);
			for_grid.push_back(to_string(shortest_path_list[src_name.second]));
			//cout << src_name.first << " " << shortest_path_list[src_name.second] << endl;
			response += string(src_name.first);
			response += ' ';
			response += to_string(shortest_path_list[src_name.second]);
			response += ' ';

		}
		response += '\0';
		print_matrix(for_grid, widths);
		for (int i = 0; i < 45; i++) cout << "-";
		cout << endl;
		return response;
	}



private:
	int num_v;
	int** graph;
	string prop_speed;
	string trans_speed;
	string mapID;
	vector<string> buffer;
	int num_e;
	string response;

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

	vector<vector<string> > map_dumps;
	vector<string>::const_iterator first = buffer.begin();
	vector<string>::const_iterator last = buffer.begin();
	int begin_value = 0;
	vector<map_info*> maps;
	bool first_map = 0;
	for (int i = 0; i < buffer.size(); i++)
	{
		if ((isalpha(buffer[i][0])) && buffer[i].size() == 1 && first_map == 0)
		{
			first_map = 1;
			begin_value = i;
		}
		//check if new map
		if (((isalpha(buffer[i][0]) && (buffer[i].size() == 1))
		        || ( i == (buffer.size() - 1) ) ) && begin_value != i && first_map)
		{
			//new map
			first = buffer.begin() + begin_value + 3; //see if this doesntt cause problems
			last = buffer.begin() + i;
			map_dumps.push_back(vector<string>(first, last));
			//get map ID
			maps.push_back(new map_info(map_dumps[map_dumps.size() - 1], buffer[begin_value],
			                            buffer[begin_value + 1], buffer[begin_value + 2]));

			begin_value = i;
		}
	}
	mapFile.close();
	return maps;
}

int main()
{
	map<string, map_info*> map_of_maps;

	vector<char*> Responses;
	string response = "";
	UDP udp;
	cout << "Server A is up and running using UDP on port " << MYPORT << endl;
	vector<map_info*> maps = read_file("map.txt");
	vector<vector<int> > shortest_paths;


	//construct map

	printf("\nThe Server A has constructed a list of %lu maps:\n", maps.size());
	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl << "Map ID    Num Vertices   Num Edges" << endl;
	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl;
	vector<string> set_print;
	for (vector<map_info*>::iterator it = maps.begin(); it != maps.end(); ++it)
	{

		map_of_maps[(*it)->get_mapID()] = (*it);
		/*
		cout << setw(strlen("Map ID")) << (*it)->get_mapID();
		cout << setw(strlen("Num Vertices")) << (*it)->get_v();
		cout << setw(strlen("Num Edges")) << (*it)->get_e();
		cout << endl;
		*/
		set_print.push_back((*it)->get_mapID());
		set_print.push_back(to_string((*it)->get_v()));
		set_print.push_back(to_string((*it)->get_e()));
	}
	vector<int> widths;
	widths.push_back(string("Map ID    ").size());
	widths.push_back(string("Num Vertices   ").size());
	widths.push_back(string("Num Edges").size());
	print_matrix(set_print, widths);

	for (int i = 0; i < 45; i++) cout << "-";
	cout << endl;
	//listens
	while (1)
	{
		udp.recieve();
		Request* incoming_request = new Request();

		string message = "The Server A has recieved input for finding the shortest paths: starting vertext %s of map %s.\n";
		printf(to_cstring(message), to_cstring(incoming_request->src), to_cstring(incoming_request->mapID));
		if (map_of_maps[incoming_request->mapID]->aliases.find(incoming_request->src) == map_of_maps[incoming_request->mapID]->aliases.end())
		{
			printf("\n Error: %s not a valid source \n", to_cstring(incoming_request->src));
		}
		cout << incoming_request->src << " " << incoming_request->mapID;
		cout << "Server A has recieved input for finding the shortest paths:" << endl;
		cout << "starting vertex " << incoming_request->src << " of map " << incoming_request->mapID << endl;
		response = map_of_maps[incoming_request->mapID]->get_response(incoming_request);
		Responses.push_back(new char[response.size()]);
		copy_to_cstring(response, Responses.back());
		//wait to send
		for (int i = 0; i < 100000000; i++) {}
		//udp_send(Responses.back());
		udp.send("aws", Responses.back(), response.size());
		cout << endl << "Server A has sent the shortest paths to AWS" << endl;

		delete [] Responses.back();
	}





	return 0;
}

//TODO garbage collection
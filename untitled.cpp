#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;

#define RELINF 1000000


class map_info
{
public:
	void dijkstra(int** graph, int* output, int numVert)
	{
		bool* inPath = new bool[numVert];
		output[0] = 0;
		inPath[0] = 0;
		int min_v, index_v;
		for ( int i = 1; i < numVert; i++)
		{
			output[i] = RELINF;
			inPath[i] = 0;
		}
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
				inPath[index_v] = 1;
			}
			if (output[index_v] >= RELINF) continue;


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
	map_info(vector<string> buffer): buffer(buffer)
	{

		this->mapID = buffer[0][0];
		this->prop_speed = (double) stoi(buffer[1]);
		this->trans_speed = (double) stoi(buffer[2]);
		num_e = buffer.size() - 3;


		//test
		int to, from, cost;
		vector<vector<int> > nodes;
		string dummy;
		for (vector<string>::iterator it = buffer.begin() + 3; it != buffer.end(); ++it)
		{
			// to from cost (0, 2, 4)
			dummy = *it;
			to = atoi(&dummy[0]);
			from = atoi(&dummy[2]);
			cost = atoi(&dummy[4]);
			num_v = (to > num_v) ? to : num_v;
			num_v = (from > num_v) ? from : num_v;
			nodes.push_back(vector<int>(3));
			nodes[nodes.size() - 1][0] = to;
			nodes[nodes.size() - 1][1] = from;
			nodes[nodes.size() - 1][2] = cost;

		}
		num_v++;
		graph = new int* [num_v];
		for (int i = 0; i < num_v; i++)
		{
			graph[i] = new int[num_v];
			for (int j = 0; j < num_v; j++) graph[i][j] = 0;
		}
		vector<int> dum;
		for (vector<vector<int> >::iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			dum = *it;

			to = dum[0];
			from = dum[1];
			cost = dum[2];
			graph[to][from] = cost;
			graph[from][to] = cost;

		}
		cout << endl << "num_v = " << num_v << endl;
		for (int i = 0; i < num_v; i++)
		{
			for (int j = 0; j < num_v; j++) cout << " " << graph[i][j];
			cout << endl;
		}
		shortest_paths = new int[num_v];
		this->dijkstra(graph, shortest_paths, num_v);
		cout << endl;
		for (int i = 0; i < num_v; i++) cout << " " << shortest_paths[i] << " ";
		cout << endl;



	}



private:
	int num_v = 0;
	int** graph;
	double prop_speed;
	double trans_speed;
	char mapID;
	vector<string> buffer;
	int* shortest_paths;
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
			first = buffer.begin() + begin_value;
			last = buffer.begin() + i;
			map_dumps.push_back(vector<string>(first, last));
			maps.push_back(new map_info(map_dumps[map_dumps.size() - 1]));
			begin_value = i;
		}


	}

	//int num_v = buffer.size() -2;



	mapFile.close();
	return maps;

}



int main()
{

	vector<map_info*> maps = read_file("map.txt");

	//Map construction



	return 0;
}
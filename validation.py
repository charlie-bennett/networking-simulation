import subprocess
import sys
import os
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
import threading
import time
import random
max_file_size = 10000


class Graph():
    def __init__(self, vertices):
        self.V = vertices
        self.graph = [[0 for column in range(vertices)]
                      for row in range(vertices)]

    # A utility function to find the vertex with
    # minimum distance value, from the set of vertices
    # not yet included in shortest path tree
    def minDistance(self, dist, sptSet):

        # Initilaize minimum distance for next node
        min = 10000000
        min_index = 0
        # Search not nearest vertex not in the
        # shortest path tree
        for v in range(self.V):
            if dist[v] < min and sptSet[v] == False:
                min = dist[v]
                min_index = v

        return min_index

    # Funtion that implements Dijkstra's single source
    # shortest path algorithm for a graph represented
    # using adjacency matrix representation
    def dijkstra(self, src):
        print(type(src), " is the type of src")
        dist = [10000000] * self.V
        dist[src] = 0
        sptSet = [False] * self.V

        for cout in range(self.V):

            # Pick the minimum distance vertex from
            # the set of vertices not yet processed.
            # u is always equal to src in first iteration
            u = self.minDistance(dist, sptSet)

            # Put the minimum distance vertex in the
            # shotest path tree
            sptSet[u] = True

            # Update dist value of the adjacent vertices
            # of the picked vertex only if the current
            # distance is greater than new distance and
            # the vertex in not in the shotest path tree
            for v in range(self.V):
                if ((self.graph[u][v] > 0) and (sptSet[v] == False)
                        and (dist[v] > dist[u]) + (self.graph[u][v])):
                    dist[v] = dist[u] + self.graph[u][v]

        return dist


class MAP(object):
    instances = []

    def __init__(self, name):
        self.name = name
        self.verticies = list()
        MAP.instances.append(self)
        self.random_src = -1
        self.prop_delay = 0
        self.trans_delay = 0
        self.graph = [[0 for column in range(len(self.verticies))]
                      for row in range(len(self.verticies))]
        self.storage = []

    def get_src(self):
        if (self.random_src == -1):
            random_src = random.sample(self.verticies, 1)[0]
            return random_src
        else:
            return random_src


sys.path.append(os.path.expanduser('~/Desktop/fall_2019/ee450/ee450_project'))

directory = ('~/Desktop/fall_2019/ee450/ee450_project')
directory = os.path.expanduser(directory)
fileobj = open(os.path.join(directory, "map.txt"), 'rb')
content = fileobj.readlines()

count = 0
temp = None
running = []
type(content[0])
start = 0
for line in content:
    line = line.rstrip()
    line = line.decode("utf-8")
    line = str(line)

    #new map entirely
    if (line.isalpha() and line.find("M") == -1):
        if (temp is not None):
            temp.verticies = list(set(running))
        running = []
        temp = MAP(line)
        start = 1
    elif (running == [] and count != 2 and start):
        if (count == 0):
            temp.prop_delay = float(line)
        elif (count == 1):
            temp.trans_delay = float(line)
        count += 1  #we dont care about these lines
    elif (start):
        count = 0
        for item in line.split(' ')[:2]:
            running.append(int(item))
        temp.storage.append(line.split(' '))
        tempor = [int(one) for one in line.split(' ')]
        temp.storage.append(tempor)

if (temp is not None):
    temp.verticies = list(set(running))

client_args = []
for entry in MAP.instances:
    temp = []
    temp.append("./client")
    temp.append(entry.name)
    temp.append(str(entry.get_src()))
    temp.append(str(int(random.random() * max_file_size)))
    client_args.append(temp[:])

execute = {
    'AWS': [['./aws']],
    'ServerA': [['./serverA']],
    'ServerB': [['./serverB']],
    'client': client_args
}
capture_output = ""


def execute_process(given_thread):

    if (given_thread.thread_name == 'client'):
        time.sleep(2)
        given_thread.my_process = subprocess.Popen("/bin/bash".encode("utf-8"),
                                                   stdin=subprocess.PIPE,
                                                   stdout=subprocess.PIPE,
                                                   shell=True,
                                                   stderr=subprocess.STDOUT)
        try:
            for arg in given_thread.args:
                argument = ""
                argument = " ".join(arg)
                argument += " \n"
                argument = argument.encode("utf-8")
                print(argument)
                given_thread.my_process.stdin.write(argument)
                time.sleep(2)
                print("Client done with first argument")
            try:
                given_thread.my_process.stdin.close()
                stdout = given_thread.my_process.stdout.read()
                given_thread.personal_log = stdout
                print(given_thread.my_process.stdout.read())
            except Exception as ex:
                print(ex)  #process already dead
                try:
                    stdout = given_thread.my_process.communicate()[0]
                except Exception as ex:
                    print(ex)
                    print("second try failed")
                    given_thread.my_process.kill()
            given_thread.all_created.set()
            print("all threads have been created")

        except Exception as ex:
            print("******")
            print("Client Communicate error")
            print(ex)
            given_thread.my_process.kill()
            given_thread.all_created.set()
            #stdout = given_thread.my_process.communicate()[0]
    elif given_thread.thread_name == "AWS":
        given_thread.my_process = subprocess.Popen(given_thread.args[0],
                                                   shell=False)
        print("AWS was called")
        new_Thread.all_created.wait(timeout=7)
        time.sleep(2)
        print("Synch with thread {}".format(given_thread.thread_name))
        brute_process = subprocess.Popen("/bin/bash".encode("utf-8"),
                                         stdin=subprocess.PIPE,
                                         shell=True)
        brute_process.stdin.write("pkill -f './aws'".encode("utf-8"))
        brute_process.stdin.close()
        return
    else:
        given_thread.my_process = subprocess.Popen(given_thread.args[0],
                                                   stdout=subprocess.PIPE,
                                                   shell=True,
                                                   stderr=subprocess.STDOUT)

        print(" called arg: {}".format(given_thread.args[0]))
        print("std out about to be printed")
        try:
            new_Thread.all_created.wait(timeout=7)
            print("Synch with thread {}".format(given_thread.thread_name))
            try:
                given_thread.my_process.kill()
                stdout = given_thread.my_process.communicate(timeout=2)[0]
            except subprocess.TimeoutExpired:
                print(subprocess.TimeoutExpired)
                print("exception from {}".format(given_thread.thread_name))
                given_thread.my_process.kill()
        except Exception as ex:
            print("*************************")
            print(ex)
            print(given_thread.thread_name)
            given_thread.my_process.terminate()
    try:
        stdout = stdout.decode('utf-8')
        print("\n From {} ".format(given_thread.thread_name))
        print(stdout)
        given_thread.personal_log = stdout
        given_thread.add_to_log(stdout)
    except Exception as ex:
        print(ex)
        print(given_thread.thread_name)
        stdout = given_thread.my_process.stdout.read()
        print(stdout)
        given_thread.my_process.terminate()
    if (given_thread.thread_name == 'client'):
        for thread in given_thread.threads:
            if (thread.thread_name != 'client'):
                thread.my_process.terminate()


class new_Thread(threading.Thread):
    threads = []
    log = ""
    all_created = threading.Event()
    response_ready = threading.Event()
    lock = threading.Lock()
    ready = threading.Lock()

    def __init__(self, thread_name, args, thread_id):
        threading.Thread.__init__(self)
        self.thread_id = thread_id
        self.args = args
        self.thread_name = thread_name
        self.my_process = None
        self.personal_log = ""
        new_Thread.threads.append(self)

    def run(self):
        print("Starting {}".format(self.thread_name))
        execute_process(self)
        print("End {}".format(self.thread_name))

    def add_to_log(self, to_add):
        new_Thread.lock.acquire(True)
        try:
            new_Thread.log = new_Thread.log + "\n From {} \n".format(
                self.thread_name)
            new_Thread.log = new_Thread.log + to_add
        finally:
            new_Thread.lock.release()


client_outputs = []

count = 0
for name, args in zip(execute.keys(), execute.values()):
    temp = new_Thread(name, args, count)
    count += 1
for thread in new_Thread.threads:
    thread.start()
print("********************")
time.sleep(.1)
new_Thread.response_ready.set()
new_Thread.all_created.wait(timeout=2)
for thread in new_Thread.threads:
    thread.join()
time.sleep(3)
tempor = []
phrase = "client: received '"
for thread in new_Thread.threads:
    if (thread.thread_name == "client"):
        program_response = thread.personal_log
        program_input = thread.args
# read
output = []
phrase = "client: received '"
for line in program_response.split("\n"):
    if (line.find(phrase) != -1):
        output.append(line[line.find(phrase) + len(phrase):])

# Driver program
graphs = []
distances_comp = []
output_comp = []

for entry in MAP.instances:
    g = Graph(len(entry.verticies))
    graphs.append(g)
    for line in entry.storage:
        g.graph[(entry.verticies.index(int(line[0])))][entry.verticies.index(
            int(line[1]))] = float(line[2])
        g.graph[entry.verticies.index(int(line[1]))][entry.verticies.index(
            int(line[0]))] = float(line[2])
    distances_comp.append(g.dijkstra(entry.verticies.index(entry.get_src())))
count = 0
for map in MAP.instances:
    output_comp.append([])
    response = distances_comp[count]
    for entry in response:
        file_size = program_input[count][-1]
        trans_delay = float(file_size) / (float(map.trans_delay) * 8)
        prop_delay = entry / float(map.prop_delay)
        output_comp[count].append(prop_delay + trans_delay)
    count = count + 1
print(output_comp)
print(output)

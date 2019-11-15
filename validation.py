import subprocess
import sys
import os
from subprocess import Popen, PIPE
import matplotlib.pyplot as plt
import threading
import time
import random
max_file_size = 10000

sys.path.append(os.path.expanduser('~/Desktop/fall_2019/ee450/ee450_project'))

directory = ('~/Desktop/fall_2019/ee450/ee450_project')
directory = os.path.expanduser(directory)
fileobj = open(os.path.join(directory, "map.txt"), 'rb')
content = fileobj.readlines()


class MAP(object):
    instances = []

    def __init__(self, name):
        self.name = name
        self.verticies = set()
        MAP.instances.append(self)


count = 0
temp = None
running = []
type(content[0])
for line in content:
    line = line.rstrip()
    line = line.decode("utf-8")
    line = str(line)
    #new map entirely
    if (line.isalpha()):
        if (temp is not None):
            temp.verticies = set(running)
        running = []
        temp = MAP(line)
    elif (running == [] and count != 2):
        count += 1  #we dont care about these lines
    else:
        count = 0
        for item in line.split(' ')[:2]:
            running.append(item)
if (temp is not None):
    temp.verticies = set(running)

client_args = []
for entry in MAP.instances:
    for source in entry.verticies:
        temp = []
        temp.append("./client")
        temp.append(entry.name)
        temp.append(source)
        temp.append(str(int(random.random() * max_file_size)))
        client_args.append(temp[:])

execute = {
    'AWS': [['./aws']],
    'ServerA': [['./A']],
    'ServerB': [['./B']],
    'client': client_args
}


def execute_process(given_thread):
    for arg in given_thread.args:
        if (given_thread.thread_name == 'client'):
            time.sleep(5)
        given_thread.my_process = subprocess.Popen(arg,
                                                   stdout=subprocess.PIPE,
                                                   shell=True,
                                                   stderr=subprocess.STDOUT)
        print(" called arg: {}".format(arg))
        print("std out about to be printed")
    time.sleep(0.00001)  #yield to main

    try:
        if (given_thread.thread_id == len(new_Thread.threads) - 1):  #client
            try:
                stdout = given_thread.my_process.communicate(timeout=10)[0]

            except subprocess.TimeoutExpired:
                given_thread.my_process.kill()
                stdout = given_thread.my_process.communicate()[0]
            new_Thread.all_created.set()
            print("unlocked")
        else:
            new_Thread.all_created.wait()
            print("Synch with thread {}".format(given_thread.thread_name))
            try:
                stdout = given_thread.my_process.communicate(timeout=2)[0]
            except subprocess.TimeoutExpired:
                given_thread.my_process.kill()
                stdout = given_thread.my_process.communicate()[0]
    except Exception as ex:
        print("*************************")
        print(ex)
        given_thread.my_process.terminate()
    stdout = stdout.decode('utf-8')
    print("\n From {} ".format(given_thread.thread_name))
    print(stdout)
    given_thread.add_to_log(stdout)


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


count = 0
for name, args in zip(execute.keys(), execute.values()):
    temp = new_Thread(name, args, count)
    count += 1
for thread in new_Thread.threads:
    thread.start()
print("********************")
new_Thread.response_ready.set()
print("HELLO THERE", new_Thread.log)
f = open("validation_results.txt", "w+")
f.write(new_Thread.log)
f.close()

import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json
import time
import threading

class myThread (threading.Thread):
	def __init__(self, line, setup):
		threading.Thread.__init__(self)
		self.line = line
		self.setup = setup

	def run(self):
		ret = os.system("ssh -l LockeZ {} '{}' ".format(self.line, self.setup))
		while ret != 0:
			time.sleep(1)
			ret = os.system("ssh -l LockeZ {} '{}' ".format(self.line, self.setup))

if __name__ == "__main__":
	exp_name = sys.argv[1]
	threads = []
	ifconfig = open("ifconfig.txt")
	node_type = -1
	i = 0
	for line in ifconfig:
		if line[0] == '#':
			continue
		if line[0] == '=':
			if line[1] == 's':
				node_type = 1
			if line[1] == 'l':
				node_type = 2
		else:
			line = line.strip('\n')
			if node_type == 1:
				server_1 = "cd Sundial; git fetch; git reset --hard origin/1pc-log; cd experiments; ./{}.sh {}_new_{} false".format(exp_name, exp_name, str(i))
				thread1 = myThread(line, server_1)
			if node_type == 2:
				storage_1 = "cd ssd/Sundial; sudo git fetch; sudo git reset --hard origin/1pc-log; cd experiments; sudo ./{}.sh {}{} true".format(exp_name, exp_name, str(i))
				thread1 = myThread(line, storage_1)
			thread1.start()
			threads.append(thread1)
			i += 1
	for t in threads:
		t.join()
	ifconfig.close()
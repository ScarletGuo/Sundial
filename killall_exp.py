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
		ret = os.system("ssh -l LockeZ {} '{}'".format(self.line, self.setup))
		# while ret != 0:
		# 	time.sleep(1)
		# 	ret = os.system("ssh -l LockeZ {} '{}'".format(self.line, self.setup))

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
			server_1 = "sudo pkill rundb"
			thread1 = myThread(line, server_1)
			thread1.start()
			threads.append(thread1)
			i += 1
	for t in threads:
		t.join()
	ifconfig.close()
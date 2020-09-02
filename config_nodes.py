import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json
import time
import _thread

server_setup = "git clone https://github.com/ScarletGuo/Sundial.git; cd Sundial; git checkout 1pc-log; ./conf"
storage_setup = "git clone https://github.com/ScarletGuo/Sundial.git; cd Sundial; git checkout 1pc-log; cp ./storage_setup.sh ../; cd ..; sudo ./storage_setup.sh"


def run(line, setup):
	ret = os.system("ssh -l LockeZ {} '{}' ".format(line, setup))
	while ret != 0:
		ret = os.system("ssh -l LockeZ {} '{}' ".format(line, setup))

if __name__ == "__main__":
	ifconfig = open("ifconfig.txt")
	node_type = -1
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
			# if node_type == 1:
			# 	ret = os.system("ssh -l LockeZ {} '{}' &".format(line, server_setup))
			# 	if ret != 0:
			# 		err_msg = "error setup server"
			# 		print("ERROR: " + err_msg)
			# if node_type == 2:
			# 	ret = os.system("ssh -l LockeZ {} '{}' &".format(line, storage_setup))
			# 	if ret != 0:
			# 		err_msg = "error setup storage"
			# 		print("ERROR: " + err_msg)
			if node_type == 1:
				_thread.start_new_thread(run, (line, server_setup, ))
			if node_type == 2:
				_thread.start_new_thread(run, (line, storage_setup, ))
			time.sleep(1)
	ifconfig.close()
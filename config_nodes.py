import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json
import time

server_setup = "git clone https://github.com/ScarletGuo/Sundial.git; cd Sundial; git checkout 1pc-log; ./conf"
storage_setup = "git clone https://github.com/ScarletGuo/Sundial.git; cd Sundial; git checkout 1pc-log; cp ./storage_setup.sh ../; cd ..; sudo ./storage_setup.sh"

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
			if node_type == 1:
				ret = os.system("ssh -l LockeZ {} '{}' &".format(line, server_setup))
				if ret != 0:
					err_msg = "error setup server"
					print("ERROR: " + err_msg)
			if node_type == 2:
				ret = os.system("ssh -l LockeZ {} '{}' &".format(line, storage_setup))
				if ret != 0:
					err_msg = "error setup storage"
					print("ERROR: " + err_msg)
			time.sleep(1)
	ifconfig.close()
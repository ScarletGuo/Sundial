import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json
import time

if __name__ == "__main__":
	exp_name = sys.argv[1]

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
				server_1 = "cd Sundial; git fetch; git reset --hard origin/1pc-log; cd experiments; ./{}.sh {}{} false".format(exp_name, exp_name, str(i))
				ret = os.system("ssh -l LockeZ {} '{}'".format(line, server_1))
				while ret != 0:
					time.sleep(1)
					ret = os.system("ssh -l LockeZ {} '{}'".format(line, server_1))
			if node_type == 2:
				storage_1 = "cd ssd/Sundial; sudo git fetch; sudo git reset --hard origin/1pc-log; cd experiments; sudo ./{}.sh {}{} true".format(exp_name, exp_name, str(i))
				ret = os.system("ssh -l LockeZ {} '{}'".format(line, storage_1))
				while ret != 0:
					time.sleep(1)
					ret = os.system("ssh -l LockeZ {} '{}'".format(line, storage_1))
			i += 1
	ifconfig.close()
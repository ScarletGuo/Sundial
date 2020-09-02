import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json

server_setup = "git clone https://github.com/ScarletGuo/Sundial.git; cd Sundial; git checkout 1pc-log; ./conf"
storage_setup = "sudo su; mkfs.ext3 /dev/sdc; cd /users/LockeZ; mkdir ssd; mount /dev/sdc /users/LockeZ/ssd; cd ssd; git clone https://github.com/ScarletGuo/Sundial.git; cd Sundial; git checkout 1pc-log; ./conf"

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
			if node_type == 1:
				ret = os.system("scp ./server_setup LockeZ@{}:/users/LockeZ".format(line))
				if ret != 0:
					err_msg = "error setup server"
					print("ERROR: " + err_msg)
				ret = os.system("ssh -l LockeZ {} './server_setup'".format(line))
				if ret != 0:
					err_msg = "error setup server"
					print("ERROR: " + err_msg)
			if node_type == 2:
				ret = os.system("scp ./storage_setup LockeZ@{}:/users/LockeZ".format(line))
				if ret != 0:
					err_msg = "error setup storage"
					print("ERROR: " + err_msg)
				ret = os.system("ssh -l LockeZ {} './storage_setup'".format(line))
				if ret != 0:
					err_msg = "error setup storage"
					print("ERROR: " + err_msg)
	ifconfig.close()
# run all nodes in ifconfig.
import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json

ifconfig = "ifconfig.txt"

def start_nodes(script, arg, curr_node):
    f = open(ifconfig)
    num_nodes = 0;
    for addr in f:
        if '#' in line:
            continue;
        if curr_node == num_nodes:
            continue;
        # start server
        os.system("sudo ssh {} 'pkill rundb'".format(addr))
	cmd = "python3 {} {} NODE_ID={}".format(script, arg, num_nodes)
        ret = os.system("sudo ssh {} 'cd /users/scarletg/Sundial/ ; export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH ; su scarletg; {}' &".format(addr, cmd))
        if ret != 0:
		err_msg = "error executing server"
		job['ERROR'] = err_msg
		print("ERROR: " + err_msg)
        else:
            print("[LOG] start node {}".format(num_nodes))
        num_nodes += 1;
        
    # start own server
    os.system("export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH")
    print("[LOG] start node {}".format(curr_node))
    os.system("python3 {} {} NODE_ID={}".format(script, arg, curr_node))

def kill_nodes(curr_node):
    f = open(ifconfig)
    num_nodes = 0;
    for addr in f:
        if '#' in line:
            continue;
        if curr_node == num_nodes:
            continue;
        os.system("sudo ssh {} 'pkill rundb'".format(addr))
        print("[LOG] kill node {}".format(num_nodes))
        num_nodes += 1; 

if __name__ == "__main__":
    arg = sys.argv[1]
    script = "test.py"
    start_node(script, arg)
        
    

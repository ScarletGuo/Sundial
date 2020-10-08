import os, sys, re, os.path
import subprocess, datetime, time, signal, json
from test_distrib import start_nodes, kill_nodes
from test import compile_and_run, parse_arg

script = "test_distrib.py"

if __name__ == "__main__":
    job = json.load(open(sys.argv[1]))
    exp_name = sys.argv[1].split('.')[0]
    if '/' in exp_name:
        exp_name = exp_name.split("/")[-1]
        curr_node = int(sys.argv[2])
    args = [""]
    for key in job:
        if isinstance(job[key], list):
            new_args = []
            for i, x in enumerate(job[key]):
                for arg in args:
                    arg = arg + "{}={} ".format(key, x)
                    new_args.append(arg)
            args = new_args
        else:
            for arg in args:
                arg = arg + "{}={} ".format(key, job[key])
    #if os.path.exists("outputs/stats.json"):
    #	os.remove("outputs/stats.json")
    for arg in args:
        if script == "test_distrib.py":
            ret = start_nodes(script, arg, curr_node)
            if ret != 0:
                continue
            print("[LOG] KILLING REMOTE SERVER ... ")
            # kill the remote servers
            kill_nodes(curr_node)
            os.system("ssh node-1 'sudo pkill rundb'")
            print("[LOG] FINISH EXECUTION ")
        else:
            compile_and_run(parse_arg(arg))
    print("[LOG] FINISH WHOLE EXPERIMENTS")

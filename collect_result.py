import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json
import pandas as pd

if __name__ == "__main__":
	exp_name = sys.argv[1]

	ifconfig = open("ifconfig.txt")
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
				ret = os.system("scp LockeZ@{}:/users/LockeZ/Sundial/outputs/{}{}.csv ./".format(line, exp_name, str(i)))
				if ret != 0:
					time.sleep(1)
					os.system("scp LockeZ@{}:/users/LockeZ/Sundial/outputs/{}{}.csv ./".format(line, exp_name, str(i)))
				i += 1
	ifconfig.close()
	output_cnt = i
	df_list = []
	for i in range(0, output_cnt):
		df = pd.read_csv(exp_name + str(i)+'.csv') 
		df_list.append(df)
	df_result = df_list[0]
	sum_thruput = df_result["Throughput"]
	for i in range(1, output_cnt):
		sum_thruput += df_list[i]["Throughput"]
	df_result["sum_throughput"] = sum_thruput
	df_result.to_csv(exp_name+"_final_8.csv", index=False)
	os.system("mv ./*.csv ./outputs")
import os, sys, re, os.path
import platform
import subprocess, datetime, time, signal, json


dbms_cfg = ["config.h", "config-std.h"]

def replace(filename, pattern, replacement):
	f = open(filename)
	s = f.read()
	f.close()
	s = re.sub(pattern,replacement,s)
	f = open(filename,'w')
	f.write(s)
	f.close()


def try_compile(job):
	os.system("cp {} {}".format(dbms_cfg[1], dbms_cfg[0]))
	# define workload
	for (param, value) in job.items():
		pattern = r"\#define\s*" + re.escape(param) + r'.*'
		if "ADDR" in param:
			replacement = "#define " + param + ' \"' + str(value) + '\"'
		else:
			replacement = "#define " + param + ' ' + str(value)
		replace(dbms_cfg[0], pattern, replacement)
	os.system("make clean > temp.out 2>&1")
	ret = os.system("make -j8 > temp.out 2>&1")
	if ret != 0:
		print("ERROR in compiling, output saved in temp.out")
		exit(0)
	else:
		os.system("rm -f temp.out")


def run(test = '', job=None):
	app_flags = ""
	if test == 'read_write':
		app_flags = "-Ar -t1"
	if test == 'conflict':
		app_flags = "-Ac -t4"
	if "UNSET_NUMA" in job and job['UNSET_NUMA']:
		os.system("numactl --all ./rundb %s | tee temp.out" % app_flags)
	else:
		os.system("./rundb %s | tee temp.out" % app_flags)
	

def compile_and_run(job) :
	try_compile(job)
	run('', job)

def parse_output(job):
	output = open("sample.out")
	phase = 0
	for line in output:
		if phase == 0:
			if "=Worker Thread=" in line:
				phase = 1
				continue
		elif phase == 1:
			if "=Input/Output Thread=" in line:
				phase = 2
				continue
			line = line.strip()
			if ":" in line:
				# for token in line.strip().split('[summary]')[-1].split(','):
				list = line.split(':')
				# list = re.split(r'\s+|:\s+', line)
				key = list[0].strip()
				list[1] = list[1].strip()
				val = re.split(r'\s+', list[1])[0]
				job[key] = val
				# break
	output.close()
	os.system("rm -f temp.out")
	return job

if __name__ == "__main__":
	job = {}
	# for item in sys.argv[1:]:
	# 	key = item.split("=")[0]
	# 	value = item.split("=")[1]
	# 	job[key] = value

	# compile_and_run(job)
	job = parse_output(job)
	stats = open("test.json", 'a+')
	stats.write(json.dumps(job)+"\n")
	stats.close()

	



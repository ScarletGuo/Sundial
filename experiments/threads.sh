cd ../
cp -r config-ycsb-std.h config.h

lognode="false"

if [ $# -eq 1 ]
then
fname="$1"
elif [ $# -eq 2 ]
then
fname="$1"
lognode="$2"
else
fname="threads_remote_compute"
fi

# algorithm
alg=WAIT_DIE
commit_alg=ONE_PC
perc_remote=0.1
# latch=LH_MCSLOCK
# [WW]
# ww_starv_free="false"
# [BAMBOO]
# dynamic="true"
# retire_on="true"
# cs_pf="true"
# opt_raw="true"
# max_waiter=0

# workload
wl="YCSB"
# req=16
# synthetic=true
zipf=0
# num_hs=1
# pos=SPECIFIED
# specified=0
# fixed=1
# fhs="WR"
# shs="WR"
read_ratio=0.5
# ordered="false"
# flip=0
# table_size="10000000"
# chain="false"

# other
threads=16
# profile="true"
# cnt=100000 
# penalty=50000

# figure 4: normalized throughput with optimal case, varying requests
for i in 0 1 2 3 4
do
# for alg in WOUND_WAIT BAMBOO
# do
for commit_alg in ONE_PC TWO_PC
do
for threads in 2 4 8 16 32
do
timeout 200 python test.py CC_ALG=${alg} COMMIT_ALG=${commit_alg} LOG_NODE=${lognode} NUM_SERVER_THREADS=${threads} PERC_REMOTE=${perc_remote} READ_PERC=${read_ratio} ZIPF_THETA=${zipf}
done
done
done

cd outputs/
python3 collect_stats.py
mv stats.csv ${fname}.csv
mv stats.json ${fname}.json
cd ..

python experiments/send_email.py ${fname}
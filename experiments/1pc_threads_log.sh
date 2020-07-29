cd ../
cp -r config-ycsb-synthetic-std.h config.h

fname="2pc_threads_compute"

# algorithm
alg=WAIT_DIE
commit_alg=ONE_PC
lognode="true"
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
# zipf=0
# num_hs=1
# pos=SPECIFIED
# specified=0
# fixed=1
# fhs="WR"
# shs="WR"
# read_ratio=1
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
# for i in 0 1 2 3 4
# do
# for alg in WOUND_WAIT BAMBOO
# do
# for specified in 0 0.25 0.5 0.75 1
# do
for threads in 2 4 8 16 32
do
timeout 200 python test.py CC_ALG=${alg} COMMIT_ALG=${commit_alg} LOG_NODE=${lognode} NUM_SERVER_THREADS=${threads}
done
# done
# done

cd outputs/
python3 collect_stats.py
mv stats.csv ${fname}.csv
mv stats.json ${fname}.json
cd ..

python experiments/send_email.py ${fname}

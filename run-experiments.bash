#!/bin/bash

# Runs a list of experiments to reproduce reported results.
# author: Daniel Nichols
# date: April 2022

# ================
# === SETTINGS ===
# ================
BUILD_EXEC="./bin/buildsa"
QUERY_EXEC="./bin/querysa"
SEQUENCES="inputs/sars-cov-2.fa inputs/ecoli.fa inputs/fungus.fa"
BUILD_SEQ_RESULTS_OUTPUT="results/buildsa-sequential.csv"
BUILD_PAR_RESULTS_OUTPUT="results/buildsa-parallel.csv"
QUERY_SEQ_RESULTS_OUTPUT="results/querysa-sequential.csv"
QUERY_PAR_RESULTS_OUTPUT="results/querysa-parallel.csv"
NUM_EXPERIMENT_REPITITIONS=5

# =============
# === TESTS ===
# =============
echo "Testing..."
make clean > /dev/null && make test USE_OPENMP=0 > /dev/null
if [ $? -ne 0 ]; then
    echo "Failed sequential tests!"
    exit 1
fi
make clean > /dev/null && make test USE_OPENMP=1 > /dev/null
if [ $? -ne 0 ]; then
    echo "Failed OpenMP tests!"
    exit 1
fi
#ecoli test
if [ -f inputs/ecoli.fa ]; then
    make clean > /dev/null && make USE_OPENMP=1 > /dev/null
    ${BUILD_EXEC} inputs/ecoli.fa tmp.sa > /dev/null
    ${QUERY_EXEC} tmp.sa inputs/query.fa naive naive-output.txt > /dev/null
    ${QUERY_EXEC} tmp.sa inputs/query.fa simpleaccel simpleaccel-output.txt > /dev/null
    rm tmp.sa

    sort -g -o naive-output.txt naive-output.txt    # match the expected output sorting
    sort -g -o simpleaccel-output.txt simpleaccel-output.txt

    diff naive-output.txt inputs/query_res.txt
    if [ $? -ne 0 ]; then 
        echo "Failed Naive query on ecoli data."
        exit 1
    fi

    diff simpleaccel-output.txt inputs/query_res.txt
    if [ $? -ne 0 ]; then
        echo "Failed SimpleAccelerant query on ecoli data."
        exit 1
    fi

    rm naive-output.txt simpleaccel-output.txt
fi

# ===================
# === EXPERIMENTS ===
# ===================
echo "Running experiments..."
mkdir -p results

# serial -- buildsa, querysa
make clean > /dev/null && make USE_OPENMP=0 > /dev/null
printf "seq_length,preftab,sa_build_time,preftab_build_time,file_size\n" > ${BUILD_SEQ_RESULTS_OUTPUT}
printf "seq_length,preftab,mode,num_queries,total_query_time,avg_query_time\n" > ${QUERY_SEQ_RESULTS_OUTPUT}
for sequence in ${SEQUENCES}; do
    for preftab_size in 0 1 2 4 8 16 32 64; do
        sa_output_file="junk-buildsa-output.sa"
        query_output_file="junk-querysa-output.txt"
        query_input_file="${sequence%.fa}-queries.fa"
        
        for repetition in {1..3}; do
            ${BUILD_EXEC} ${sequence} ${sa_output_file} --preftab ${preftab_size} >> ${BUILD_SEQ_RESULTS_OUTPUT}
            ${QUERY_EXEC} ${sa_output_file} ${query_input_file} naive + >> ${QUERY_SEQ_RESULTS_OUTPUT}
            ${QUERY_EXEC} ${sa_output_file} ${query_input_file} simpleaccel + >> ${QUERY_SEQ_RESULTS_OUTPUT}

            rm -f ${sa_output_file} ${query_output_file}
        done
    done
done

# parallel -- buildsa, querysa
make clean > /dev/null && make USE_OPENMP=1 > /dev/null
printf "seq_length,preftab,sa_build_time,preftab_build_time,file_size\n" > ${BUILD_PAR_RESULTS_OUTPUT}
printf "seq_length,preftab,mode,num_queries,total_query_time,avg_query_time\n" > ${QUERY_PAR_RESULTS_OUTPUT}
for sequence in ${SEQUENCES}; do
    for preftab_size in 0 1 2 4 8 16 32 64; do
        sa_output_file="junk-buildsa-output.sa"
        query_output_file="junk-querysa-output.txt"
        query_input_file="${sequence%.fa}-queries.fa"
        
        for repetition in {1..3}; do
            ${BUILD_EXEC} ${sequence} ${sa_output_file} --preftab ${preftab_size} >> ${BUILD_PAR_RESULTS_OUTPUT}
            ${QUERY_EXEC} ${sa_output_file} ${query_input_file} naive + >> ${QUERY_PAR_RESULTS_OUTPUT}
            ${QUERY_EXEC} ${sa_output_file} ${query_input_file} simpleaccel + >> ${QUERY_PAR_RESULTS_OUTPUT}

            rm -f ${sa_output_file} ${query_output_file}
        done
    done
done

# ====================
# === PLOT RESULTS ===
# ====================
echo "Plotting results..."
python3 generate-plots.py ${BUILD_SEQ_RESULTS_OUTPUT} ${BUILD_PAR_RESULTS_OUTPUT} ${QUERY_SEQ_RESULTS_OUTPUT} ${QUERY_PAR_RESULTS_OUTPUT}

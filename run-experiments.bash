#!/bin/bash

# Runs a list of experiments to reproduce reported results.
# author: Daniel Nichols
# date: April 2022

# === SETTINGS ===
BUILD_EXEC="./bin/buildsa"
QUERY_EXEC="./bin/querysa"
BUILD_INPUT_SEQ="inputs/ecoli.fa"
BUILD_SEQ_RESULTS_OUTPUT="results/buildsa-sequential.csv"
BUILD_PAR_RESULTS_OUTPUT="results/buildsa-parallel.csv"
NUM_EXPERIMENT_REPITITIONS=5

# === TESTS ===
# TODO

# === EXPERIMENTS ===
mkdir -p results

# buildsa -- serial
make clean && make USE_OPENMP=0
printf "seq_length,preftab,sa_build_time,preftab_build_time,file_size\n" > ${BUILD_SEQ_RESULTS_OUTPUT}
for preftab_size in 0 1 2 4 8 16 32 64; do
    output_file="junk-buildsa-output.sa"
    for repition in {1..5}; do
        ${BUILD_EXEC} ${BUILD_INPUT_SEQ} ${output_file} --preftab ${preftab_size} >> ${BUILD_SEQ_RESULTS_OUTPUT}
        rm ${output_file}
    done
done

# buildsa -- parallel
make clean && make USE_OPENMP=1
printf "seq_length,preftab,sa_build_time,preftab_build_time,file_size\n" > ${BUILD_PAR_RESULTS_OUTPUT}
for preftab_size in 0 1 2 4 8 16 32 64; do
    output_file="junk-buildsa-output.sa"
    for repition in {1..5}; do
        ${BUILD_EXEC} ${BUILD_INPUT_SEQ} ${output_file} --preftab ${preftab_size} >> ${BUILD_PAR_RESULTS_OUTPUT}
        rm ${output_file}
    done
done

# === PLOT RESULTS ===
python3 generate-plots.py ${BUILD_SEQ_RESULTS_OUTPUT} ${BUILD_PAR_RESULTS_OUTPUT} junk

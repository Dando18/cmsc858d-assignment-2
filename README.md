# CMSC 858D Assignment 2

This repo contains the source code for CMSC858D assignment 2.
It is written in C++ and can be built using `make`.
You can build the debug version with `make DEBUG=1`.
By default the code builds with OpenMP support.
To turn this off build with `make USE_OPENMP=0`.

For detailed description of the API for the suffix array you can run `make docs`.
Assuming you have doxygen and latex installed this will build documentation in `docs/`.

## Running the Code

**buildsa** can be run with 

```
# --preftab is optional
./bin/buildsa <input-fasta> <output-file> --preftab k
```

**querysa** can be run with 

```
./bin/querysa <suffix-array> <queries-fasta> <mode> <output>
```

To reproduce the reported plots you can run

```
bash run-experiments.bash
```

This assumes you have the 3 specified genomes in `inputs/`.
To substitute simply add FASTA files to `inputs/` and update the
`SEQUENCES=...` line in `run-experiments.bash`. 

## Project Layout

- `include/` contains the suffix array implementation alongside several utility files.
- `src/` includes the drivers for buildsa, querysa, and some tests.
- `inputs/` contains FASTA files for use in the scripts.


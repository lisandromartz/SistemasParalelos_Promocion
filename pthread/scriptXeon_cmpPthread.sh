#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=XeonPHI
#SBATCH -o Resultados/output.txt
#SBATCH -e Resultados/errors.txt
source $ONEAPI_PATH/setvars.sh > /dev/null 2>&1
icc -pthread -o cmpAB_pthread cmpAB_pthread.c
./cmpAB_pthread $1 $2
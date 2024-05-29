#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=XeonPHI
#SBATCH -o Resultados/output.txt
#SBATCH -e Resultados/errors.txt
source $ONEAPI_PATH/setvars.sh > /dev/null 2>&1
icc -pthread -o cmp_pthread cmp_pthread.c
./cmp_pthread $1 $2
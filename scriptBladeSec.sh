#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -o Resultados/outputSec.txt
#SBATCH -e Resultados/errorsSec.txt
./cmp_sec $1

#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH -o Resultados/output.txt
#SBATCH -e Resultados/errors.txt
./cmp_secBlade $1

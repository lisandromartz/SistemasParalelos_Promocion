#!/bin/bash
#SBATCH -N 2
#SBATCH --exclusive
#SBATCH --tasks-per-node=2
#SBATCH -o Resultados/output.txt
#SBATCH -e Resultados/errors.txt
mpirun cmp_mpi $1

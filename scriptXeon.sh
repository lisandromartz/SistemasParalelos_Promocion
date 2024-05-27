#!/bin/bash
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --partition=Blade
#SBATCH -o Resultados/output.txt
#SBATCH -e Resultados/errors.txt
./sort $1

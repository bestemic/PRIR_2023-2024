#!/bin/bash
#
#SBATCH -t 0-0:02 # time (D-HH:MM)
#SBATCH -N 1
#SBATCH -n 2
#SBATCH -o slurm.%N.%j.out # STDOUT
#SBATCH -e slurm.%N.%j.err # STDERR

mpiexec ./a.out

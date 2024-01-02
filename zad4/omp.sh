#!/bin/bash
#
#SBATCH -t 0-0:03 # time (D-HH:MM)
#SBATCH -p OMP    # partycja OMP
#SBATCH -c 8      # liczba rdzeni
#SBATCH -o slurm.%N.%j.out # STDOUT
#SBATCH -e slurm.%N.%j.err # STDERRs}n

if [ -n "$SLURM_CPUS_PER_TASK" ]; then
  omp_threads=${SLURM_CPUS_PER_TASK}
else
  omp_threads=1
fi

c++ -O2 -fopenmp DataSupplier.cpp Force.cpp main.cpp MyForce.cpp SimpleDataSupplier.cpp Simulation.cpp

export OMP_NUM_THREADS=${omp_threads}
echo "Value of OMP_NUM_THREADS: $OMP_NUM_THREADS"

/usr/bin/time ./a.out 2>&1
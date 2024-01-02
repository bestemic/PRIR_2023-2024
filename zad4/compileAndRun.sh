#!/bin/bash

c++ -O2 -fopenmp DataSupplier.cpp Force.cpp main.cpp MyForce.cpp SimpleDataSupplier.cpp Simulation.cpp 

export OMP_NUM_THREADS=3
echo "Value of OMP_NUM_THREADS: $OMP_NUM_THREADS"

/usr/bin/time ./a.out 2>&1

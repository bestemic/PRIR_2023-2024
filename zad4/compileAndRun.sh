#!/bin/bash

/opt/homebrew/bin/g++-13 -O2 -fopenmp DataSupplier.cpp Force.cpp main.cpp MyForce.cpp SimpleDataSupplier.cpp Simulation.cpp 

export OMP_NUM_THREADS=1

/usr/bin/time ./a.out 2>&1
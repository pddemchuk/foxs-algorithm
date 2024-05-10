#!/bin/bash

# $1 - размер матриц $2 - колво процессов для фокса

cc matrix_generator.c -o gen && ./gen $1

cc sequential_alg.c -o seq
./seq $1

echo ""

mpicc parallel_alg.c -o par -lm
mpiexec --oversubscribe -np $2 ./par $1

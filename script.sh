#!/bin/bash

# $1 - номер алг, $2 - размер матрицб $3 - колво процессов для фокса

cc matrix_generator.c -o gen && ./gen $2

cc sequential_alg.c -o seq
./seq $2

echo ""

mpicc parallel_alg.c -o par -lm
mpiexec --oversubscribe -np $3 ./par $2

# if (( $1 == 0 ))
# then
#     mpicc sequential_alg.c -o seq
#     mpiexec --oversubscribe -np 1 ./seq $2
# else
#     mpicc parallel_alg.c -o par -lm
#     mpiexec --oversubscribe -np $3 ./par $2
# fi

#for (( i = 3; i < 34; i = i + 3 ))
#do
#	mpiexec --oversubscribe -np $i main 
#done

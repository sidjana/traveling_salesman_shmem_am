#!/bin/bash


rm -rf *.log 

for numcities in 04 14 15 #16 17 20
do
  for nprocs in 2 4 8 16 32 64 128 256 512
  do
     for outfile in "tsp_shmem_LLPQ" # "tsp_shmem_PPQ" "tsp_shmem_MMPQ" "tsp_mpi"
     do
        srun -p crill --exclusive -n $nprocs ${outfile}.out ../support/tspfile${numcities}.txt > temp.log
        grep "#Time;" ./temp.log >> ${outfile}.log
     done
  done
done


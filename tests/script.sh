#!/bin/bash


rm -rf *.log 

for numcities in 04 14 20 #15 16 17 
do
  for nprocs in 2 4 8 16 32 64 128 256 512 1024 2048
  do
     for outfile in "tsp_shmem_LLPQ" "tsp_mpi" # "tsp_shmem_PPQ" "tsp_shmem_MMPQ" 
     do
	for count in `seq 1 10`
	do
	   echo "Running with $numcities cities, $nprocs PEs, $outfile version, ${count}th iteration" >> progress.log
       	   srun -p crill --exclusive -n $nprocs ${outfile}.out ../support/tspfile${numcities}.txt > temp.log
       	   grep "#Time;" ./temp.log >> ${outfile}.log
	done
     done
  done
done


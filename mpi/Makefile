

CFLAGS =  -O2
MPICPP = mpic++ 

.PHONY: clean run build all default



build all default: tsp.C list.C tsp.h list.h 
	make clean;
	$(MPICPP) $(CFLAGS) $^ ../support/rtc.o -o ./tsp_mpi.out;

run:
	srun -n 2 ./tsp*.out ../support/tspfile04.txt


clean:
	rm -rf ./tsp*.out

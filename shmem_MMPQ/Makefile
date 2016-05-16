

CFLAGS =  -O2
OSHCPP = oshc++ 

.PHONY: clean run build all default



build all default: list.cpp tsp.cpp tsp.h list.h master.cpp worker.cpp
	make clean;
	$(OSHCPP) $(CFLAGS) $^ ../support/rtc.o -o ./tsp_shmem_MMPQ.out;

run:
	srun -n 2 --exclusive ./tsp*.out ../support/tspfile04.txt


clean:
	rm -rf ./tsp*.out



CFLAGS = -O2 
OSHCPP = oshc++

.PHONY: run build all default



build all default: list.c tsp.c tsp.h list.h master.c worker.c
	$(OSHCPP) $(CFLAGS) $^ -o ./tsp.out

run:
	srun -n 2 ./tsp.out


clean:
	rm -rf ./tsp.out

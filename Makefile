

CFLAGS =  -g
OSHCPP = oshc++ 

.PHONY: clean run build all default



build all default: list.c tsp.c tsp.h list.h master.c worker.c
	make clean;
	$(OSHCPP) $(CFLAGS) $^ -o ./tsp.out;

run:
	srun -n 4 ./tsp.out


clean:
	rm -rf ./tsp.out

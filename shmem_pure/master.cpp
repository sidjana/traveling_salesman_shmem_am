#include "list.h"
#include "tsp.h"

extern "C" {
        void get_rtc_(unsigned long long int*);
        void get_rtc_res_(unsigned long long int*);
}

extern int* waiting;     // to save peids
extern int isnewpath, isdone, NumProcs, mype, NumCities;
extern int *best_path_list, *subscribe_list, *put_path_list;
extern Msg_t  *worker_new_path;
extern Msg_t msg_in, worker_shortest_path;

extern volatile int nwait;

unsigned long long int start_time, stop_time, res;

Path Shortest;
LIST queue;

inline void 
subscribe_pe(int req_pe)
{
	*(waiting+nwait) = req_pe;
	nwait++;
}

inline void 
announce_done()
{
        for (int i=1; i<NumProcs; i++)
	   shmem_int_swap(&isdone, 1, i);
}


static void
handler_master_subscribe()
{
	for(int i=1;i<NumProcs;i++)
	    if(shmem_int_swap((subscribe_list+i), 0, MASTER_PE)) 
		subscribe_pe(i);
}


static void 
handler_master_putpath()
{
	for(int j=1;j<NumProcs;j++) {
	    int pathcnt=0;
	    pathcnt = shmem_int_swap((put_path_list+j), 0, MASTER_PE);
	    if(pathcnt) {
	    	shmem_getmem(worker_new_path, worker_new_path, sizeof(Msg_t)*pathcnt, j);
	    	for(int i=0;i<pathcnt;i++) {
	    	   Path* P = new Path();
	    	   P->Set ( (worker_new_path+i)->length, 
	    	            (worker_new_path+i)->city, 
	    	    	(worker_new_path+i)->visited);
		   fflush(stdout);
	    	   queue.Insert(P, (worker_new_path+i)->length);
	    	}
                //printf("Master: Got new paths (cnt:%d), source = %d\n", pathcnt, j);
		subscribe_pe(j);
	    }
	}
}


static void 
handler_master_bestpath()
{
	static int bpath=0;
	int new_path_found = 0;
	for(int i=1;i<NumProcs;i++) {
	    if(shmem_int_swap((best_path_list+i), 0, MASTER_PE)) {
		shmem_getmem(&worker_shortest_path, &worker_shortest_path, sizeof(Msg_t), i);
	        if (worker_shortest_path.length < Shortest.length) {
                   bpath ++;
                   printf("Master: Got best path %d, source = %d, length = %d\n", 
                        bpath, i, worker_shortest_path.length);
                   fflush(stdout);
                   Shortest.Set (worker_shortest_path.length, worker_shortest_path.city, NumCities);
		   new_path_found = 1;
                }
		subscribe_pe(i);
	    }
	}
	if(new_path_found) 
            for(int i =1; i<NumProcs ; i++) // PE 0 is MASTER_PE 
	    	shmem_int_swap(best_path_list+MASTER_PE, Shortest.length,i); 
}	


inline int 
assign_task()
{
	int retval = 0;
	Path* P;
	Msg_t* msg_buf = new Msg_t;
	while(nwait>0) {
           if(!queue.IsEmpty()) {
              // get a path and send it along with bestlength
              P = (Path *)queue.Remove(NULL); 
              msg_buf->length = P->length;
              memcpy (msg_buf->city, P->city, MAXCITIES*sizeof(int));
              msg_buf->visited = P->visited;
              delete P;
	      --nwait;
	      int dest_pe = *(waiting+nwait);
	      shmem_putmem(&msg_in, msg_buf, sizeof(Msg_t), dest_pe);
	      shmem_quiet();
	      shmem_int_swap(&isnewpath,1,dest_pe);
	      shmem_quiet();
	      //printf("Master sending new path to %d\n",dest_pe);
           }else {
	      if(nwait == NumProcs-1) {
	        get_rtc_(&stop_time);
                announce_done();
                retval = 1;
	      }
	      break;
	   }
	} 

	return retval;
}


void Master ()
{
  // To keep track of processes that are waiting for a Path
  waiting = (int*) malloc(NumProcs*sizeof(int)); // worker stack
  nwait = 0;                 // 1 master and the rest are workers
  Path* P = new Path;
  queue.Insert(P, 0);	      // initialize queue with the first task
                              // one zero-length path
  Shortest.length = INTMAX;   // The initial Shortest path must be bad

  get_rtc_res_(&res);
  printf("Coord started ...\n"); fflush(stdout);
  shmem_barrier_all();
  get_rtc_(&start_time);
  while (1) 
  {
    handler_master_subscribe();
    if(assign_task())
       break;
    handler_master_bestpath();
    handler_master_putpath();
  }
  shmem_barrier_all();
  printf("Shortest path:\n");
  Shortest.Print();
  double time = (stop_time - start_time)*1.0/double(res);
  printf("\n#Time;%20.5f;NumProcs;%d;NumCities;%d\n",time,NumProcs,NumCities);
}



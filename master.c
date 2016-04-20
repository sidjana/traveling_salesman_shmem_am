#include "list.h"
#include "tsp.h"

extern int* waiting;     // to save peids
extern Msg_t msg_in;
extern shmemx_am_mutex lock_shortestlen, lock_queue, lock_workers_stack;
extern int newshortestlen, isnewpath, isdone, NumProcs, mype, isshortest, NumCities;
extern volatile int nwait;

Path Shortest;
LIST queue;


void 
handler_master_subscribe(void *temp, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
	shmemx_am_mutex_lock(&lock_workers_stack);
	*(waiting+nwait) = req_pe;
	nwait++;
	printf("Master: Worker %d added to work queue\n",req_pe);
	fflush(stdout);
	shmemx_am_mutex_unlock(&lock_workers_stack);
}


void 
handler_master_putpath(void *msg_new, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
	Msg_t* msg_buf = (Msg_t*) msg_new;
	Path* P = new Path();
	P->Set (msg_buf->length, msg_buf->city, msg_buf->visited);
	shmemx_am_mutex_lock(&lock_queue);
	queue.Insert(P, msg_buf->length);
	shmemx_am_mutex_unlock(&lock_queue);
	printf("Master: Received new path from Worker %d\n",req_pe);
	fflush(stdout);
}


void 
handler_master_bestpath(void *msg_new, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
	static int bpath=0;
	Msg_t* msg_buf = (Msg_t*)(msg_new);
	shmemx_am_mutex_lock(&lock_shortestlen);
	if (msg_buf->length < Shortest.length)
	{
           bpath ++;
           printf("Got best path %d, source = %d, length = %d\n", 
                bpath, req_pe, msg_buf->length);
           fflush(stdout);
           Shortest.Set (msg_buf->length, msg_buf->city, NumCities);
	   newshortestlen = msg_buf->length;
	   shmem_int_swap(&isshortest,1,mype);
        }
	shmemx_am_mutex_unlock(&lock_shortestlen);
}	


inline void 
update_shortest()
{
        for( int i = 1 ; i<NumProcs ; i++ ) 
           shmem_int_p(&newshortestlen, newshortestlen, i);
}


inline void 
announce_done()
{
        for (int i=1; i<NumProcs; i++)
	   shmem_int_p(&isdone, 1, i);
}


inline int 
assign_task()
{
	Path* P;
	Msg_t* msg_buf = new Msg_t;
	shmemx_am_mutex_lock(&lock_workers_stack);
	int temp_nwait = nwait;
	shmemx_am_mutex_unlock(&lock_workers_stack);
	if(temp_nwait>0) {
           shmemx_am_mutex_lock(&lock_queue);
           if(!queue.IsEmpty()) {
              // get a path and send it along with bestlength
              P = (Path *)queue.Remove(NULL); 
              shmemx_am_mutex_unlock(&lock_queue);
              msg_buf->length = P->length;
              memcpy (msg_buf->city, P->city, MAXCITIES*sizeof(int));
              msg_buf->visited = P->visited;
              delete P;
	      shmemx_am_mutex_lock(&lock_workers_stack);
	      --nwait;
	      int dest_pe = *(waiting+nwait);
	      shmemx_am_mutex_unlock(&lock_workers_stack);
	      shmem_putmem(&msg_in, msg_buf, sizeof(Msg_t), dest_pe);
	      shmem_quiet();
	      shmem_int_swap(&isnewpath,1,dest_pe);
	      shmem_quiet();
	      printf("Master: assigned new task to Worker %d\n",dest_pe);
	      fflush(stdout);
	      return 0;
           }else if(temp_nwait == NumProcs-1) {
              shmemx_am_mutex_unlock(&lock_queue);
              announce_done();
              return 1;
	   }
	}
}


void Master ()
{
  // To keep track of processes that are waiting for a Path
  waiting = (int*) malloc(NumProcs*sizeof(int)); // worker stack
  nwait = 0;          // 1 master and the rest are workers
  Path* P = new Path;
  queue.Insert(P, 0);	       // initialize queue with the first task
                               // one zero-length path
  Shortest.length = INTMAX;   // The initial Shortest path must be bad

  shmem_barrier_all();
  printf("Coord started ...\n"); fflush(stdout);
  while (1) 
  {
    shmemx_am_poll();
    if(shmem_int_swap(&isshortest,0,mype))  
       update_shortest();
    if(assign_task())
       break;
  }
  shmem_barrier_all();
  printf("Shortest path:\n");
  Shortest.Print();
}



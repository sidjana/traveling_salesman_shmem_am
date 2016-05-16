#include "list.h"
#include "tsp.h"

extern "C" {
        void get_rtc_(unsigned long long int*);
        void get_rtc_res_(unsigned long long int*);
}

extern int* waiting;     // to save peids
extern Msg_t msg_in;
extern shmemx_am_mutex lock_shortestlen, lock_queue, lock_workers_stack;
extern int newshortestlen, isnewpath, isdone, NumProcs, mype, isshortest, NumCities;
extern volatile int nwait;

unsigned long long int start_time, stop_time, res;

Path Shortest;
LIST queue;


inline void 
announce_done()
{
        for (int i=1; i<NumProcs; i++)
	   shmem_int_p(&isdone, 1, i);
}

inline void
update_shortest()
{
        for( int i = 1 ; i<NumProcs ; i++)  
	   shmem_int_swap(&newshortestlen,newshortestlen,i);
}


void 
handler_master_subscribe(void *temp, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
	printf("about to subscribe %d\n",req_pe);
	fflush(stdout);
	shmemx_am_mutex_lock(&lock_workers_stack);
	*(waiting+nwait) = req_pe;
	nwait++;
	printf("Subscribing %d\n",req_pe);
	shmemx_am_mutex_unlock(&lock_workers_stack);
}


void 
handler_master_putpath(void *inbuf, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
	Msg_t* msg_buf = (Msg_t*)inbuf;
	int pathcnt = nbytes/sizeof(Msg_t);

	shmemx_am_mutex_lock(&lock_queue);
	for(int i=0;i<pathcnt;i++) {
	   Path* P = new Path();
	   P->Set (msg_buf[i].length, msg_buf[i].city, msg_buf[i].visited);
	   queue.Insert(P, msg_buf[i].length);
	}
	//printf("Master received %d paths from %d\n",pathcnt,req_pe);
	handler_master_subscribe(NULL, 0, req_pe, NULL);
	shmemx_am_mutex_unlock(&lock_queue);
}


void 
handler_master_bestpath(void *msg_new, size_t nbytes, int req_pe, shmemx_am_token_t token)
{
	static int bpath=0;
	Msg_t* msg_buf = (Msg_t*)(msg_new);
	if (msg_buf->length < Shortest.length)
	{
           bpath ++;
           printf("Master: Got best path %d, source = %d, length = %d\n", 
                bpath, req_pe, msg_buf->length);
           fflush(stdout);
           Shortest.Set (msg_buf->length, msg_buf->city, NumCities);
	   shmem_int_swap(&newshortestlen,msg_buf->length,mype);
	   shmem_int_swap(&isshortest,1,mype);
	   shmem_quiet();
        }
	handler_master_subscribe(NULL, 0, req_pe, NULL);
}	


inline int 
assign_task()
{
	int retval = 0, i=-1;
	Path* P;
	Msg_t* msg_buf = new Msg_t;
	Msg_t* temp_msg_buf = (Msg_t*) malloc(NumCities*sizeof(Msg_t));
	int* temp_dest_pe = (int*) malloc(NumCities*sizeof(int));

        shmemx_am_mutex_lock(&lock_queue);	   // lock for queue
	shmemx_am_mutex_lock(&lock_workers_stack); // lock for nwait
        while(nwait>0) {
	   if(!queue.IsEmpty()) {
               // get a path and send it along with bestlength
               P = (Path *)queue.Remove(NULL); 
               msg_buf->length = P->length;
               memcpy (msg_buf->city, P->city, MAXCITIES*sizeof(int));
               msg_buf->visited = P->visited;
               delete P;
	       --nwait;
	       ++i;
	       memcpy((temp_msg_buf+i),msg_buf,sizeof(Msg_t));
	       memcpy((temp_dest_pe+i),(waiting+nwait),sizeof(int));
	   } else
	       break;
	}

	if(nwait==NumProcs-1) {
	       get_rtc_(&stop_time);
               retval = 1;
	}

	shmemx_am_mutex_unlock(&lock_workers_stack);
        shmemx_am_mutex_unlock(&lock_queue);

	while(i>-1) {
		printf("Master sending worker %d = (%d,%d)\n",*(temp_dest_pe+i),
				(temp_msg_buf+i)->length,(temp_msg_buf+i)->visited);
	         shmem_putmem(&msg_in, (temp_msg_buf+i), sizeof(Msg_t), *(temp_dest_pe+i));
	         shmem_quiet();
	         shmem_int_swap(&isnewpath,1,*(temp_dest_pe+i));
		 i--;
	}
	shmem_quiet();

	if(retval) {
          announce_done();
	}

	return retval;
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

  get_rtc_res_(&res);
  shmem_barrier_all();
  printf("Coord started ...\n"); fflush(stdout);
  shmem_barrier_all();
  get_rtc_(&start_time);
  while (1) 
  {
    shmemx_am_poll();
    if(assign_task())
       break;
    if(shmem_int_swap(&isshortest,0,mype))
	update_shortest();
  }
  shmem_barrier_all();
  printf("Shortest path:\n");
  Shortest.Print();
  double time = (stop_time - start_time)*1.0/double(res);
  printf("\n#Time;%20.5f;NumProcs;%d;NumCities;%d\n",time,NumProcs,NumCities);
}



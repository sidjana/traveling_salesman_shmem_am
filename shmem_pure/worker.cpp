#include <shmem.h>
#include "list.h"
#include "tsp.h"


extern int Dist[];
extern int newshortestlen, isnewpath, isdone, NumProcs, mype, isshortest, NumCities;
extern volatile int nwait;
extern int *best_path_list, *subscribe_list, *put_path_list;
extern Msg_t *worker_new_path;
extern Msg_t msg_in, worker_shortest_path;


void Worker ()
{ 
  *(best_path_list+MASTER_PE)=INTMAX;
  isdone = 0; isnewpath = 0;
  shmem_int_swap(subscribe_list+mype, 1,MASTER_PE);
  memset(&msg_in,0,sizeof(Msg_t));
  
  shmem_barrier_all();

  while (1) {

    newshortestlen = shmem_int_fadd((best_path_list+MASTER_PE),0,mype);
    if (shmem_int_swap(&isnewpath,0,mype)) { 
	int subscribed = 0;
	//printf("Worker %d just received new path: %d,%d\n",mype,msg_in.length,msg_in.visited);
	//fflush(stdout);

    	msg_in.visited++;

    	if (msg_in.visited==NumCities) {
	  //printf("Worker %d checking for short distance\n",mype);
	  //fflush(stdout);
    	  int d1 = Dist[(msg_in.city[NumCities-2])*NumCities + msg_in.city[NumCities-1]];
    	  int d2 = Dist[(msg_in.city[NumCities-1]) * NumCities ];

    	  if (d1 * d2) { 
    	     // both edges exist 
    	     msg_in.length += d1 + d2;
    	  
    	     // if path is good, send it to master
    	     if (msg_in.length < newshortestlen) { 
	        memcpy(&worker_shortest_path,&msg_in,sizeof(Msg_t));
	        shmem_int_swap(best_path_list+mype, 1,MASTER_PE);
		subscribed=1; // default subscription on path return
	     } 
    	  }
    	  // not a valid path, ask for another partial path
    	}
	else {

    	  // For each city not yet visited, extend the path:
    	  // (use of symmetric buffer msg_in to compute every extended path)
    	  int length = msg_in.length;
	  int pathcnt = 0;
	  memset(worker_new_path,0,sizeof(Msg_t)*NumCities);
    	  for (int i=msg_in.visited-1; i<NumCities; i++) {
    	    // swap city[i] and city[visted-1]
    	    if (i > msg_in.visited-1) {
    	       int tmp = msg_in.city[msg_in.visited-1];
    	       msg_in.city[msg_in.visited-1] = msg_in.city[i];
    	       msg_in.city[i] = tmp;
    	    }
    	  
    	    // visit city[visited-1]
    	    if (int d = Dist[(msg_in.city[msg_in.visited-2])*NumCities +
			      msg_in.city[msg_in.visited-1] ]) {
    	        msg_in.length = length + d;
    	        if (msg_in.length < newshortestlen) { 
		  memcpy((worker_new_path+pathcnt),&msg_in,sizeof(Msg_t));
		  pathcnt++;
		}
    	    }
    	  }
	  fflush(stdout);
	  memset(&msg_in,0,sizeof(Msg_t));
	  if(pathcnt) {
	    shmem_int_swap(put_path_list+mype, pathcnt,MASTER_PE);
	    subscribed=1; // default subscrition on path return 
	  }
        }

	if (!subscribed) 
	  shmem_int_swap(subscribe_list+mype, 1,MASTER_PE);

        shmem_quiet();
    } /* end of new path check */

    if (shmem_int_fadd(&isdone,0,mype)) { 
        printf("Worker %d received DONE_TAG ..\n", mype); 
        break; 
    }

  } /* end of while(1) */
  shmem_barrier_all();

}

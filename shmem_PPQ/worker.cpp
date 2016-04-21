#include <shmem.h>
#include "list.h"
#include "tsp.h"


#define MASTER_PE 0
extern Msg_t msg_in;
extern int Dist[];
extern int newshortestlen, isnewpath, isdone, NumProcs, mype, isshortest, NumCities;
extern volatile int nwait;
int temp_buf;

void Worker ()
{ 
  newshortestlen = INTMAX;
  isdone = 0; isnewpath = 0;
  
  shmem_barrier_all();

  shmemx_am_request(MASTER_PE, hid_SUBSCRIBE, NULL, 0);
  shmemx_am_quiet();
  while (1) {

    if (shmem_int_swap(&isnewpath,0,mype)) { 
	//printf("Worker %d just received new path\n",mype);
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
    	     if (msg_in.length < newshortestlen) 
    	        shmemx_am_request(MASTER_PE, hid_BESTPATH, &msg_in, sizeof(Msg_t));
    	  }
    	  // not a valid path, ask for another partial path
    	}
	else {
	  //printf("Worker %d evaluating path\n",mype);
	  //fflush(stdout);

    	  // For each city not yet visited, extend the path:
    	  // (use of symmetric buffer msg_in to compute every extended path)
    	  int length = msg_in.length;
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
    	        if (msg_in.length < newshortestlen) 
    	          shmemx_am_request(MASTER_PE, hid_PUTPATH, &msg_in, sizeof(Msg_t));
    	    }
    	  }
        }
        //printf("Worker %d subscribing\n",mype);
        //fflush(stdout);
        shmemx_am_request(MASTER_PE, hid_SUBSCRIBE, NULL, 0);
	shmemx_am_quiet();
    } /* end of new path check */

    if (shmem_int_swap(&isdone,0,mype)) { 
        printf("Worker %d received DONE_TAG ..\n", mype); 
        break; 
    }

  } /* end of while(1) */
  shmem_barrier_all();

}


volatile  int shortestLength, newshortestlen, isnewpath;

void handler_worker_done()
{
	// TODO use a handler safe lock around this
	//isdone = 1;
}

void handler_worker_update_bestpath()
{
	// TODO use a handler safe lock around this
        newshortestlen = msg.length;
}


void handler_worker_newpath()
{
    	msg.visited++;
    	shortestLength = newshortestlen;

    	if (msg.visited==NumCities) {
    	  int d1 = Dist[ (msg.city[NumCities-2])*NumCities + msg.city[NumCities-1] ];
    	  int d2 = Dist[(msg.city[NumCities-1]) * NumCities ];
    	  if (d1 * d2)   { 
    	     // both edges exist 
    	     msg.length += d1 + d2;
    	  
    	     // if path is good, send it to master
    	     if (msg.length < shortestLength) {
    	        //MPI_Send (&msg, MSGSIZE, MPI_INT, 0, BEST_PATH_TAG, MPI_COMM_WORLD);
    	        shmem_am_request(COORD_PE, msg, MSGSIZE*sizeof(int), hid_BESTPATH);
    	     }
    	  }
    	  // not a valid path, ask for another partial path
    	}
	else {
	  // TODO use a handler safe lock around this
          isnewpath = 1;
	}
}


//TODO put a handler safe lock for every use of this //macro
#define IS_DONE   \
      if (isdone) { \
         printf("Worker %d received DONE_TAG ..\n", myrank); \
         break; \
      }


void Worker ()
{ 
  MPI_Status status;
  Msg_t msg;
  newshortestlen = INT_MAX;
  isdone = 0; isnewpath = 0;
  
  shmem_barrier_all();
  printf("Worker started ...\n"); fflush(stdout);

  MPI_Send (NULL, 0, MPI_INT, 0, GET_PATH_TAG, MPI_COMM_WORLD);

  while (1) {
    //MPI_Recv (&msg, MSGSIZE, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    shmemx_am_poll();

    IS_DONE;

    if (isnewpath) { 

    	  // For each city not yet visited, extend the path:
    	  // (use of same msg space to compute every extended the path)
    	  int length = msg.length;
    	  for (int i=msg.visited-1; i<NumCities; i++) {
    	    // swap city[i] and city[visted-1]
    	    if (i > msg.visited-1) {
    	       int tmp = msg.city[msg.visited-1];
    	       msg.city[msg.visited-1] = msg.city[i];
    	       msg.city[i] = tmp;
    	    }
    	  
    	    // visit city[visited-1]
    	    if (int d = Dist[(msg.city[msg.visited-2])*NumCities + msg.city[msg.visited-1] ]) {
    	        msg.length = length + d;
    	        if (msg.length < shortestLength) {
    	          //MPI_Send (&msg, MSGSIZE, MPI_INT, 0, PUT_PATH_TAG, MPI_COMM_WORLD);
    	          shmem_am_request(COORD_PE, msg, MSGSIZE*sizeof(int), hid_PUTPATH);
    	        }
    	    }
    	  }
        }

        IS_DONE;
        isnewpath=0;
        //MPI_Send (NULL, 0, MPI_INT, 0, GET_PATH_TAG, MPI_COMM_WORLD);
        shmem_am_request(COORD_PE, NULL, 0, hid_GETPATH);
    }
}


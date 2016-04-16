
void handler_master_putpath()
{
	  P = new Path();
	  P->Set (msg.length, msg.city, msg.visited);
	  queue.Insert(P, msg.length);
}


void handler_master_bestpath()
{
	if (msg.length < Shortest.length)
	{
           bpath ++;
           printf("Got best path %d, source = %d, length = %d\n", 
                bpath, status.MPI_SOURCE, msg.length);
           fflush(stdout);

	   // update Shortest:
	   // TODO send shortest path with every PE
           Shortest.Set (msg.length, msg.city, NumCities);
           //for( int i = 1 ; i<NumProcs ; i++ )
           //   MPI_Send( &(Shortest.length), 1, MPI_INT, i, 
           //     UPDATE_BEST_PATH_TAG, MPI_COMM_WORLD );
        }
}	


void handler_master_subscribe()
{
	if (!queue.IsEmpty()) 
	{
	  // get a path and send it along with bestlength
	  P = (Path *)queue.Remove(NULL); 
	  msg.length = P->length;
	  memcpy (msg.city, P->city, MAXCITIES*sizeof(int));
	  msg.visited = P->visited;
	  MPI_Send (&msg, MSGSIZE, MPI_INT, status.MPI_SOURCE, 
		    REPLY_PATH_TAG, MPI_COMM_WORLD);
	  delete P;
	}
	else 
	{
	  // requester must wait
	  waiting[nwait++] = status.MPI_SOURCE;
	  if (nwait==NumProcs-1) {
	    // Tell everbody that we're done
	    for (int i=1; i<NumProcs; i++)
	      MPI_Send (NULL, 0, MPI_INT, i, DONE_TAG, MPI_COMM_WORLD);
	  }
	}
	break;
    }
}

int* waiting;     // to save ranks
int nwait = 0;	  // how many are waiting
int bpath = 0;    // just count the number of best
                  // path received

Path Shortest;
LIST queue;
Path *P;    

void Master ()
{

  // To keep track of processes that are waiting for a Path
  waiting = new int[NumProcs];     // to save ranks
  P = new Path;
  queue.Insert(P, 0);	       // initialize queue with the first task
                               // one zero-length path
  Shortest.length = INT_MAX;   // The initial Shortest path must be bad

  shmem_barrier_all();
  printf("Coord started ...\n"); fflush(stdout);
  while (nwait < NumProcs-1) 
  {
    //MPI_Recv (&msg, MSGSIZE, MPI_INT, MPI_ANY_SOURCE, 
    //	      MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
    shmem_am_poll();

	//TODO
	if (nwait>0) 
	{
	  // Don't put path into queue; send it to one waiting process
	  MPI_Send (&msg, MSGSIZE, MPI_INT, waiting[--nwait],
		    REPLY_PATH_TAG, MPI_COMM_WORLD);
	  shmem_am_request();
	} 
  }
  printf("Shortest path:\n");
  Shortest.Print();
}




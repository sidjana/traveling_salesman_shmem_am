//////////////////////////////////////////////////////////////////////////
// Traveling Salesman Problem with OpenSHMEM Active Messages
// Note: this is a C++ program.
//
// This program follows a master - worker communication pattern
//   - Process 0 manages the queue of incomplte paths, and keeps
//   track of the best path.
//   - All other processes are workers that get and put jobs from and into 
//   the queue. Each time they get a path, they are also informed
//   about the best length so far.
//
// Note that, unlike previous examples, this one does not work with
// only one process.
//
// Starting city is assumed to be city 0.
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <shmem.h>

#include "list.h"
#include "tsp.h"
#include "master.h"

// Symmetric Vars
int *waiting, *Dist; // to save peIds
int newshortestlen, isnewpath, isdone, NumProcs, mype, isshortest, nwait, NumCities;
long pSync[_SHMEM_BCAST_SYNC_SIZE];
Msg_t msg_in;


///////////////////////////////////////////////////////////////////////////

Path::Path ()
{ 
  length=0; 
  visited=1;
  for (int i=0; i<NumCities; i++) city[i]=i;
}

void Path::Set (int len, int *cit, int vis)
{
  length = len;
  memcpy (city, cit, NumCities*sizeof(int));
  visited = vis;
}

void Path::Print()
{
  for (int i=0; i<visited; i++) 
     printf("  %d", city[i]);
  printf("; length = %d\n", length);
}

///////////////////////////////////////////////////////////////////////////


void Fill_Dist( void )
{

  // TODO Self initialize input data from a file
  // and not the standard input (as is done now)
  if (mype == 0)   
     scanf("%d", &NumCities);

  // global operation, all processes must call it
  for (int i=0; i < _SHMEM_BCAST_SYNC_SIZE; i++) 
	  pSync[i] = _SHMEM_SYNC_VALUE;

  shmem_barrier_all();
  shmem_broadcast32(&NumCities, &NumCities, 1, 0, 0, 0, NumProcs, pSync);
  assert(NumCities<=MAXCITIES);

  Dist = new int[NumCities*NumCities];

  if (mype == 0)
     for( int i = 0 ; i<NumCities ; i++ )
        for( int j = 0 ; j<NumCities ; j++ )
           scanf("%d", &Dist[i*NumCities + j]);
  
  // global operation, all processes must call it
  shmem_barrier_all();
  shmem_broadcast32(Dist, Dist, NumCities*NumCities, 0, 0, 0, NumProcs, pSync);
  
  if (mype == 0) {       // print the matrix for debugging
     printf("Number of cities: %d\n", NumCities);
     for( int i = 0 ; i<NumCities ; i++ ) {
        for( int j=0 ; j<NumCities ; j++ )
           printf("%5d", Dist[i*NumCities+j] );
        printf("\n");
     }
  }
}



int main(int argc, char *argv[])
{
  shmem_init();
  mype = shmem_my_pe();
  NumProcs = shmem_n_pes();
  shmemx_am_attach(hid_BESTPATH, &handler_master_bestpath);
  shmemx_am_attach(hid_SUBSCRIBE, &handler_master_subscribe);
  shmemx_am_attach(hid_PUTPATH, &handler_master_putpath);

  if (NumProcs<2) {
    printf("At least 2 processes are required\n");
    exit(-1);
  }  

  
  // Initialize distance matrix. Ususally done by one process 
  // and bcast, or initialized from a file in a shared file system.
  Fill_Dist();  // process 0 read the data and broadcast it to the others

  if (mype==0) 
    Master();
  else
    Worker();
  
  shmemx_am_detach(hid_BESTPATH);
  shmemx_am_detach(hid_SUBSCRIBE);
  shmemx_am_detach(hid_PUTPATH);
  shmem_finalize();
  return 0;
}


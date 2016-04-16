//////////////////////////////////////////////////////////////////////////
// Traveling Salesman Problem with MPI
// Note: this is a C++ program.
//
// Process 0 manages the queue of incomplte paths, and keeps
// track of the best path.
//
// All other processes are workers that get and put jobs from and into 
// the queue. Each time they get a path, they are also informed
// about the best length so far.
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
#include <mpi.h>
#include <memory.h>

#include "list.h"
#include "tsp.h"


int myrank, NumProcs, NumCities;

int *Dist;

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

  if (myrank == 0)    // I'll read the matrix
     // actualy, you must redirect the input to a file
     // I'll not print any message, first it reads the number of cities
     // and then it reads the matrix
     scanf("%d", &NumCities);

  // global operation, all processes must call it
  MPI_Bcast( &NumCities, 1, MPI_INT, 0, MPI_COMM_WORLD);
  assert(NumCities<=MAXCITIES);

  Dist = new int[NumCities*NumCities];

  if (myrank == 0)
     for( int i = 0 ; i<NumCities ; i++ )
        for( int j = 0 ; j<NumCities ; j++ )
           scanf("%d", &Dist[i*NumCities + j]);
  
  // global operation, all processes must call it
  MPI_Bcast( Dist,                   // the buffer
             NumCities*NumCities,    // number of elements
             MPI_INT,                // type of elements
             0,                      // the root for the broadcast
             MPI_COMM_WORLD);        // the most used communicator
  
  if (myrank == 0)        // print the matrix for debugging
  {
     printf("Number of cities: %d\n", NumCities);
     for( int i = 0 ; i<NumCities ; i++ )
     {
        for( int j=0 ; j<NumCities ; j++ )
           printf("%5d", Dist[i*NumCities+j] );
        printf("\n");
     }
  }
}




int main(int argc, char *argv[])
{
  MPI_Init (&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &NumProcs);

  if (NumProcs<2) {
    printf("At least 2 processes are required\n");
    exit(-1);
  }  


  // Initialize distance matrix. Ususally done by one process 
  // and bcast, or initialized from a file in a shared file system.
  Fill_Dist();  // process 0 read the data and broadcast it to the others

  if (myrank==0) 
    Master();
  else
    Worker();
  
  MPI_Finalize();
  return 0;
}


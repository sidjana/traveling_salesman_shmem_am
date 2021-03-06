// Tsp.h

#include <stdlib.h>
#include <shmem.h>
#include <shmemx.h>
#include <stdio.h>
#include <memory.h>



const unsigned int MAXCITIES = 25;

class Path {
 public:
  int length;			// Current length of partial path
  int city[MAXCITIES];
  int visited;			// Number of cities in the partial path

  // Array city[] is a permutation of all cities.
  // city[0]..city[visited-1] is the current partial path;
  // city[visited]..city[NumCities-1] are the cities not yet in the path 

  Path();			// Initialize a Path with city 0 visited
  void Set (int len, int *cit, int vis);  // Sets visited, length and city
  void Print();
};


/////////////////////////// Messages stuff ///////////////////////////////

enum {hid_BESTPATH=420, hid_SUBSCRIBE=421, hid_PUTPATH=422};


// This is the payload of PUT_PATH_TAG, GOOD_PATH_TAG, 
// and REPLY_PATH_TAG messages
struct Msg_t {
  int length;
  int city[MAXCITIES];
  int visited;
};

const unsigned int MSGSIZE = sizeof(Msg_t)/sizeof(int);
const unsigned int INTMAX = 32767;

#define MASTER_PE 0
void Master();
void Worker();



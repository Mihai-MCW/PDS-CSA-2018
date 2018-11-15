#define TRACE

#define NB_TIMES_PROD   2 
#define NB_TIMES_CONS  2 

#define NB_PROD_MAX   20
#define NB_CONS_MAX  20

#define NB_POSITIONS  20 // Maximum size of the buffer 

typedef struct {
  char info[80];
  int  type;
  int  rankProd;
} TypeMessage;

typedef struct {
  TypeMessage buffer[NB_POSITIONS];  // Buffer
  int iPut;                        // Indice next put
  int iGet;			     // Indice next get
} CriticalResource;

// Shared Variables
CriticalResource criticRes;  // Modifications so possible conflicts 
int nbPositions;                // Size of the buffer 
                                // No modif so no conflicts
typedef struct {                // Parameters of the threads
  int rank;
  int typeMsg; 
} Parameters;

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
#define TRACE

#define NB_TIMES_PROD   2 
#define NB_TIMES_CONS  2 

#define NB_PROD_MAX   20
#define NB_CONS_MAX  20

#define NB_POSITIONS  20 // Maximum size of the buffer 

pthread_mutex_t mutex_display = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  char info[80];
  int  typeOfMessage;
  int  producerNumber;
} TypeMessage;

typedef struct {
  TypeMessage buffer[NB_POSITIONS];  // Buffer
  int posW;              // Index next put
  int posR;			     // Index next get
} CriticalResource;

typedef struct {                // Parameters of the threads
  int threadNumber;
  int typeOfMessage;
} Parameters;

// Shared Variables
CriticalResource criticalRes;  // Modifications so possible conflicts 
int nbPositions;                // Size of the buffer 

// Semaphores / mutexes
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t semProd, semCons, semDisplay;
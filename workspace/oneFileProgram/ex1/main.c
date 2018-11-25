#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

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

/*---------------------------------------------------------------------*/
/* codeErr : code returned by a primitive 
 * msgErr  : error message, personalised
 * valErr  : returned value by the thread
 */
/*--------------------------------------------------*/

void thdError(int codeErr, char *msgErr, int valueErr) {
  int *retour = malloc(sizeof(int));
  *retour = valueErr;
  fprintf(stderr, "%s: %d (%s) \n", msgErr, codeErr, strerror(codeErr));
  pthread_exit(retour);
}

/*--------------------------------------------------*/
void initializeSharedVariables(void) {
  int i;
  /* The buffer, its indices and the number of full positions */
  criticalRes.posW = 0;
  criticalRes.posR = 0;
  for (i = 0; i < nbPositions; i++) {
    strcpy(criticalRes.buffer[i].info, "Empty");
    criticalRes.buffer[i].typeOfMessage = -1;
    criticalRes.buffer[i].producerNumber = -1;
  }
}

/*--------------------------------------------------*/
void showBuffer (void) {
  int i;
	
  printf("[ ");
  for (i = 0; i < nbPositions; i++) {
	  if (criticalRes.buffer[i].producerNumber == -1) printf("(Empty)");
	  else printf("(%d, %s, %d)",
		   criticalRes.buffer[i].typeOfMessage, criticalRes.buffer[i].info, criticalRes.buffer[i].producerNumber);
  }
  printf("]\n");
}

/*--------------------------------------------------*/
void makePut (TypeMessage theMessage) {
  strcpy(criticalRes.buffer[criticalRes.posW].info, theMessage.info);
  criticalRes.buffer[criticalRes.posW].typeOfMessage = theMessage.typeOfMessage;
  criticalRes.buffer[criticalRes.posW].producerNumber = theMessage.producerNumber;
  criticalRes.posW = (criticalRes.posW + 1) % nbPositions;
}

/*--------------------------------------------------*/
void makeGet (TypeMessage *theMessage) {
  strcpy(theMessage->info, criticalRes.buffer[criticalRes.posR].info);
  theMessage->typeOfMessage = criticalRes.buffer[criticalRes.posR].typeOfMessage;
  theMessage->producerNumber = criticalRes.buffer[criticalRes.posR].producerNumber;
	
	criticalRes.buffer[criticalRes.posR].producerNumber = -1;
	criticalRes.buffer[criticalRes.posR].typeOfMessage = -1;
	strcpy(criticalRes.buffer[criticalRes.posR].info, "Empty");

  criticalRes.posR = (criticalRes.posR + 1) % nbPositions;
}
/*--------------------------------------------------*/
void makePutSecure(TypeMessage Message){ // puts the message securely
  extern sem_t semProd, semCons;
  extern pthread_mutex_t mutex;

  sem_wait(&semProd);
  pthread_mutex_lock(&mutex);
  
  makePut(Message); // accesses the critical res
  printf("\x1B[35m\nA message was put\n\x1B[0m");

  pthread_mutex_unlock(&mutex);
  sem_post(&semCons);
 };
 void makeGetSecure(TypeMessage *Message){ //gets the message securely
  extern sem_t semProd, semCons;
  extern pthread_mutex_t mutex;
  sem_wait(&semCons);
  pthread_mutex_lock(&mutex);

  makeGet(Message); // accesses the critical res
  printf("\x1B[36m\nA message was taken\n\x1B[0m");

  pthread_mutex_unlock(&mutex);
  sem_post(&semProd);
 };
/*--------------------------------------------------*/
void * producer (void *arg) {
  extern sem_t semProd, semCons;
  extern pthread_mutex_t mutex;
  int i;
  TypeMessage theMessage;
  Parameters *param = (Parameters *)arg;
  
	sleep(1); // to make sure that all consumers and producers have been created before starting --> only for display reasons, so that there is no interleaving...
	
  for (i = 0; i < NB_TIMES_PROD; i++) {
    sem_wait(&semCons);
    pthread_mutex_lock(&mutex);
    
    sprintf (theMessage.info, "%s n°%d", "Hello", i);
    theMessage.typeOfMessage = param->typeOfMessage;
    theMessage.producerNumber = param->threadNumber;

    printf("\tProducer %d : Message = (%d, %s, %d)\n",
			 param->threadNumber, theMessage.typeOfMessage, theMessage.info, theMessage.producerNumber);

	  makePut(theMessage);
    #ifdef TRACE
	    showBuffer();
    #endif
    printf("\x1B[35m\nA message was put\n\x1B[0m");
    //usleep(rand()%(100 * param->rang + 100));
    pthread_mutex_unlock(&mutex);
    sem_post(&semProd);
  }
  return NULL;
}

/*--------------------------------------------------*/
void * consumer (void *arg) {
  extern sem_t semProd, semCons;
  extern pthread_mutex_t mutex;
  int i;
  TypeMessage theMessage;
  Parameters *param = (Parameters *)arg;
	sleep(1); // to make sure that all consumers and producers have been created before starting --> only for display reasons, so that there is no interleaving...

  for (i = 0; i < NB_TIMES_CONS; i++) {
	  sem_wait(&semProd);
    pthread_mutex_lock(&mutex);

	  //pthread_mutex_lock(&mutex_display);
	  makeGet(&theMessage);
	  printf("\t\tConso %d : Message = (%d, %s, %d)\n",
           param->threadNumber, theMessage.typeOfMessage, theMessage.info, theMessage.producerNumber);
    #ifdef TRACE
	  showBuffer();
    #endif
    printf("\x1B[36m\nA message was taken\n\x1B[0m");
    pthread_mutex_unlock(&mutex);
    sem_post(&semCons);
	  //pthread_mutex_unlock(&mutex_display);
    //usleep(rand()%(100 * param->rang + 100));
  }
  return NULL;
}

void initializeSemaphore(sem_t *sem, int pshared, unsigned int value){
    if (0 != sem_init(sem, pshared, value)) {
        perror("Unable to create a semaphore");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
  int i, state;
  int nbThds, nbProd, nbCons;
  Parameters paramThds[NB_PROD_MAX + NB_CONS_MAX];
  pthread_t idThdProd[NB_PROD_MAX], idThdConso[NB_CONS_MAX];

  if (argc < 3) {
    printf ("Usage: %s <Nb Producers <= %d> <Nb Consumers <= %d> <Nb Positions <= %d> \n", 
             argv[0], NB_PROD_MAX, NB_CONS_MAX, NB_POSITIONS);
    exit(2);
  }
	pthread_mutex_init(&mutex_display, NULL);
//	srand((int)pthread_self());
	
  nbProd  = atoi(argv[1]);
  if (nbProd > NB_PROD_MAX)
    nbProd = NB_PROD_MAX;
  nbCons = atoi(argv[2]);
  if (nbCons > NB_CONS_MAX)
    nbCons = NB_CONS_MAX;
  nbThds = nbProd + nbCons;
  nbPositions = atoi(argv[3]);
  if (nbPositions > NB_POSITIONS)
    nbPositions = NB_POSITIONS;

  initializeSharedVariables();
  initializeSemaphore(&semProd,0,nbPositions);
  initializeSemaphore(&semCons,0,0);
  initializeSemaphore(&semDisplay,0,1);
  pthread_mutex_init(&mutex,0);

  /* Creation of nbProd producers and nbConso consumers */
  for (i = 0; i <  nbThds; i++) {
    if (i < nbProd) {
      paramThds[i].typeOfMessage = i%2;
      paramThds[i].threadNumber = i;
      if ((state = pthread_create(&idThdProd[i], NULL, producer, &paramThds[i])) != 0)
        thdError(state, "Creation producer", state);
      #ifdef TRACE
        printf("Creation thread producer n°%d -> %d/%d\n", i, paramThds[i].threadNumber, nbProd-1);
      #endif
    }
    else {
      paramThds[i].typeOfMessage = i%2;
      paramThds[i].threadNumber = i - nbProd;
      if ((state = pthread_create(&idThdConso[i-nbProd], NULL, consumer, &paramThds[i])) != 0)
        thdError(state, "Creation consumer", state);
      #ifdef TRACE
        printf("Creation thread consumer n°%d -> %d/%d\n", i-nbProd, paramThds[i].threadNumber, nbCons-1);
      #endif
    }
  }

  /* Wait the end of threads */
  for (i = 0; i < nbProd; i++) {
    if ((state = pthread_join(idThdProd[i], NULL)) != 0)
      thdError(state, "Join threads producers", state);
    #ifdef TRACE
      pthread_mutex_lock(&mutex_display);
      printf("End thread producer n°%d\n", i);
      pthread_mutex_unlock(&mutex_display);
    #endif
  }

  for (i = 0; i < nbCons; i++) {
    if ((state = pthread_join(idThdConso[i], NULL)) != 0) 
      thdError(state, "Join threads consumers", state);
    #ifdef TRACE
      pthread_mutex_lock(&mutex_display);
      printf("End thread consumer n°%d\n", i);
      pthread_mutex_unlock(&mutex_display);
    #endif
  }
    
  #ifdef TRACE
    printf ("\nEnd of main \n");
    showBuffer();
  #endif

  return 0;
}
  

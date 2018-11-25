#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include "definitions.h"
#include "functions.h"
#include "utility.h"
/*---------------------------------------------------------------------*/
/* codeErr : code returned by a primitive 
 * msgErr  : error message, personalised
 * valErr  : returned value by the thread
 */
/*--------------------------------------------------*/



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
  initializeSemaphore(&empty,0,nbPositions);
  initializeSemaphore(&full,0,0);
  initializeSemaphore(&black,0,1);
  initializeSemaphore(&white,0,0);
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
  

/*
 * Producer-consommer, base without synchronisation
 * */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include "definitions.h"
#include "sem.h"
#include "functions.h"
#include "utility.h"
sem_t semProd, semCons;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;;
int amount = 0;
/*---------------------------------------------------------------------*/
/* codeErr : code retourned by a primitive 
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
  initializeSemaphore(&semProd,0,1);
  initializeSemaphore(&semCons,0,0);
  pthread_mutex_init(&mutex,0);
  
  /* Creation of nbProd producers and nbConso consumers */
  for (i = 0; i <  nbThds; i++) {
    if (i < nbProd) {
      paramThds[i].typeMsg = i%2;
      paramThds[i].rank = i;
      if ((state = pthread_create(&idThdProd[i], NULL, producer, &paramThds[i])) != 0)
        thdErr(state, "Creation producer", state);
      #ifdef TRACE
        printf("Creation thread prod %lu, number %d\n", (unsigned long) idThdProd[i], paramThds[i].rank);
      #endif
    }
    else {
      paramThds[i].typeMsg = i%2;
      paramThds[i].rank = i;
      if ((state = pthread_create(&idThdConso[i-nbProd], NULL, consumer, &paramThds[i])) != 0)
        thdErr(state, "Creation consumer", state);
      #ifdef TRACE
        printf("Creation thread consumer %lu number %d\n", (unsigned long) idThdConso[i-nbProd], paramThds[i].rank);
      #endif
    }
  }
	

  /* Wait the end of threads */
  for (i = 0; i < nbProd; i++) {
    if ((state = pthread_join(idThdProd[i], NULL)) != 0)
      thdErr(state, "Join threads producers", state);
    #ifdef TRACE
        printf("End thread producer of rank %d\n", i);
    #endif
  }

  for (i = 0; i < nbCons; i++) {
    if ((state = pthread_join(idThdConso[i], NULL)) != 0) 
      thdErr(state, "Join threads consumers", state);
    #ifdef TRACE
      printf("End thread consumer of rank %d\n", i);
    #endif
  }
    
#ifdef TRACE
  printf ("\nEnd of main \n");
	showBuffer("Final State of the buffer", NULL);
#endif

sem_destroy(&semProd);
sem_destroy(&semCons);
pthread_mutex_destroy(&mutex);

  return 0;
}

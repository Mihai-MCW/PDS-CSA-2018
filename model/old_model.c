/*
 * Producer-consommer, base without synchronisation
 * */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define TRACE

#define NB_TIMES_PROD   2 
#define NB_TIMES_CONS  2 

#define NB_PROD_MAX   20
#define NB_CONS_MAX  20

#define NB_POSITIONS  20 // Maximum size of the buffer 

static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  char info[80];
  int  type;
  int  rankProd;
} TypeMessage;

typedef struct {
  TypeMessage buffer[NB_POSITIONS];  // Buffer
  int iDepot;                        // Indice next put
  int iRetrait;			     // Indice next get
} CriticalResource;

// Shared Variables
CriticalResource resCritiques;  // Modifications so possible conflicts 
int nbPositions;                // Size of the buffer 
                                // No modif so no conflicts
typedef struct {                // Parameters of the threads
  int rank;
  int typeMsg; 
} Parameters;

/*---------------------------------------------------------------------*/
/* codeErr : code retourned by a primitive 
 * msgErr  : error message, personalised
 * valErr  : returned value by the thread
 */
void thdErreur(int codeErr, char *msgErr, int valeurErr) {
  int *retour = malloc(sizeof(int));
  *retour = valeurErr;
  fprintf(stderr, "%s: %d (%s) \n", msgErr, codeErr, strerror(codeErr));
  pthread_exit(retour);
}

/*--------------------------------------------------*/
void initializeSharedVariables(void) {
  int i;
  /* The buffer, its indices and the number of full positions */
  resCritiques.iDepot = 0;
  resCritiques.iRetrait = 0;
  for (i = 0; i < nbPositions; i++) {
    strcpy(resCritiques.buffer[i].info, "Empty");
    resCritiques.buffer[i].type = 0;
    resCritiques.buffer[i].rankProd = -1;
  }
}

/*--------------------------------------------------*/
void showBuffer (char *s, Parameters *param) {
  int i;
	pthread_mutex_lock(&m);
	if (param != NULL) printf("%d : %s : [ ", param->rank, s);
	else printf("END : %s : [ ", s);
  for (i = 0; i < nbPositions; i++) {
	  if (strcmp(resCritiques.buffer[i].info, "Empty"))
		{
	  		printf("[Type%d] %s, ",
            resCritiques.buffer[i].type,
            resCritiques.buffer[i].info);
	  }
	  else printf("Empty,");
  }
  printf("]\n");
	pthread_mutex_unlock(&m);

}

/*--------------------------------------------------*/
void makePut (TypeMessage leMessage) {
  strcpy(resCritiques.buffer[resCritiques.iDepot].info, leMessage.info);
  resCritiques.buffer[resCritiques.iDepot].type = leMessage.type;
  resCritiques.buffer[resCritiques.iDepot].rankProd = leMessage.rankProd;
  resCritiques.iDepot = (resCritiques.iDepot + 1) % nbPositions;
}

/*--------------------------------------------------*/
void makeGet (TypeMessage *leMessage) {
  strcpy(leMessage->info, resCritiques.buffer[resCritiques.iRetrait].info);
  strcpy(resCritiques.buffer[resCritiques.iRetrait].info, "Empty");
  leMessage->type = resCritiques.buffer[resCritiques.iRetrait].type;
	resCritiques.buffer[resCritiques.iRetrait].type=0;
  leMessage->rankProd = resCritiques.buffer[resCritiques.iRetrait].rankProd;
	resCritiques.buffer[resCritiques.iRetrait].rankProd=-1;
  resCritiques.iRetrait = (resCritiques.iRetrait + 1) % nbPositions;
}
  
/*--------------------------------------------------*/
void * producer (void *arg) {
  int i;
  TypeMessage leMessage;
  Parameters *param = (Parameters *)arg;

  srand((int)pthread_self());

  for (i = 0; i < NB_TIMES_PROD; i++) {
    sprintf (leMessage.info, "%d/%d", i, param->rank);
    leMessage.type = param->typeMsg;
    leMessage.rankProd = param->rank;

#ifdef TRACE
	  showBuffer("Before Putting", param);
#endif
    makePut(leMessage);
    printf("\tProducer %d : Message put = [T%d] %s (by %d)\n",
           param->rank, leMessage.type, leMessage.info, leMessage.rankProd);
#ifdef TRACE
	  showBuffer("After Putting", param);
#endif
    //usleep(rand()%(100 * param->rang + 100));
  }
  return NULL;
}

/*--------------------------------------------------*/
void * consumer (void *arg) {
  int i;
  TypeMessage unMessage;
  Parameters *param = (Parameters *)arg;

  for (i = 0; i < NB_TIMES_CONS; i++) {
#ifdef TRACE
	  showBuffer("Before Getting", param);
#endif
	makeGet(&unMessage);
    printf("\t\tConso %d : Message got = [T%d] %s (from %d)\n",
           param->rank, unMessage.type, unMessage.info, unMessage.rankProd);
#ifdef TRACE
	  showBuffer("After Getting", param);
#endif
    //usleep(rand()%(100 * param->rang + 100));
  }
  return NULL;
}

/*--------------------------------------------------*/
int main(int argc, char *argv[]) {
  int i, etat;
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

  /* Creation of nbProd producers and nbConso consumers */
  for (i = 0; i <  nbThds; i++) {
    if (i < nbProd) {
      paramThds[i].typeMsg = i%2;
      paramThds[i].rank = i;
      if ((etat = pthread_create(&idThdProd[i], NULL, producer, &paramThds[i])) != 0)
        thdErreur(etat, "Creation producer", etat);
#ifdef TRACE
      printf("Creation thread prod %lu, number %d\n", (unsigned long) idThdProd[i], paramThds[i].rank);
#endif
    }
    else {
      paramThds[i].typeMsg = i%2;
      paramThds[i].rank = i;
      if ((etat = pthread_create(&idThdConso[i-nbProd], NULL, consumer, &paramThds[i])) != 0)
        thdErreur(etat, "Creation consumer", etat);
#ifdef TRACE
      printf("Creation thread consumer %lu number %d\n", (unsigned long) idThdConso[i-nbProd], paramThds[i].rank);
#endif
    }
  }
	

  /* Wait the end of threads */
  for (i = 0; i < nbProd; i++) {
    if ((etat = pthread_join(idThdProd[i], NULL)) != 0)
      thdErreur(etat, "Join threads producers", etat);
#ifdef TRACE
    printf("End thread producer of rank %d\n", i);
#endif
  }

  for (i = 0; i < nbCons; i++) {
    if ((etat = pthread_join(idThdConso[i], NULL)) != 0) 
      thdErreur(etat, "Join threads consumers", etat);
#ifdef TRACE
    printf("End thread consumer of rank %d\n", i);
#endif
  }
    
#ifdef TRACE
  printf ("\nEnd of main \n");
	showBuffer("Final State of the buffer", NULL);
#endif

  return 0;
}


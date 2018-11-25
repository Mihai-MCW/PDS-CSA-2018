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
/*****************************************************************
****************************** START *****************************
*********************** SECURED PUT AND GET *********************** 
******************************************************************/

void makePutSecure (TypeMessage theMessage, Parameters *param){
  if( theMessage.typeOfMessage == 0 ){ sem_wait(&black); printf("\x1B[32m\nblack\n\x1B[0m");}
    else {sem_wait(&white); printf("\x1B[31m\nwhite\n\x1B[0m");}
  sem_wait(&empty);
  pthread_mutex_lock(&mutex);
  ////////////////////   
  makePut(theMessage);
  printf("\x1B[35m\nA message was put of type %d\n\x1B[0m",theMessage.typeOfMessage);
  ///////////////////
  pthread_mutex_unlock(&mutex);
  sem_post(&full);
}

void makeGetSecure (TypeMessage *theMessage, Parameters *param) {
  sem_wait(&full);
  pthread_mutex_lock(&mutex);
  ////////////////////
  makeGet(theMessage);
  printf("\x1B[36m\nA message was taken of type %d\n\x1B[0m",theMessage->typeOfMessage);
  ////////////////////
  pthread_mutex_unlock(&mutex);
  sem_post(&empty);
}
/*****************************************************************
****************************** END ******************************
*********************** SECURED PUT AND GET *********************** 
******************************************************************/
void * producer (void *arg) {
  extern sem_t empty, full, black, white;
  extern pthread_mutex_t mutex;
  extern int prod;
  int i;
  TypeMessage theMessage;
  Parameters *param = (Parameters *)arg;
  
	sleep(1); // to make sure that all consumers and producers have been created before starting --> only for display reasons, so that there is no interleaving...
	
  for (i = 0; i < NB_TIMES_PROD; i++) {
    sprintf (theMessage.info, "%s n°%d", "Hello", i);
    theMessage.typeOfMessage = param->typeOfMessage;
    theMessage.producerNumber = param->threadNumber;

    printf("\tProducer %d : Message = (%d, %s, %d)\n",
			 param->threadNumber, theMessage.typeOfMessage, theMessage.info, theMessage.producerNumber);

	  makePutSecure(theMessage,param); 
    #ifdef TRACE
	    showBuffer();
    #endif
    //usleep(rand()%(100 * param->rang + 100));
    
    if ( theMessage.typeOfMessage == 0 ){ sem_post(&white);}
    else{ sem_post(&black);}
  }
  return NULL;
}

/*--------------------------------------------------*/
void * consumer (void *arg) {
  extern sem_t empty, full, black, white;
  extern pthread_mutex_t mutex;
  extern int cons;
  int i;
  TypeMessage theMessage;
  Parameters *param = (Parameters *)arg;
	sleep(1); // to make sure that all consumers and producers have been created before starting --> only for display reasons, so that there is no interleaving...

  for (i = 0; i < NB_TIMES_CONS; i++) {
	  
	  //pthread_mutex_lock(&mutex_display);
	  makeGetSecure(&theMessage, param);
	  printf("\t\tConso %d : Message = (%d, %s, %d)\n",
           param->threadNumber, theMessage.typeOfMessage, theMessage.info, theMessage.producerNumber);
    #ifdef TRACE
	  showBuffer();
    #endif
    
	  //pthread_mutex_unlock(&mutex_display);
    //usleep(rand()%(100 * param->rang + 100));
  }
  return NULL;
}

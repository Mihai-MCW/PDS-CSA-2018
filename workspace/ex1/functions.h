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
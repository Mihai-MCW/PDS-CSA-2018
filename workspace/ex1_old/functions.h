
/********************************************************
* * * * * This file contains the main functions * * * * *
*********************************************************/

void thdErr(int codeErr, char *msgErr, int valeurErr) {
  int *retour = malloc(sizeof(int));
  *retour = valeurErr;
  fprintf(stderr, "%s: %d (%s) \n", msgErr, codeErr, strerror(codeErr));
  pthread_exit(retour);
}

/*--------------------------------------------------*/
void initializeSharedVariables(void) {
  int i;
  /* The buffer, its indices and the number of full positions */
  criticRes.iPut = 0;
  criticRes.iGet = 0;
  for (i = 0; i < nbPositions; i++) {
    strcpy(criticRes.buffer[i].info, "Empty");
    criticRes.buffer[i].type = 0;
    criticRes.buffer[i].rankProd = -1;
  }
}

/*--------------------------------------------------*/
void showBuffer (char *s, Parameters *param) {
  int i;
	pthread_mutex_lock(&m);
	if (param != NULL) printf("%d : %s : [ ", param->rank, s);
	else printf("END : %s : [ ", s);
  for (i = 0; i < nbPositions; i++) {
	  if (strcmp(criticRes.buffer[i].info, "Empty"))
		{
	  		printf("[Type%d] %s, ",
            criticRes.buffer[i].type,
            criticRes.buffer[i].info);
	  }
	  else printf("Empty,");
  }
  printf("]\n");
	pthread_mutex_unlock(&m);

}

/*--------------------------------------------------*/
void makePut (TypeMessage Message) {
  strcpy(criticRes.buffer[criticRes.iPut].info, Message.info);
  criticRes.buffer[criticRes.iPut].type = Message.type;
  criticRes.buffer[criticRes.iPut].rankProd = Message.rankProd;
  criticRes.iPut = (criticRes.iPut + 1) % nbPositions;
}

/*--------------------------------------------------*/
void makeGet (TypeMessage *Message) {
  strcpy(Message->info, criticRes.buffer[criticRes.iGet].info);
  strcpy(criticRes.buffer[criticRes.iGet].info, "Empty");
  Message->type = criticRes.buffer[criticRes.iGet].type;
	criticRes.buffer[criticRes.iGet].type=0;
  Message->rankProd = criticRes.buffer[criticRes.iGet].rankProd;
	criticRes.buffer[criticRes.iGet].rankProd=-1;
  criticRes.iGet = (criticRes.iGet + 1) % nbPositions;
}

/*--------------------------------------------------*/
/*--------------------------------------------------*/
/*--------------------------------------------------*/
 void makePutSecure(TypeMessage Message){ // puts the message securely
  extern sem_t semProd, semCons;
  extern pthread_mutex_t mutex;
  extern int amount;
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
  extern int amount;
  sem_wait(&semCons);
  pthread_mutex_lock(&mutex);

  makeGet(Message); // accesses the critical res
  printf("\x1B[36m\nA message was taken\n\x1B[0m");
  amount--;

  pthread_mutex_unlock(&mutex);
  sem_post(&semProd);
 };
/*--------------------------------------------------*/
/*--------------------------------------------------*/
/*--------------------------------------------------*/
void * producer (void *arg) {
  int i;
  TypeMessage Message;
  Parameters *param = (Parameters *)arg;

  srand((int)pthread_self());

  for (i = 0; i < NB_TIMES_PROD; i++) {
    sprintf (Message.info, "%d/%d", i, param->rank);
    Message.type = param->typeMsg;
    Message.rankProd = param->rank;

    #ifdef TRACE
        showBuffer("Before Putting", param);
    #endif

    makePutSecure(Message);
      
    printf("\x1B[33m\tProducer %d : Message put = [T%d] %s (by %d)\n\x1B[0m" ,
            param->rank, Message.type, Message.info, Message.rankProd);
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
  TypeMessage aMessage;
  Parameters *param = (Parameters *)arg;

  for (i = 0; i < NB_TIMES_CONS; i++) {
    #ifdef TRACE
        showBuffer("Before Getting", param);
    #endif
	  makeGetSecure(&aMessage);
    printf("\x1B[32m\t\tConsumer %d : Message got = [T%d] %s (from %d)\n\x1B[0m",
           param->rank, aMessage.type, aMessage.info, aMessage.rankProd);
    #ifdef TRACE
        showBuffer("After Getting", param);
    #endif
    //usleep(rand()%(100 * param->rang + 100));
  }
  return NULL;
}
void initializeSemaphore(sem_t *sem, int pshared, unsigned int value){
    if (0 != sem_init(sem, pshared, value)) {
        perror("Unable to create a semaphore");
        exit(EXIT_FAILURE);
    }
}

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

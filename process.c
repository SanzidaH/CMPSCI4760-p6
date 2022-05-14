/**
* @Author: Sanzida Hoque
* @Course: CMP_SCI 4760 Operating Systems 
* @Sources: https://t4tutorials.com/fifo-page-replacement-algorithm-memory-management-technique-operating-systems
            https://www.geeksforgeeks.org/page-replacement-algorithms-in-operating-systems/
            https://www.geeksforgeeks.org/program-page-replacement-algorithms-set-2-fifo/
*           
*/


#include "config.h"

int pid, pno, addRequested;

int main(int argc, char *argv[]){    

/* signal handling with cleanAll */
    signal(SIGALRM, cleanAll); 
    signal(SIGINT, cleanAll); 
    signal(SIGTERM, cleanAll);


/* Ensuring that IPC_CREAT created the segment in oss */
    clock_nsid = shmget(ftok("Makefile", '1'), sizeof(unsigned int), IPC_EXCL | 0666);
    clock_sid = shmget(ftok("Makefile", '2'), sizeof(unsigned int), IPC_EXCL | 0666);
    pct_id = shmget(ftok("Makefile", '3'), sizeof(pct), IPC_EXCL | 0666);
    //printf("ipcrm -m %d\n", clock_nsid);
    //printf("ipcrm -m %d\n", clock_sid);
     
    if (clock_nsid == -1 || clock_sid == -1 || pct_id == -1){
        perror("process: Error: Shared memory allocation failed");
        abort();
     // return 1;
    }
       
    clock_ns = (unsigned int *)shmat(clock_nsid, NULL, 0);
    clock_s = (unsigned int *)shmat(clock_sid, NULL, 0);
    pct = (struct PCT*)shmat(pct_id, 0, 0);
  
    if (clock_ns == (void *) -1 || clock_s == (void *) -1 || pct == (void *) -1) {
       perror("process: Error: Shared memory attachment failed");
       abort();
      //return 1;
    }
   
    srand(time(NULL) + getpid());
    pno = -1;
    for (int i = 0; i < MAXPROC; i++){
        if (getpid() == pct->inUse[i]){
            pno = i;
            break;
        }
    }
    
    
   if(pno== -1){
        cleanAll();
   }
   else{
    set_sem();
    sem_wait(18);
    sem_signal(18);

    while (1){
    /*  Multiply that page number by 1024 and then
        add a random offset of from 0 to 1023 to get the actual memory address requested. */
               
        addRequested = ((rand() % 33) * 1024) + (rand() % 1023);
        /* Do this with randomness, but bias it towards reads */
        if (rand() % 100 <= 20){
            pct->pcb[pno].reqtype = WRITE;
        }
        else{
            pct->pcb[pno].reqtype = READ;
        }
        /* request queued */
        pct->pcb[pno].requested = addRequested;
        sem_wait(pno);
        while (semctl(sem_id, pno, GETVAL, 0) != 1);
        /* user process will check whether it should terminate */
        int prob = rand() % 100;
        if ((prob = 10)){
            pct->pcb[pno].usedAll = true;
            break;
        }
      increase_clock(10);
      }
   }
    cleanAll();
}

void increase_clock(long unsigned int inc){
        clock_ns[0] = clock_ns[0] + inc;//Increasing system clock
        while (clock_ns[0] >= TO_NANO) {//Coversion of nanosecond to second 
               clock_s[0] = clock_s[0] + 1;
               clock_ns[0] = clock_ns[0] - TO_NANO;
         }
}

void cleanAll(){
    shmdt(pct);
    sem_wait(19);
    abort();
   // exit(EXIT_SUCCESS);
}

/* Semaphor */

void sem_signal(int sem){    
    struct sembuf sb = {sem, 1, 0}; //free resource
    if (semop(sem_id, &sb, 1) == -1) {
        perror("process: semop failed\n");
        cleanAll();        
    }
}

void sem_wait(int sem){    
    struct sembuf sb = {sem, -1, 0}; //allocate resource
    if (semop(sem_id, &sb, 1) == -1) {
        perror("process: semop failed\n");
        cleanAll();        
    }
}

void set_sem(){
    if((sem_id = semget(ftok("oss.c", '4'), 20, 0)) == -1){
        perror("process: semget failed\n");
        cleanAll();
    }
}



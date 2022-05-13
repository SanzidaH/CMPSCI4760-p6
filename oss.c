/**
* @Author: Sanzida Hoque
* @Course: CMP_SCI 4760 Operating Systems 
* @Sources: https://t4tutorials.com/fifo-page-replacement-algorithm-memory-management-technique-operating-systems
            https://www.geeksforgeeks.org/page-replacement-algorithms-in-operating-systems/
*           
*/


#include "config.h"

unsigned int processScheduleNS = 0, trackNS = 0, frameIndex = 0;
int forkedchild = 0, sysForks = 0, memAccessCount = 0, pageFaultCount = 0;


int main(int argc, char *argv[]){
    /* signal handling with cleanAll*/
    signal(SIGALRM, cleanAll); 
    signal(SIGINT, cleanAll); 
    signal(SIGSEGV, cleanAll); 
    signal(SIGKILL, cleanAll); 

    /* Creating a new shared memory segment */ 
    clock_nsid = shmget(ftok("Makefile", '1'), sizeof(unsigned int), IPC_CREAT | 0666);
    clock_sid = shmget(ftok("Makefile", '2'), sizeof(unsigned int), IPC_CREAT | 0666);
    pct_id = shmget(ftok("Makefile", '3'), sizeof(pct), IPC_CREAT | 0666);
    
    if (clock_nsid == -1 || clock_sid == -1 || pct_id == -1){
        perror("oss: Error: Shared memory allocation failed");
     // return 1;
        abort();
    }
    
    clock_ns = (unsigned int *)shmat(clock_nsid, NULL, 0);
    clock_s = (unsigned int *)shmat(clock_sid, NULL, 0);
    pct = (struct PCT *)shmat(pct_id, 0, 0);
    
    if (clock_ns == (void *) -1 || clock_s == (void *) -1 || pct == (void *) -1) {
       perror("oss: Error: Shared memory attachment failed");
       abort();
      //return 1;
    }

    /* Setting System clock to zero */
    clock_ns[0] = 0;
    clock_s[0] = 0;
 
   /* Initializing PCT and Frame Table */
    for(int proc = 0; proc < MAXPROC; proc++){
        pct->inUse[proc] = 0;
        pct->procs[proc].usedAll = false;
        pct->procs[proc].requested = 0;
        pct->procs[proc].pageCount = 0;
        pct->procs[proc].pageIndex = 0;
        for(int pg = 0; pg < PROCMEM; pg++){
            pct->procs[proc].pageTable[pg].frame_num = -1;
            pct->procs[proc].pageTable[pg].address = 0;
        }
    }
    for(int f = 0; f < MAXMEM; f++){
        frametable[f].procnum = 0;
        frametable[f].dirtyBit = 0;
        frametable[f].address = 0;
    }
    
    /* Logging */
    file = fopen("osslog", "w+");
    srand(time(NULL));
    sem_init();
    
    /*You should terminate after more than 100 processes have gotten into your system, or if more than 2 real life seconds have passed*/
    alarm(2);
    fputs("operating system simulator main process starts...\n",file);
    while(1){    
           
   /*  oss should periodically check if all the processes are queued for device and if
      so, advance the clock to fulfill the request at the head; resolve any possible deadlock in case
       memory is low and all processes end up waiting */

        int waitingQ[MAXPROC] = {0};
        for (int p = 0; p < MAXPROC; p++){
            if (pct->procs[p].usedAll){
                semctl(sem_id, p, SETVAL, 1);
                pct->procs[p].pageCount = 0;
                pct->procs[p].pageIndex = 0;
                pct->procs[p].requested = 0;
                pct->procs[p].reqtype = 0;
                pct->procs[p].usedAll = false;
               for (int pg = 0; pg < PROCMEM; pg++){
                  if (pct->procs[p].pageTable[pg].frame_num != -1){
                     frametable[pct->procs[p].pageTable[pg].frame_num].procnum = 0;
                     frametable[pct->procs[p].pageTable[pg].frame_num].dirtyBit = 0;
                     frametable[pct->procs[p].pageTable[pg].frame_num].address = 0;
                  }
               }
            pct->inUse[p] = 0;
          }
           if (semctl(sem_id, p, GETVAL, 0) == 0){
              waitingQ[p]++;
           }      
       }
  

   for (int p = 0; p < MAXPROC; p++){
    if (waitingQ[p] >= MAXPROC){
        killpg(pct->inUse[p],SIGTERM);    
    }
   }   
    
    /* fork a user process at random times when user processes in system is less than 18 */   
    if (processScheduleNS == 0 || ((((*clock_s * TO_NANO) + *clock_ns) > processScheduleNS) && semctl(sem_id, 19, GETVAL, 0) < MAXPROC)){//&& forkedchild < 30                                  
    	/* Terminate if more that 100 processes have gotten into system */
    	if (sysForks >= 100){
      	  fputs("OSS: more than 100 processes in system, Terminating..\n", file);
      	  printf("OSS: more than 100 processes in system, Terminating..\n");
          cleanAll();
 	 }
        else{
           for (int p = 0; p < MAXPROC; p++){
               if(pct->inUse[p] == 0){
                  sem_signal(19);
                  sysForks++;
                  childPid = fork();
                  if(childPid != 0){
                      fprintf(file, "Master: P%d (pid %d) forked at %d : %d\n", p, childPid, *clock_s, *clock_ns); 
                      pct->inUse[p] = childPid;
                      processScheduleNS =  (*clock_s * TO_NANO) + *clock_ns + ((rand() % 500000000) + 1000000);   //random time for forking (1-500ms)
                      fprintf(file, "Scheduled next fork after: %u ns\n", processScheduleNS);
                      forkedchild++;            
                  }
                  else{                  
                       execl("./process", "./process", NULL);
                  }
               }
            }
         }
      }
       
        for (int i = 0; i < MAXPROC; i++){
            if (semctl(sem_id, i, GETVAL, 0) == 0){ //frame available 
                    if (pct->procs[i].reqtype == READ){ //Read type
                        /* Master: P2 requesting read of address 25237 at time xxx:xxx */
                        fprintf(file, "Master: P%d requesting read of address %d at time %d : %d\n", i, pct->procs[i].requested, *clock_s, *clock_ns);
                        if (inFrameTbl(pct->procs[i].requested)){
                           /* if there is no page fault, oss just increments the clock by 10 nanoseconds and sends a signal on the corresponding semaphore */                        
                            increase_clock(10);
                            sem_signal(i);                      
                            memAccessCount++;  //increment memory access                         
                        }
                        else{
                        /* In case of page fault, oss queues the request to the device. Each request for disk read/write takes about 14ms to be fulfilled. */
                            printf("page fault detected!\n");
                            /* Master: Address 12345 is not in a frame, pagefault */
                            fprintf(file, "Master: Address %d is not in frame table, pagefault\n", pct->procs[i].requested);
                            increase_clock(14*TO_NANO);
                            /* Update PCT and frame table */
                            frametable[frameIndex].address = pct->procs[i].requested;
                            frametable[frameIndex].dirtyBit = 0;
                            frametable[frameIndex].procnum = i;
                            pct->procs[i].pageTable[pct->procs[i].pageIndex].address = pct->procs[i].requested;
                            pct->procs[i].pageTable[pct->procs[i].pageIndex].frame_num = frameIndex;
                            pct->procs[i].pageIndex += 1;
                            pct->procs[i].pageCount++;
                            increase_clock(14*TO_NANO);
                            pageFaultCount++; // increment page fault
                            frameIndex = (frameIndex + 1) % MAXMEM;// memory to be swapped out for FIFO 
                            sem_signal(i);
                        }
                    }
                    else{ // Write Type
                        fprintf(file, "Master: P%d requesting write of address %d at time %d : %d\n", i, pct->procs[i].requested, *clock_s, *clock_ns);
                        if (inFrameTbl(pct->procs[i].requested)){
                            /*Master: Address 12345 in frame 203, writing data to frame at time xxx:xxx*/
                            fprintf(file, "Master: Address %d in frame %d writing data to frame at time %d : %d\n", pct->procs[i].pageTable[pct->procs[i].pageIndex].frame_num, 
                            pct->procs[i].requested, *clock_s, *clock_ns);  
                            increase_clock(14*TO_NANO);    
                            /* if there is no page fault, oss just increments the clock by 10 nanoseconds and sends a signal on the corresponding semaphore */
                            memAccessCount++;
                            sem_signal(i);
                        }
                        else{ // page fault since not found in page table
                            /*Master: Address 12345 is not in a frame, pagefault*/
                            fprintf(file, "Master: Address %d is not in a frame, pagefault\n", pct->procs[i].requested);
                            increase_clock(14*TO_NANO);                          
                            pageFaultCount++;
                            frametable[frameIndex].procnum = i;
                            frametable[frameIndex].address = pct->procs[i].requested;
                            fprintf(file, "Master: Address %d in frame %d, writing data to frame at time %d : %d\n", pct->procs[i].requested, frameIndex, *clock_s, *clock_ns);                 
                            frametable[frameIndex].dirtyBit = 1;
                            fprintf(file, "Master: Dirty bit of frame %d set, adding additional time to the clock", frameIndex);                           
                            increase_clock(TO_NANO);
                            pct->procs[i].pageTable[pct->procs[i].pageIndex].address = pct->procs[i].requested;
                            pct->procs[i].pageTable[pct->procs[i].pageIndex].frame_num = frameIndex;
                            pct->procs[i].pageIndex += 1;
                            pct->procs[i].pageCount++;
                            increase_clock(14*TO_NANO);
                            memAccessCount++;
                            frameIndex = (frameIndex + 1) % MAXMEM;
                            sem_signal(i);
                        }
                    }
                
            }
        }
        sem_wait(18);
	increase_clock(14*TO_NANO);
        sem_signal(18);
        
        
       /*  print its memory map every second showing the allocation of frames */     
        if (((*clock_s * TO_NANO) + *clock_ns) > (trackNS + TO_NANO)) {    
               trackNS = (*clock_s * TO_NANO) + *clock_ns;
               increase_clock(100);
               memoryMap();
        }
    }

    return 0;
}

void increase_clock(long unsigned int inc){
        clock_ns[0] = clock_ns[0] + inc;//Increasing system clock
        while (clock_ns[0] >= TO_NANO) {//Coversion of nanosecond to second 
               clock_s[0] = clock_s[0] + 1;
               clock_ns[0] = clock_ns[0] - TO_NANO;
         }
}


void ReportStatistics(){
    fprintf(file, "\n## Statistics Report ##\n");
    fprintf(file, "Number of memory accesses per second %f\n", (float)memAccessCount / (float)*clock_s);
    fprintf(file, "Number of page faults per memory access %f\n", (float)pageFaultCount / (float)memAccessCount);
    fprintf(file, "Average memory access speed %f\n", (float)*clock_s / (float)memAccessCount);   
}

void memoryMap(){
   fprintf(file, "Current memory layout at time %d : %d is:\n", *clock_s, *clock_ns);
   fprintf(file, "\t\tOccupied\tDirty Bit\n");

    for (int i = 0; i < MAXMEM; i++)
    {
        fprintf(file, "Frame %d:\t %s \t %s\n", i, frametable[i].address == 0 ? "No" : "Yes", frametable[i].dirtyBit == true ? "1" : "0");
    }
    fputs("\n", file);
}

bool inFrameTbl(unsigned int add){
    for (int i = 0; i < MAXMEM; i++){
        if (frametable[i].address == add){
            return true;
        }
    }
    return false;
}

void cleanAll(){
    memoryMap();
    ReportStatistics();
    for (int p = 0; p < MAXPROC; p++) {
        wait(NULL);
    }      
    sleep(2);
    if (shmdt(clock_ns) == -1 || shmdt(clock_s) == -1 || shmdt(pct) == -1) {
      perror("OSS: Error: shmdt failed to detach memory");
      exit(EXIT_FAILURE);
     // abort();
    }
   /* if (shmctl(clock_nsid, IPC_RMID, 0) == -1 || shmctl(clock_sid, IPC_RMID, 0) == -1 || shmctl(pct_id, IPC_RMID, 0) == -1 || shmctl(sem_id, IPC_RMID, 0) == -1) {
      perror("OSS: Error: shmctl failed to delete shared memory");
     // abort();
    }*/ 
    
    shmctl(clock_nsid, IPC_RMID, NULL);
    shmctl(clock_sid, IPC_RMID, NULL);
    shmctl(pct_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID, NULL);   
    fputs("OSS: Aborting...\n", file);
    fclose(file);
    printf("OSS: Aborting..\n");
    abort();
}

/* Semaphore */
void sem_init(){
    if((sem_id = semget(ftok("oss.c", '4'), 20, IPC_CREAT | 0666)) == -1){
        perror("OSS: semget failed\n"); 
        exit(EXIT_FAILURE);  
    }
    semctl(sem_id, 19, SETVAL, 1);
    semctl(sem_id, 18, SETVAL, 1);

    for(int p = 0; p < MAXPROC; p++){
        semctl(sem_id, p, SETVAL, 1);
    }
}

void sem_signal(int sem){    
    struct sembuf sb = {sem, 1, 0}; //free resource
    if (semop(sem_id, &sb, 1) == -1) {
        perror("OSS: semop failed");
        exit(EXIT_FAILURE); 
    }
}

void sem_wait(int sem){    
    struct sembuf sb = {sem, -1, 0}; //allocate resource
    if (semop(sem_id, &sb, 1) == -1) {
        perror("OSS: semop failed");
        exit(EXIT_FAILURE); 
    }
}

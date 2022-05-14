/**
* @Author: Sanzida Hoque
* @Course: CMP_SCI 4760 Operating Systems 
* @Sources: https://t4tutorials.com/fifo-page-replacement-algorithm-memory-management-technique-operating-systems
            https://www.geeksforgeeks.org/page-replacement-algorithms-in-operating-systems/
            https://www.geeksforgeeks.org/program-page-replacement-algorithms-set-2-fifo/
            https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
*           
*/


#include "config.h"

unsigned int processScheduleNS = 0, trackNS = 0, frameIndex = 0, prevIndex = 0;
int forkedchild = 0, sysForks = 0, accessCount = 0, pageFaultCount = 0, totalFrame = 0;
bool replace = false, ref = false, pgfaultflag = true;

int main(int argc, char *argv[]){
    /* signal handling with cleanAll*/
    signal(SIGALRM, cleanAll); 
    signal(SIGINT, cleanAll); 
    signal(SIGTERM, cleanAll);
    struct Queue *memoryQ = createQueue(256);
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
    *clock_ns = 0;
    *clock_s = 0;
    
    
 
   /* Initializing PCT and Frame Table */
    for(int proc = 0; proc < MAXPROC; proc++){
        pct->inUse[proc] = 0;
        pct->pcb[proc].usedAll = false;
        pct->pcb[proc].requested = 0;
        pct->pcb[proc].pageIndex = 0;
        pct->pcb[proc].usedPage = 0;
        for(int pg = 0; pg < PROCMEM; pg++){
            pct->pcb[proc].pager[pg].frameNumber = -1;
            pct->pcb[proc].pager[pg].reqAdd = 0;
        }
    }
   
    for(int f = 0; f < CAPACITY; f++){
        frametable[f].pid = 0;
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
        // Empty if full
            if (pct->pcb[p].usedAll){
                semctl(sem_id, p, SETVAL, 1);
                pct->inUse[p] = 0;
                pct->pcb[p].pageIndex = 0;
                pct->pcb[i].usedPage = 0;
                pct->pcb[p].requested = 0;
                pct->pcb[p].reqtype = 0;
                pct->pcb[p].usedAll = false;
               for (int pg = 0; pg < PROCMEM; pg++){
                  if (pct->pcb[p].pager[pg].frameNumber != -1){
                     frametable[pct->pcb[p].pager[pg].frameNumber].pid = 0;
                     frametable[pct->pcb[p].pager[pg].frameNumber].address = 0;
                     frametable[pct->pcb[p].pager[pg].frameNumber].dirtyBit = 0;
                     frametable[pct->pcb[p].pager[pg].frameNumber].refBit = 0;
                  }
               }            
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
          cleanAll();
 	 }
        else{
           for (int p = 0; p < MAXPROC; p++){
               if(pct->inUse[p] == 0){
                  sysForks++;
                  childPid = fork();
                  if(childPid == 0){
                      execl("./process", "./process", NULL);            
                  }
                  else{                  
                      fprintf(file, "Master: P%d (pid %d) forked at %d : %d\n", p, childPid, *clock_s, *clock_ns); 
                      pct->inUse[p] = childPid;
                      processScheduleNS =  (*clock_s * TO_NANO) + *clock_ns + ((rand() % 500000000) + 1000000);   //random time for forking (1-500ms)
                      fprintf(file, "Master: Scheduled next fork after: %u ns\n", processScheduleNS);
                      forkedchild++;
                      sem_signal(19);
                  }
               }
            }
         }
      }
       
       //Check if replace or swap requires
       for (int i = 0; i < MAXPROC; i++){
        totalFrame += pct->pcb[i].usedPage;
       }
       if(totalFrame < CAPACITY){
           replace = false;
       }else{
           replace = true;
       }
       
       
        for (int i = 0; i < MAXPROC; i++){
            if (semctl(sem_id, i, GETVAL, 0) == 0){ //request in queue         
                for (int j = 0; j < CAPACITY; j++){
                   if(memoryQ->array[j] == pct->pcb[i].requested){
                         pgfaultflag = false;
                   }
                   else{
                         pgfaultflag = true;
                   }
                }
                    if (pct->pcb[i].reqtype == READ){ //Read type
                        /* Master: P2 requesting read of address 25237 at time xxx:xxx */
                        fprintf(file, "Master: P%d requesting read of address %d at time %d : %d\n", i, pct->pcb[i].requested, *clock_s, *clock_ns);
                        if (!pgfaultflag){
                           /* if there is no page fault, oss just increments the clock by 10 nanoseconds and sends a signal on the corresponding semaphore */                        
                            increase_clock(10);
                            sem_signal(i);                      
                            accessCount++;  //increment memory access                         
                        }
                        else{
                        /* In case of page fault, oss queues the request to the device. Each request for disk read/write takes about 14ms to be fulfilled. */
                            printf("page fault detected!\n");
                            /* Master: Address 12345 is not in a frame, pagefault */
                            fprintf(file, "Master: Address %d is not in frame table, pagefault\n", pct->pcb[i].requested);
                            increase_clock(14*TO_NANO);
                            if(isFull(memoryQ)){
                                for (int f = 0; f < CAPACITY; f++){
                                      dequeue(memoryQ);
   			           }
                            }
                            enqueue(memoryQ, pct->pcb[i].requested);
                            /* Update PCT and frame table */
                            frametable[frameIndex].pid = i;
                            frametable[frameIndex].address = pct->pcb[i].requested;
                            frametable[frameIndex].dirtyBit = 0;
                            frametable[frameIndex].refBit = 1;
                          
                            pct->pcb[i].pager[pct->pcb[i].pageIndex].reqAdd = pct->pcb[i].requested;
                            pct->pcb[i].pager[pct->pcb[i].pageIndex].frameNumber = frameIndex;
                            pct->pcb[i].pageIndex++;
                            pct->pcb[i].usedPage++;
                            increase_clock(14 * TO_NANO);
                            pageFaultCount++; // increment page fault
                            
                            sem_signal(i);
                            frameIndex = (frameIndex + 1) % CAPACITY;//next frame
                        }
                    }
                    else{ // Write Type
                        fprintf(file, "Master: P%d requesting write of address %d at time %d : %d\n", i, pct->pcb[i].requested, *clock_s, *clock_ns);
                        
                        
                        if (!pgfaultflag){
                            /*Master: Address 12345 in frame 203, writing data to frame at time xxx:xxx*/
                            fprintf(file, "Master: Address %d in frame %d writing data to frame at time %d : %d\n", pct->pcb[i].pager[pct->pcb[i].pageIndex].frameNumber, 
                            pct->pcb[i].requested, *clock_s, *clock_ns);    
                            /* if there is no page fault, oss just increments the clock by 10 nanoseconds and sends a signal on the corresponding semaphore */
                            increase_clock(10 * TO_NANO);  
                            accessCount++;
                            sem_signal(i);
                        }
                        else{ // page fault
                            /*Master: Address 12345 is not in a frame, pagefault*/
                            fprintf(file, "Master: Address %d is not in a frame, pagefault\n", pct->pcb[i].requested);                       
                            pageFaultCount++;
                            
                            if(isFull(memoryQ)){
                                for (int f = 0; f < CAPACITY; f++){
                                      dequeue(memoryQ);
   			           }
                            }
                            
                            frametable[frameIndex].pid = i;
                            frametable[frameIndex].address = pct->pcb[i].requested;
                            enqueue(memoryQ, pct->pcb[i].requested);
                            fprintf(file, "Master: Address %d in frame %d, writing data to frame at time %d : %d\n", pct->pcb[i].requested, frameIndex, *clock_s, *clock_ns);                 
                            frametable[frameIndex].dirtyBit = 1;
                            frametable[frameIndex].refBit = 1;
                            
                            fprintf(file, "Master: Dirty bit of frame %d set, adding additional time to the clock", frameIndex);                           
                            increase_clock(14 * TO_NANO);
                            
                            pct->pcb[i].pager[pct->pcb[i].pageIndex].reqAdd = pct->pcb[i].requested;
                            pct->pcb[i].pager[pct->pcb[i].pageIndex].frameNumber = frameIndex;
                            pct->pcb[i].pageIndex++;
                            pct->pcb[i].usedPage++;
                            accessCount++;
                            sem_signal(i);
                            frameIndex = (frameIndex + 1) % CAPACITY;//next frame
                                                       
                        }
                    }
                
            }
        }
        sem_wait(18);
	increase_clock(14 * TO_NANO);
        sem_signal(18);
        if(frametable[frameIndex].refBit == 1){
            prevIndex = frameIndex;
        }
        
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
    fprintf(file, "Number of memory accesses per second %f\n", (float)accessCount / (float)*clock_s);
    fprintf(file, "Number of page faults per memory access %f\n", (float)pageFaultCount / (float)accessCount);
    fprintf(file, "Average memory access speed %f\n", (float)*clock_s / (float)accessCount);    
}

void memoryMap(){
   fprintf(file, "Current memory layout at time %d : %d is:\n", *clock_s, *clock_ns);
   fprintf(file, "\t\tOccupied\tDirty Bit\n");

    for (int i = 0; i < CAPACITY; i++)
    {
        fprintf(file, "Frame %d:\t %s \t %s\n", i, frametable[i].address == 0 ? "No" : "Yes", frametable[i].dirtyBit == true ? "1" : "0");
    }
    fputs("\n", file);
}

bool checkFrame(unsigned int add){
    for (int i = 0; i < CAPACITY; i++){        
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
     // exit(EXIT_FAILURE);
      //abort();
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
       // exit(EXIT_FAILURE);
       abort();  
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
        abort();
        //exit(EXIT_FAILURE); 
    }
}

void sem_wait(int sem){    
    struct sembuf sb = {sem, -1, 0}; //allocate resource
    if (semop(sem_id, &sb, 1) == -1) {
        perror("OSS: semop failed");
        abort();
        //exit(EXIT_FAILURE); 
    }
}

/* Queue */
struct Queue* createQueue(unsigned capacity){
    struct Queue* queue = (struct Queue*)malloc(
    sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
    queue->capacity * sizeof(int));
    return queue;
}
 
int isFull(struct Queue* queue){
    return (queue->size == queue->capacity);
}
 

int isEmpty(struct Queue* queue){
    return (queue->size == 0);
}
 
void enqueue(struct Queue* queue, int item){
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    printf("%d enqueued to memory queue\n", item);
}
 
int dequeue(struct Queue* queue){
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 

int front(struct Queue* queue){
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
 

int rear(struct Queue* queue){
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}
 

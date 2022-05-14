/*libraries*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<getopt.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <limits.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdbool.h>

/* macros */
#define MAXPROC 18
#define MAXRESOURCE 10
#define TO_NANO 1000000000UL
#define timeslice 100000
#define PROCMEM 32
#define CAPACITY 256
#define READ 1
#define WRITE 2

/* DS */

union semun {
    int val;                /* value for SETVAL */
    struct semid_ds *buf;   /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;  /* array for GETALL & SETALL */
    struct seminfo *__buf;  /* buffer for IPC_INFO */
};

union semun arg;

struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};
 

struct Frame{ // Physical memory
   bool dirtyBit;// switches on when an update is made to a page/modified
   bool refBit;//  whether this page has been referred in the last clock cycle or not
   //bool presentBit;//particular page you are looking for is present or absent. 
   int pid;
   int address;    
};

struct Page{ // Virtual Memory
    int reqAdd;
    int frameNumber;// frame number in which the current page you are looking for is present 
};

struct PCB{//Proc
   bool usedAll;
   int requested;
   int reqtype;
   int pageIndex; 
   int usedPage; 
   struct Page pager[PROCMEM];
};

struct PCT{
   struct PCB pcb[MAXPROC];
   int inUse[MAXPROC];
};

struct PCT *pct;
struct Frame frametable[CAPACITY];
/* shm variables */

//System clock
unsigned int *clock_s;
unsigned int *clock_ns;
int clock_sid, clock_nsid, pct_id, sem_id;

FILE *cstest = NULL, *logfile = NULL, *file = NULL;
int *allpid2 = NULL, *parentpid = NULL, *childpid = NULL;
int childPid;
int availableResource[10], work[10];
bool finishedProc[MAXPROC];
bool doSim = true;

/* functions */   
void memoryMap();
bool checkFrame(unsigned int);
void sem_init();
void cleanAll();
void ReportStatistics();
void increase_clock(long unsigned int inc);
void set_sem();
void sem_signal(int);
void sem_wait(int);
struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);
int front(struct Queue* queue);
int rear(struct Queue* queue);

/* signalhandlers */
void signal_timer(int signal);
void signal_abort();

/* iterators */
int i, j, c; 


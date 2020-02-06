/*
Project 1
Andreas Gagas
main.c - discrete event simulator 
*/

#include <stdio.h>
#include <stdlib.h>
#include "queue.c" // queue implementation as linkedlist of nodes ... 
// structs: node struct and queue struct 
// functions for queue: Queue* initQueue(), void enqueue(Queue *queue, struct event *data), struct event* dequeue(Queue *queue), int isEmpty(Queue *queue)
// functions for priortiy queue: pushPQ(Queue *queue, struct event *e), struct event* popPQ(Queue *queue, struct event *e)

//void readConfigFile(); // const values based on thoses read from the configuration file 
int random_num_between_interval(int min, int max);

void handle_start_simulation(Queue *pQ); 
void handle_end_simulation(Queue *pQ);
void handle_process_arrival(Queue *pQ, Queue *cpuQueue, struct event *old); 
void hand_process_arrive_cpu(Queue *pQ, struct event *old);

 // global variables (for simplicity)
int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, CPU_MIN, CPU_MAX,
DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX; 
double QUIT_PROB, NETWORK_PROB; 

int id_count; // testing this out 

// 1 for T ex. SYSTEM_EXIT 0 --> FALSE

// typedef enum {START_SIM, PROCESS_ARRIVAL, PROCESS_ARRIVE_CPU, PROCESS_EXIT_CPU, PROCESS_EXIT_SYSTEM, 
// //                0             1                 2                  3                 4
// PROCESS_ARRIVE_DISK1, PROCESS_EXIT_DISK1, PROCESS_ARRIVE_DISK2, PROCESS_EXIT_DISK2,
// //      5                       6                 7                      8                        
// PROCESS_ARRIVE_NETWORK, PROCESS_EXIT_NETWORK, END_SIM} TYPES;
// //     9                        10                11

// struct event{
// // represent an event 

//   int id; // unique processID (each systemArrival is counted)
//   TYPES type; // event type, defined above 
//   int time; // length of job, determined by randon num. between const min and max of type 
// };

int main(int args, char *argv[]){
  
  // TEMP FIX FORMAT LATER 
  // const values (make const later), generate random num between their interval
  // int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, CPU_MIN, CPU_MAX, 
  // DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX,NETWORK_MIN, NETWORK_MAX; 
  // double QUIT_PROB, NETWORK_PROB; 

  FILE *fptr = NULL;
  fptr = fopen("CONFIG1.conf", "r"); // change to command line input later 

  if(fptr == NULL)
    puts("main.c: could not open file");
  
  // read the int value from file (follows this format), store in corresponding event variable 
  fscanf(fptr, "%*s %d", &SEED);
  fscanf(fptr, "%*s %d", &INIT_TIME);
  fscanf(fptr, "%*s %d", &FIN_TIME);
  fscanf(fptr, "%*s %d", &ARRIVE_MIN);
  fscanf(fptr, "%*s %d", &ARRIVE_MAX);
  fscanf(fptr, "%*s %lf", &QUIT_PROB);
  fscanf(fptr, "%*s %lf", &NETWORK_PROB);
  fscanf(fptr, "%*s %d", &CPU_MIN);
  fscanf(fptr, "%*s %d", &CPU_MAX);
  fscanf(fptr, "%*s %d", &DISK1_MIN);
  fscanf(fptr, "%*s %d", &DISK1_MAX);
  fscanf(fptr, "%*s %d", &DISK2_MIN);
  fscanf(fptr, "%*s %d", &DISK2_MAX);
  fscanf(fptr, "%*s %d", &NETWORK_MIN);
  fscanf(fptr, "%*s %d", &NETWORK_MAX);

  // printf("%d\n", FIN_TIME); 

  fclose(fptr);

  srand(SEED); // seed is from config file 


  Queue *cpuQueue;
  cpuQueue = initQueue();               // ***** cpuQueue->status 1 for BUSY 0 for IDLE *****
  //Queue *disk1Queue;
  //disk1Queue = initQueue(); 
  //Queue *disk2Queue;
  //disk2Queue = initQueue();
  //Queue *networkQueue;
  //networkQueue = initQueue();
  Queue *pQueue;
  pQueue = initQueue(); 


  //int processIDCount = 0; // systemArrival counter for unique processID PUT IN handler 



  // INIT 
  handle_start_simulation(pQueue); // push START_SIM event onto pQueue, generate first job arrival 
  handle_end_simulation(pQueue); // push END_SIM event onto pQueue (always lowest priority)
  id_count = 0; 
  // START_SIM -> PROCESS_ARRIVAL -> END_SIM ... start main loop 

  int i = 0; 
  // MAIN LOOP 
  while(i<1){

    // grab the top priority event 
    struct event *e; 
    e = malloc(sizeof(struct event)); 
    e = popPQ(pQueue); 

    switch(e->type)
    {
        case PROCESS_ARRIVAL:
          handle_process_arrival(pQueue, cpuQueue, e); 
          break; 
        

    }

    ++i; 
    
  }


  // main loop to call handler func (use switch case to select correct event generator/handler)
    // clock set to next event time       clock is now: ... 
    // poll and carry out highest priority event ... 

      // ************ 5 iterations a loop 

    // isBusy() change status 

  
  // *********** main loop . pop event --> switch case correct event handler  
  struct node *n; 
  n = malloc(sizeof(struct node)); 
  n = pQueue->front; 

  // print queue 
  for(n = pQueue->front; n != 0; n = n->next) {
      printf("%d ", n->e->type);
  }

  //printf("%d", pQueue->front->e->type); 

  return 0; 
}

int random_num_between_interval(int min, int max){
  return (min + rand() % (max-min+1)); // [min-max]
}

// handler func. for each event type (will actually change state of system)
// START_SIM, SYSTEM_ARRIVAL, CPU_START, CPU_END, SYSTEM_EXIT, DISK1_ARRIVAL, DISK1_END, DISK2_ARRIVAL, DISK2_END, NETWORK_ARRIVAL, 
// NETWORK_END, END_SIM


void handle_start_simulation(Queue *pQ){
  // will generate once 

  struct event *e; 
  e = malloc(sizeof(struct event)); 
  e->type = START_SIM; 
  e->time = INIT_TIME; 
  e->id = 0; // doesn't need an id, prob. delete  

  // ADD TO PRIORITY QUEUE ... THIS IS FIRST EVENT 
  pushPQ(pQ, e);

  struct event *new; 
  new = malloc(sizeof(struct event)); 
  new->type = PROCESS_ARRIVAL; 
  new->time = id_count;  // 0  
  new->id = 1;  

  // ADD first job arrival (time 0) to queue, get the loop going ... 
  pushPQ(pQ, new); 
}

void handle_end_simulation(Queue *pQ){
  
  struct event *e;
  e = malloc(sizeof(struct event)); 
  e->type = END_SIM; 
  e->time = FIN_TIME; 
  e->id = 0; // doesnt need an id, prob. delete

  // ADD TO PRIORITY QUEUE ... SHOULD BE BOTTOM THROUGHOUT 
  pushPQ(pQ, e); 
}

void handle_process_arrival(Queue *pQ, Queue *cpuQueue, struct event *old){
  // will generate itself 


  if(cpuQueue->status == 1 || (cpuQueue->counter >0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQueue, old); 
  }
  else if(cpuQueue->status == 0 && cpuQueue->counter == 0){
    // no cpuQueue required, generate NEXT event (PROCESS_ARRIVE_CPU)

    struct event *next; 
    next = malloc(sizeof(struct event));
    next->type = PROCESS_ARRIVE_CPU;
    next->id = id_count; // id_count is still old event id
    next->time = old->time; 

    cpuQueue->status = 1; // CPU NOW BUSY 
  }

  // NEW EVENT ARRIVAL 
  ++id_count; // increment count, each func call, this is the processID 
  int num = random_num_between_interval(ARRIVE_MIN, ARRIVE_MAX); 

  struct event *new; 
  new = malloc(sizeof(struct event)); 
  new->type = PROCESS_ARRIVAL; 
  new->id = id_count; 
  new->time = old->time + num; // new event time 

  pushPQ(pQ, new); // add to PQ 
}

void hand_process_arrive_cpu(Queue *pQ, struct event *old){
  // will generate PROCESS_FINISH_CPU event 

  int num = random_num_between_interval(CPU_MIN, CPU_MAX); 



}



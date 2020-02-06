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
int handle_end_simulation(Queue *pQ, struct event *old);
void handle_process_arrival(Queue *pQ, Queue *cpuQueue, struct event *old); 
void handle_process_arrive_cpu(Queue *pQ, struct event *old);
void handle_process_exit_cpu(Queue *pQ, Queue *cpuQ, Queue *networkQ, Queue *disk1Q, Queue *disk2Q, struct event *old); // has the most going for it 
void handle_process_exit_system(Queue *pQ, struct event *old); 
void handle_process_arrive_disk1(Queue *pQ, Queue *disk1Q, struct event *old); 


 // global variables (for simplicity)
int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, CPU_MIN, CPU_MAX,
DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX; 
double QUIT_PROB, NETWORK_PROB; 

int id_count; // testing this out 


// typedef enum {START_SIM, PROCESS_ARRIVAL, PROCESS_ARRIVE_CPU, PROCESS_EXIT_CPU, PROCESS_EXIT_SYSTEM, 
// //                0             1                 2                  3                 4
// PROCESS_ARRIVE_DISK1, PROCESS_EXIT_DISK1, PROCESS_ARRIVE_DISK2, PROCESS_EXIT_DISK2,
// //      5                       6                 7                      8                        
// PROCESS_ARRIVE_NETWORK, PROCESS_EXIT_NETWORK, END_SIM} TYPES;
// //     9                        10                11


// for reference, corresponse to enum TYPES value 
char *event_types[12] = {"start sim", "process arrival", "process arrives cpu", "process exits cpu", "process exits SYSTEM", 
"process arrives disk1", "process exits disk1", "process arrives disk2", "process exits disk2", "process arrives network", "process exits network", 
"end sim"}; 


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
  Queue *disk1Queue;
  disk1Queue = initQueue(); 
  Queue *disk2Queue;
  disk2Queue = initQueue();
  Queue *networkQueue;
  networkQueue = initQueue();
  Queue *pQueue;
  pQueue = initQueue(); 


  // INIT 
  handle_start_simulation(pQueue); // push START_SIM event and END_SIM onto pQueue, generate first job arrival 
  id_count = 0; 
  // START_SIM -> PROCESS_ARRIVAL -> END_SIM ... start main loop 

  int i = 1; 
  int control = 1; 
  // MAIN LOOP 
  //while(pQueue->front != 0){
  //while(control){
    // loop will end when top priority event is END_SIM (whose event time is equal to FIN_TIME)
  while(i<2){

    // grab the top priority event 
    struct event *e; 
    e = malloc(sizeof(struct event)); 
    e = popPQ(pQueue); 

    switch(e->type)
    {
        case PROCESS_ARRIVAL:
          handle_process_arrival(pQueue, cpuQueue, e); 
          break; 
        
        case PROCESS_ARRIVE_CPU:
          handle_process_arrive_cpu(pQueue, e);
          break; 
        
        case PROCESS_EXIT_CPU:
          handle_process_exit_cpu(pQueue, cpuQueue, networkQueue, disk1Queue, disk2Queue, e); 
          break; 
        
        case PROCESS_EXIT_SYSTEM:
          handle_process_exit_system(pQueue, e); 



        case END_SIM: 
          control = handle_end_simulation(pQueue, e); // control set to 0, exit loop 
          break;  

    }

    ++i; 
    
  }


  struct node *n; 
  n = malloc(sizeof(struct node)); 
  n = pQueue->front; 

  // print queue 
  for(n = pQueue->front; n != 0; n = n->next) {
      printf("%d ", n->e->type);
  }

  //printf("%d", pQueue->front->e->type);

  // free(pQueue) ...  (struct nodes freed inside func calls in queue.c, and struct events freed inside handlers)

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

  struct event *start; 
  start = malloc(sizeof(struct event)); 
  start->type = START_SIM; 
  start->time = INIT_TIME; 
  start->id = 0; // doesn't need an id, prob. delete  

  struct event *end;
  end = malloc(sizeof(struct event)); 
  end->type = END_SIM; 
  end->time = FIN_TIME; 
  end->id = 0; // doesnt need an id, prob. delete

  pushPQ(pQ, start); // THIS IS FIRST EVENT 
  pushPQ(pQ, end);  // ADD TO PRIORITY QUEUE ... SHOULD BE BOTTOM THROUGHOUT 

  struct event *new; 
  new = malloc(sizeof(struct event)); 
  new->type = PROCESS_ARRIVAL; 
  new->time = id_count;  // 0  
  new->id = 1;  

  // ADD first job arrival (time 0) to queue, get the loop going ... 
  pushPQ(pQ, new); 

}

int handle_end_simulation(Queue *pQ, struct event *old){
  // end the simulation b/c old->time == FIN_TIME
  
  // just simply exits the loop then ....
  return 0; 

  // struct event *e;
  // e = malloc(sizeof(struct event)); 
  // e->type = END_SIM; 
  // e->time = FIN_TIME; 
  // e->id = 0; // doesnt need an id, prob. delete

  // // ADD TO PRIORITY QUEUE ... SHOULD BE BOTTOM THROUGHOUT 
  // pushPQ(pQ, e); 
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
    //next->id = id_count; // id_count is still old event id
    next->id = old->id; 
    next->time = old->time; // same time 

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

  // case to free up old event, but if enqueued 
  // free(old) 
}

void handle_process_arrive_cpu(Queue *pQ, struct event *old){
  // will generate PROCESS_FINISH_CPU event 

  int num = random_num_between_interval(CPU_MIN, CPU_MAX); 
  struct event *new; 
  new = malloc(sizeof(struct event)); 
  new->type = PROCESS_EXIT_CPU;  
  new->id = old->id; 
  new->time = old->time + num; // new event time 

  // free(old); 
}

void handle_process_exit_cpu(Queue *pQ, Queue *cpuQ, Queue *networkQ, Queue *disk1Q, Queue *disk2Q, struct event *old){
  // will generate PROCESS_EXIT_SYSTEM

  double random = random_num_between_interval(0, 1); // random double between [0-1]

  if(random < QUIT_PROB){
    // generate PROCESS_EXIT_SYSTEM event 

    struct event *new_exit; 
    new_exit = malloc(sizeof(struct event)); 
    new_exit->type = PROCESS_EXIT_SYSTEM;
    new_exit->id = old->id; 
    new_exit->time = old->time; // // exits happens right away ?? 
    pushPQ(pQ, new_exit); // will just free up struct event old 
  }

  else if(random < NETWORK_PROB){
    // go to Network instead of disk 

    if(networkQ->status == 0 && networkQ->counter == 0){
      // Network is idle AND Network queue is empty 
      // generate PROCESS_ARRIVE_NETWORK event 

      int num = random_num_between_interval(NETWORK_MIN, NETWORK_MAX); 

      struct event *new_arrival; 
      new_arrival = malloc(sizeof(struct event)); 
      new_arrival->type = PROCESS_ARRIVE_NETWORK; 
      new_arrival->id = old->id; 
     // new_arrival->time = old->time;
      new_arrival->time = old->time + num; 

      pushPQ(pQ, new_arrival); 
    }

    else{
      // Network is busy || Network queue is non-empty ...  add to networkQeueue 

      enqueue(networkQ, old); 
    }
  }

  // GO TO DISK DIRECTLY 
  else if(disk1Q->status == 0 && disk1Q->counter == 0){
     // disk queue is empty and disk is idle .. generate new event 
    
    int num = random_num_between_interval(DISK1_MIN, DISK1_MAX); 

    struct event *new_arrival;
    new_arrival = malloc(sizeof(struct event));
    new_arrival->type = PROCESS_ARRIVE_DISK1; 
    new_arrival->id = old->id; 
    new_arrival->time = old->time + num; 

    pushPQ(pQ, new_arrival); 
    disk1Q->status = 1; 
  }

  else if(disk2Q->status == 0 && disk2Q->counter == 0){
    // disk queue is empty and disk is idle .. generate new event 
    
    int num = random_num_between_interval(DISK2_MIN, DISK2_MAX); 

    struct event *new_arrival;
    new_arrival = malloc(sizeof(struct event));
    new_arrival->type = PROCESS_ARRIVE_DISK2; 
    new_arrival->id = old->id; 
    new_arrival->time = old->time + num; 

    pushPQ(pQ, new_arrival); 
    disk2Q->status = 1; 
  }

  else{
    // push old event to SMALLER disk queue 

    if(disk1Q->counter > disk2Q->counter)
      enqueue(disk2Q, old); 

    else if(disk1Q->counter == disk2Q->counter){
      // queues are equal in size, generate a random num to pick a queue 

      int num = random_num_between_interval(1, 10); 

      if(num > 5)
        enqueue(disk2Q, old); 
      else
        enqueue(disk1Q, old); 
    }

    else
      enqueue(disk1Q, old); 
  }

  // AT THIS POINT the old event (PROCESS_EXIT_CPU) is handled, the CPU IS NOW OPEN TO THOSE IN CPUQUEUE 

  // ****** do this for other end events (eliminate the need to create a new event type) ***********
  if(cpuQ->counter > 0){

    struct event *new; 
    new = malloc(sizeof(struct event)); 
    new = dequeue(cpuQ); 
    new->type = PROCESS_ARRIVE_CPU; // change type from PROCESS_ARRIVAL to ... 
    // id is the same
    new->time = old->time; 
    pushPQ(pQ, new); 

    cpuQ->status = 1; // busy 
  }

  // free(old) but only if not on queue FIX LATER 
}

void handle_process_exit_system(Queue *pQ, struct event *old){
  // free old struct event, dont generate any new events 

  free(old);
}

void handle_process_arrive_disk1(Queue *pQ, Queue *disk1Q, struct event *old){


}



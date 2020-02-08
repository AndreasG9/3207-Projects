/*
Project 1
Andreas Gagas
main.c - discrete event simulator 
*/


// AT END, have 0 memory leaks (free all Queues, nodes freed when dequeued / popped, free all old events as new the new event is generated, UNLESS QUEUED) ****


#include <stdio.h>
#include <stdlib.h>
#include "queue.c" // queue implementation as linkedlist of nodes ... 
// structs: node struct and queue struct 
// functions for queue: Queue* initQueue(), void enqueue(Queue *queue, struct event *data), struct event* dequeue(Queue *queue), int isEmpty(Queue *queue)
// functions for priortiy queue: pushPQ(Queue *queue, struct event *e), struct event* popPQ(Queue *queue, struct event *e)

void read_config_file();
int random_num_between_interval(int min, int max);

void init_simulation(Queue *pQ); 
void handle_start_simulation(Queue *pQ, struct event *old); 
int handle_end_simulation(Queue *pQ, struct event *old);
void handle_process_arrival(Queue *pQ, Queue *cpuQueue, struct event *old); 
void handle_process_arrive_cpu(Queue *pQ, struct event *old);
void handle_process_exit_cpu(Queue *pQ, Queue *cpuQ, Queue *networkQ, Queue *disk1Q, Queue *disk2Q, struct event *old); // has the most going for it 
void handle_process_exit_system(Queue *pQ, struct event *old); 
void handle_process_arrive_disk1(Queue *pQ, Queue *disk1Q, struct event *old); 
void handle_process_exit_disk1(Queue *pQ, Queue *cpuQ, Queue *disk1Q, struct event *old); 
void handle_process_arrive_disk2(Queue *pQ, Queue *disk2Q, struct event *old);
void handle_process_exit_disk2(Queue *pQ, Queue *cpuQ, Queue *disk2Q, struct event *old); 
void handle_process_arrive_network(Queue *pQ, Queue *networkQ, struct event *old); 
void handle_process_exit_network(Queue *pQ, Queue *cpuQ, Queue *networkQ, struct event *old); 

 // global variables (for simplicity)
int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, CPU_MIN, CPU_MAX,
DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX; 
double QUIT_PROB, NETWORK_PROB; 
int id_count; // testing this out 

// ==================================== EVENT TYPES (in queue.c)=======================================
// typedef enum {START_SIM, PROCESS_ARRIVAL, PROCESS_ARRIVE_CPU, PROCESS_EXIT_CPU, PROCESS_EXIT_SYSTEM, 
// //                0             1                 2                  3                 4
// PROCESS_ARRIVE_DISK1, PROCESS_EXIT_DISK1, PROCESS_ARRIVE_DISK2, PROCESS_EXIT_DISK2,
// //      5                       6                 7                      8                        
// PROCESS_ARRIVE_NETWORK, PROCESS_EXIT_NETWORK, END_SIM} TYPES;
// //     9                        10                11


// for reference, corresponds to enum TYPES value 
char *event_types[12] = {"simulations starts", "arrives", "arrives at cpu", " exits the cpu", "exits the SYSTEM", 
"arrives at disk1", "exits the disk1", "arrives at disk2", "exits the disk2", "arrives at network", "exits the network", 
"simulation"}; 


int main(int args, char *argv[]){

  read_config_file(); // const values (in name) based on thoses read from the configuration file 
  srand(SEED); // seed is from config file 

  // INIT 
  Queue *cpuQueue;
  cpuQueue = initQueue();               
  Queue *disk1Queue;
  disk1Queue = initQueue(); 
  Queue *disk2Queue;
  disk2Queue = initQueue();
  Queue *networkQueue;
  networkQueue = initQueue();
  Queue *pQueue;
  pQueue = initQueue(); 

  init_simulation(pQueue); // push START_SIM event and END_SIM onto pQueue, when START_SIM popped, generate FIRST process arrival 
  id_count = 1; 


  int i = 1; 
  int control = 1; 
  // MAIN LOOP 
  //while(pQueue->front != 0){
 // while(control){
    // loop will end when top priority event is END_SIM (whose event time is equal to FIN_TIME)
  while(i<5){

    // grab the top priority event 
    struct event *current; 
    current = malloc(sizeof(struct event)); 
    current = popPQ(pQueue); 

    switch(current->type)
    {
        case START_SIM:
          //printf("\nSTART_SIM PULLED\n"); 
          handle_start_simulation(pQueue, current); // called once, will generate first PROCESS_ARRIVAL
          break; 

        case PROCESS_ARRIVAL:
        //puts("HANDLE PROCESS ARRIVAL POPPED"); 
          handle_process_arrival(pQueue, cpuQueue, current); 
          break; 
        
        case PROCESS_ARRIVE_CPU:
          handle_process_arrive_cpu(pQueue, current);
          break; 
        
        case PROCESS_EXIT_CPU:
          handle_process_exit_cpu(pQueue, cpuQueue, networkQueue, disk1Queue, disk2Queue, current); 
          break; 
        
        case PROCESS_EXIT_SYSTEM:
          handle_process_exit_system(pQueue, current); 
          break; 
        
        case PROCESS_ARRIVE_DISK1:
          handle_process_arrive_disk1(pQueue, disk1Queue, current); 
          break; 

        case PROCESS_EXIT_DISK1:
          handle_process_exit_disk1(pQueue, cpuQueue, disk1Queue, current); 
          break; 

        case PROCESS_ARRIVE_DISK2:
          handle_process_arrive_disk2(pQueue, disk2Queue, current); 
          break; 
        
        case PROCESS_EXIT_DISK2:
          handle_process_exit_disk2(pQueue, cpuQueue, disk2Queue, current); 
          break; 
        
        case PROCESS_ARRIVE_NETWORK:
          handle_process_arrive_network(pQueue, networkQueue, current);
          break;
        
        case PROCESS_EXIT_NETWORK:
          handle_process_exit_network(pQueue, cpuQueue, networkQueue, current); 
          break; 

        case END_SIM: 
          control = handle_end_simulation(pQueue, current); // control set to 0, exit loop 
          break;  

    }

    ++i; 
    
  }


  struct node *n; 
  n = malloc(sizeof(struct node)); 
  n = pQueue->front; 

  // print queue 
  for(n = pQueue->front; n != 0; n = n->next) {
      //printf("%d ", n->e->time);
  }

  //printf("\ncpu queue:%d \n", cpuQueue->front->e->type); 

  //printf("%d", pQueue->front->e->type);

  // free(pQueue) ...  (struct nodes freed inside func calls in queue.c, and struct events freed inside handlers)

  // TESTING 
  struct event *e; 
  e = malloc(sizeof(struct event));
  e->type = PROCESS_ARRIVE_CPU;
  e->id = 50;
  e->time = 40; 

 // handle_process_exit_cpu(pQueue, cpuQueue, networkQueue, disk1Queue, disk2Queue, e); 

  return 0; 
}

void read_config_file(){
  
  FILE *fptr = NULL;
  fptr = fopen("constants.CONF", "r"); // change to command line input later 

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
}

int random_num_between_interval(int min, int max){
  return (min + rand() % (max-min+1)); // [min-max]
}

// handler func. for each event type (will actually change state of system)
// START_SIM, SYSTEM_ARRIVAL, CPU_START, CPU_END, SYSTEM_EXIT, DISK1_ARRIVAL, DISK1_END, DISK2_ARRIVAL, DISK2_END, NETWORK_ARRIVAL, 
// NETWORK_END, END_SIM

void init_simulation(Queue *pQ){
  // push START_SIM and FIN_TIME onto pQueue
  // START_SIM will be the first event popped 

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
}

void handle_start_simulation(Queue *pQ, struct event *old){
  // will generate once, popping the START_SIM event will generate a PROCESS_ARRIVAL, thus starting the sim 

  printf("At time %d, %s.\n", old->time, event_types[old->type]);

  struct event *new_process_arrival; 
  new_process_arrival = malloc(sizeof(struct event)); 
  new_process_arrival->type = PROCESS_ARRIVAL; 
  new_process_arrival->time = INIT_TIME; // 0  
  new_process_arrival->id = id_count; // 0 

  // ADD first job arrival (time 0) to queue, get the loop going ... 
  pushPQ(pQ, new_process_arrival); 
}

int handle_end_simulation(Queue *pQ, struct event *old){
  // end the simulation b/c old->time == FIN_TIME

  // finish writing to log
  // write final stat to STAT 
  // close file ptrs 
  // all that is left to free is the current old event, and the queues (everything else freed earlier)
  // print END message 
  
  // just simply exits the loop then ....
  return 0; 
}

void handle_process_arrival(Queue *pQ, Queue *cpuQueue, struct event *old){
  // will generate itself 

  printf("At time %d, process %d, %s.\n", old->time, old->id, event_types[old->type]); 

  int dont_free = 0; 

  if(cpuQueue->status == 1 || (cpuQueue->counter > 0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQueue, old); 
    dont_free = 1; // dont free the old event 
  }

  else if(cpuQueue->status == 0 && cpuQueue->counter == 0){
    // no cpuQueue required, generate NEXT event (PROCESS_ARRIVE_CPU)
    //printf("\nNO QUEUE\n"); 

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event));
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU;
    //new_arrive_cpu->id = id_count; // id_count is still old event id
    new_arrive_cpu->id = old->id; 
    new_arrive_cpu->time = old->time; // same time 

    pushPQ(pQ, new_arrive_cpu); 
    cpuQueue->status = 1; // CPU NOW BUSY 
  }

  // NEW EVENT ARRIVAL 
  ++id_count; // increment count, each func call, this is the processID 
  int num = random_num_between_interval(ARRIVE_MIN, ARRIVE_MAX); 

  struct event *new_arrival; 
  new_arrival = malloc(sizeof(struct event)); 
  new_arrival->type = PROCESS_ARRIVAL; 
  new_arrival->id = id_count; 
  //printf("\nNEW PROCESS_ARRIVAL ID: %d\n", id_count); 
  new_arrival->time = old->time + num; // new event time 
  //printf("NEW PROCESS_ARRIVAL TIME: %d\n", new_arrival->time); 

  pushPQ(pQ, new_arrival); // add to PQ 
  
  // case to free up old event, but if NOT IN QUEUE 
 if(dont_free == 0)
    free(old); 
}

void handle_process_arrive_cpu(Queue *pQ, struct event *old){
  // will generate PROCESS_EXIT_CPU event 

  printf("At time %d, process %d, %s\n", old->time, old->id, event_types[old->type]); 

  int num = random_num_between_interval(CPU_MIN, CPU_MAX); 

  struct event *new_exit_cpu; 
  new_exit_cpu = malloc(sizeof(struct event)); 
  new_exit_cpu->type = PROCESS_EXIT_CPU;  
  new_exit_cpu->id = old->id; 
  new_exit_cpu->time = old->time + num; // new event time 
  pushPQ(pQ, new_exit_cpu); 

  free(old); 
}

void handle_process_exit_cpu(Queue *pQ, Queue *cpuQ, Queue *networkQ, Queue *disk1Q, Queue *disk2Q, struct event *old){
  // will generate PROCESS_EXIT_SYSTEM

  printf("At time %d, process %d, %s\n", old->time, old->id, event_types[old->type]); 

  cpuQ->status = 0; // CPU now idle 
  int dont_free = 0; // free old event is NOT IN QUEUE 

  //double random = random_num_between_interval(0, 1); // random double between [0-1]

  if((random_num_between_interval(0,1)) < QUIT_PROB){
    // generate PROCESS_EXIT_SYSTEM event 

    struct event *new_exit_system; 
    new_exit_system = malloc(sizeof(struct event)); 
    new_exit_system->type = PROCESS_EXIT_SYSTEM;
    new_exit_system->id = old->id; 
    new_exit_system->time = old->time; // same time 
    pushPQ(pQ, new_exit_system);
    
  }

  else if((random_num_between_interval(0,1)) < NETWORK_PROB){
    // go to Network instead of disk 

    if(networkQ->status == 0 && networkQ->counter == 0){
      // Network is idle AND Network queue is empty 
      // generate PROCESS_ARRIVE_NETWORK event 

      int num = random_num_between_interval(NETWORK_MIN, NETWORK_MAX); 

      struct event *new_arrive_network; 
      new_arrive_network = malloc(sizeof(struct event)); 
      new_arrive_network->type = PROCESS_ARRIVE_NETWORK; 
      new_arrive_network->id = old->id; 
      new_arrive_network->time = old->time + num; 

      pushPQ(pQ, new_arrive_network); 
      networkQ->status = 1; // Network BUSY 
    }

    else{
      // Network is busy || Network queue is non-empty ...  add to networkQeueue 

      enqueue(networkQ, old); 
      dont_free = 1; 
    }
  }

  // GO TO DISK DIRECTLY 
  else if(disk1Q->status == 0 && disk1Q->counter == 0){
     // disk queue is empty and disk is idle .. generate new event 
    
    int num = random_num_between_interval(DISK1_MIN, DISK1_MAX); 

    struct event *new_arrive_disk1;
    new_arrive_disk1 = malloc(sizeof(struct event));
    new_arrive_disk1->type = PROCESS_ARRIVE_DISK1; 
    new_arrive_disk1->id = old->id; 
    new_arrive_disk1->time = old->time + num; 

    pushPQ(pQ, new_arrive_disk1); 
    disk1Q->status = 1; // Disk1 BUSY 
  }

  else if(disk2Q->status == 0 && disk2Q->counter == 0){
    // disk queue is empty and disk is idle .. generate new event 
    
    int num = random_num_between_interval(DISK2_MIN, DISK2_MAX); 

    struct event *new_arrive_disk2;
    new_arrive_disk2 = malloc(sizeof(struct event));
    new_arrive_disk2->type = PROCESS_ARRIVE_DISK2; 
    new_arrive_disk2->id = old->id; 
    new_arrive_disk2->time = old->time + num; 

    pushPQ(pQ, new_arrive_disk2); 
    disk2Q->status = 1; // Disk2 BUSY 
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

    dont_free = 1; // don't free old event yet 
  }

  // AT THIS POINT the old event (PROCESS_EXIT_CPU) is handled, the CPU IS NOW OPEN TO THOSE IN CPUQUEUE 

  // ****** do this for other end events (eliminate the need to create a new event type) ***********
  if(cpuQ->counter > 0){

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event)); 
    new_arrive_cpu = dequeue(cpuQ); 
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; // change type from PROCESS_ARRIVAL to ... 
    //new_arrive_cpu->id = old->id; 
    new_arrive_cpu->time = old->time; 
    pushPQ(pQ, new_arrive_cpu); 

    cpuQ->status = 1; // busy 
    dont_free = 1; 
  }

  if(dont_free == 0)
    free(old); 
  // free(old) but only if not on queue 
}

void handle_process_exit_system(Queue *pQ, struct event *old){
  // free old struct event, dont generate any new events 

  printf("PROCESS TYPE: %d ID: %d has LEFT SYSTEM\n", old->id, old->type); 
  free(old);
}

void handle_process_arrive_disk1(Queue *pQ, Queue *disk1Q, struct event *old){
// generate PROCESS_EXIT_DISK1 event

  int num = random_num_between_interval(DISK1_MIN, DISK1_MAX); 

  struct event *new_exit_disk1; 
  new_exit_disk1 = malloc(sizeof(struct event)); 
  new_exit_disk1->type = PROCESS_EXIT_DISK1;  
  new_exit_disk1->id = old->id; 
  new_exit_disk1->time = old->time + num; // new event time 

  pushPQ(pQ, new_exit_disk1); 
  free(old); 
}

void handle_process_exit_disk1(Queue *pQ, Queue *cpuQ, Queue *disk1Q, struct event *old){
// back to start (CPU or queue), handle who uses disk1 next 

  disk1Q->status = 0; // disk1 now idle 
  int dont_free = 0; 

  if(cpuQ->status == 1 || (cpuQ->counter > 0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQ, old); 
    dont_free = 1; // dont free the old event 
  }

  else if(cpuQ->status == 0 && (cpuQ->counter == 0)){
    // cpu is IDLE and cpu queue is EMPTY 

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event));
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; 
    new_arrive_cpu->id = old->id;
    new_arrive_cpu->time = old->time; 

    pushPQ(pQ, new_arrive_cpu);
    cpuQ->status = 1; 
    dont_free = 1; 
  }

  // DISK1
  if(disk1Q->counter > 0){
    // disk1Q non-empty 

    struct event *new_arrive_disk1; 
    new_arrive_disk1 = malloc(sizeof(struct event));
    new_arrive_disk1 = dequeue(disk1Q); 
    // time

    pushPQ(pQ, new_arrive_disk1);
    disk1Q->status = 1; 
  }

  if(dont_free == 0)
    free(old); 
}

void handle_process_arrive_disk2(Queue *pQ, Queue *disk2Q, struct event *old){
// generate PROCESS_EXIT_DISK2 event

  int num = random_num_between_interval(DISK2_MIN, DISK2_MAX); 

  struct event *new_arrive_disk2; 
  new_arrive_disk2 = malloc(sizeof(struct event)); 
  new_arrive_disk2->type = PROCESS_EXIT_DISK2;  
  new_arrive_disk2->id = old->id; 
  new_arrive_disk2->time = old->time + num; 

  pushPQ(pQ, new_arrive_disk2); 
  free(old); 
}

void handle_process_exit_disk2(Queue *pQ, Queue *cpuQ, Queue *disk2Q, struct event *old){
// back to start (CPU or queue), handle who uses disk2 next 

  disk2Q->status = 0; // disk1 now idle 
  int dont_free = 0; 

  if(cpuQ->status == 1 || (cpuQ->counter > 0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQ, old); 
    dont_free = 1; // dont free the old event 
  }

  else if(cpuQ->status == 0 && (cpuQ->counter == 0)){
    // cpu is IDLE and cpu queue is EMPTY 

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event));
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; 
    new_arrive_cpu->id = old->id;
    new_arrive_cpu->time = old->time; 

    pushPQ(pQ, new_arrive_cpu);
    cpuQ->status = 1; 
  }

  // DISK1
  if(disk2Q->counter > 0){
    // disk1Q non-empty 

    struct event *new_arrival_disk2; 
    new_arrival_disk2 = malloc(sizeof(struct event));
    new_arrival_disk2 = dequeue(disk2Q); 
    // time

    pushPQ(pQ, new_arrival_disk2);
    disk2Q->status = 1; 
  }

  if(dont_free == 0)
    free(old); 
}

void handle_process_arrive_network(Queue *pQ, Queue *networkQ, struct event *old){
  // generate PROCESS_EXIT_NETWORK event 

  int num = random_num_between_interval(NETWORK_MIN, NETWORK_MAX); 

  struct event *new_exit_network;
  new_exit_network = malloc(sizeof(struct event));
  new_exit_network->type = PROCESS_EXIT_NETWORK; 
  new_exit_network->id = old->id;
  new_exit_network->time = old->time + num; 

  pushPQ(pQ, new_exit_network); 
  free(old); 
}

void handle_process_exit_network(Queue *pQ, Queue *cpuQ, Queue *networkQ, struct event *old){
  // back to start (CPU or queue), handle who uses network next 

  networkQ->status = 0; // network IDLE 
  int dont_free = 0; 
  
  if(cpuQ->status == 1 || (cpuQ->counter > 0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQ, old); 
    dont_free = 1; // dont free the old event 
  }

  else if(cpuQ->status == 0 && (cpuQ->counter == 0)){
    // cpu is IDLE and cpu queue is EMPTY 

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event));
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; 
    new_arrive_cpu->id = old->id;
    new_arrive_cpu->time = old->time; 

    pushPQ(pQ, new_arrive_cpu);
    cpuQ->status = 1; 
  }

  if(networkQ->counter > 0){
    // networkQ is non-empty 

    struct event *new_arrive_network;
    new_arrive_network = malloc(sizeof(struct event));
    new_arrive_network = dequeue(networkQ); 

    pushPQ(pQ, new_arrive_network);
    networkQ->status = 1; 
  }

  if(dont_free == 0)
    free(old); 
}






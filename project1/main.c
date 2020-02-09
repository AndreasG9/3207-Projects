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

void read_config_file(FILE *fptrLOG);
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

int id_count, cpu_util, disk1_util, disk2_util, network_util; 
// int pq_count, max_pq_count, ... <--- declared inside queue.c  


// ==================================== EVENT TYPES (in queue.c)=======================================
// typedef enum {START_SIM, PROCESS_ARRIVAL, PROCESS_ARRIVE_CPU, PROCESS_EXIT_CPU, PROCESS_EXIT_SYSTEM, 
// //                0             1                 2                  3                 4
// PROCESS_ARRIVE_DISK1, PROCESS_EXIT_DISK1, PROCESS_ARRIVE_DISK2, PROCESS_EXIT_DISK2,
// //      5                       6                 7                      8                        
// PROCESS_ARRIVE_NETWORK, PROCESS_EXIT_NETWORK, END_SIM} TYPES;
// //     9                        10                11


// for reference, corresponds to enum TYPES value ... ex. printf("%s", event_types[PROCESS_EXIT_DISK2])) --> "exits the disk2"
char *event_types[12] = {"simulations STARTS", "arrival", "arrives at cpu", "exits the cpu", "exits the SYSTEM", 
"arrives at disk1", "exits the disk1", "arrives at disk2", "exits the disk2", "arrives at network", "exits the network", 
"simulation ENDS"}; 

int main(int args, char *argv[]){

  FILE *fptrLOG = NULL;
  fptrLOG = fopen("events.log", "w"); 

  if (fptrLOG == NULL) 
    { 
      printf("main.c: could not open events.log"); 
      exit(1); 
    } 

  read_config_file(fptrLOG); // const values (in name) based on thoses read from the configuration file, write constants to log file 
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
  int control = 1, id_count = 1; 
  int events_popped = 0; 
  pq_count, max_pq_count = 0; 
  cpu_count, max_cpu_count, disk1_count, max_disk1_count, disk2_count, max_disk2_count, network_count, max_network_count = 0; 
  cpu_util, disk1_util, disk2_util, network_util = 0; 


  // MAIN LOOP 
  while(control){

    // grab the top priority event 
    struct event *current; 
    current = malloc(sizeof(struct event)); 
    current = popPQ(pQueue); 
    ++events_popped; // for STAT 

    switch(current->type)
    {
        case START_SIM:
          //printf("At time %d, %s.\n", current->time, event_types[current->type]);
          fprintf(fptrLOG, "At time %d, %s.\n", current->time, event_types[current->type]);
          handle_start_simulation(pQueue, current); // called once, will generate first PROCESS_ARRIVAL
          break; 

        case PROCESS_ARRIVAL:
          //printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_arrival(pQueue, cpuQueue, current); 
          break; 
        
        case PROCESS_ARRIVE_CPU:
          //printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_arrive_cpu(pQueue, current);
          break; 
        
        case PROCESS_EXIT_CPU:
         // printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
         fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_exit_cpu(pQueue, cpuQueue, networkQueue, disk1Queue, disk2Queue, current); 
          break; 
        
        case PROCESS_EXIT_SYSTEM:
         // printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
         fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_exit_system(pQueue, current); 
          break; 
        
        case PROCESS_ARRIVE_DISK1:
          //printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_arrive_disk1(pQueue, disk1Queue, current); 
          break; 

        case PROCESS_EXIT_DISK1:
          //printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_exit_disk1(pQueue, cpuQueue, disk1Queue, current); 
          break; 

        case PROCESS_ARRIVE_DISK2:
          //printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_arrive_disk2(pQueue, disk2Queue, current); 
          break; 
        
        case PROCESS_EXIT_DISK2:
          //printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_exit_disk2(pQueue, cpuQueue, disk2Queue, current); 
          break; 
        
        case PROCESS_ARRIVE_NETWORK:
         // printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
         fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_arrive_network(pQueue, networkQueue, current);
          break;
        
        case PROCESS_EXIT_NETWORK:
          //printf("At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]); 
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          handle_process_exit_network(pQueue, cpuQueue, networkQueue, current); 
          break; 

        case END_SIM: 
          //printf("At time %d, %s.\n", current->time, event_types[current->type]);
          fprintf(fptrLOG, "At time %d, process %d, %s.\n", current->time, current->id, event_types[current->type]);
          control = handle_end_simulation(pQueue, current); // control set to 0, exit loop 
          break;  
    }

    get_counts(pQueue, cpuQueue, disk1Queue, disk2Queue, networkQueue); // every event drawn, get current count of queues  
  }

  fclose(fptrLOG); // no more log data 


  // STAT FILE 
  FILE *fptrSTAT = NULL;
    fptrSTAT = fopen("math.stat", "w"); 

  if(fptrSTAT == NULL){
    puts("main.c: could not open file math.stat");
    exit(1); 
  }

  printf("TOTAL TIME: %d\n", cpu_util); 

  fprintf(fptrSTAT, "%s\n%s %lf\n", "---------------Queues---------------", "Average size of CPU Queue at any given event time: ", ((double)cpu_count) / events_popped);
  fprintf(fptrSTAT, "%s %d\n", "Max Size of CPU Queue:", max_cpu_count); 
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Disk1 Queue:", ((double)disk1_count) / events_popped); 
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Disk1 Queue:", max_disk1_count); 
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Disk2 Queue:", ((double)disk2_count) / events_popped); 
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Disk2 Queue:", max_disk2_count); 
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Network Queue:", ((double)network_count) / events_popped); 
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Network Queue:", max_network_count); 
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Event Queue:", ((double)pq_count) / events_popped); 
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Event Queue:", max_pq_count); 

  fprintf(fptrSTAT, "\n%s\n", "---------------Utilization---------------"); 
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of CPU: ", ((double)cpu_util) / (FIN_TIME-INIT_TIME)); 
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of Disk 1: ", ((double)disk1_util) / (FIN_TIME-INIT_TIME)); 
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of Disk 2: ", ((double)disk2_util) / (FIN_TIME-INIT_TIME)); 
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of Network: ", ((double)network_util) / (FIN_TIME-INIT_TIME)); 


  //(FIN_TIME-INIT_TIME)

  // all thats left to free are the Queues (nodes and events within already freed)
  free(cpuQueue), free(disk1Queue), free(disk2Queue), free(networkQueue), free(pQueue); 

  puts("Discrete Event Simulator/ Longest lab ever COMPELTED");
  return 0; 
}

void read_config_file(FILE *fptrLOG){
  
  FILE *fptr = NULL;
  fptr = fopen("constants.CONF", "r"); // change to command line input later?? 

  if(fptr == NULL){
    puts("main.c: could not open file constants.CONF");
    exit(1); 
  }
  
  // read the int value from file (follows this format), store in corresponding event variable. ALSO, print log file %s %d 
  char temp[20]; 
  //fscanf(fptr, "%*s %d", &SEED);

  fscanf(fptr, "%s %d", temp, &SEED); 
  fprintf(fptrLOG, "%s %d\n", temp, SEED); 
  fscanf(fptr, "%s %d", temp, &INIT_TIME);
  fprintf(fptrLOG, "%s %d\n", temp, INIT_TIME);  
  fscanf(fptr, "%s %d", temp, &FIN_TIME);
  fprintf(fptrLOG, "%s %d\n", temp, FIN_TIME);
  fscanf(fptr, "%s %d", temp, &ARRIVE_MIN);
  fprintf(fptrLOG, "%s %d\n", temp, ARRIVE_MIN);
  fscanf(fptr, "%s %d", temp, &ARRIVE_MAX);
  fprintf(fptrLOG, "%s %d\n", temp, ARRIVE_MAX);
  fscanf(fptr, "%s %lf", temp, &QUIT_PROB);
  fprintf(fptrLOG, "%s %lf\n", temp, QUIT_PROB);
  fscanf(fptr, "%s %lf", temp, &NETWORK_PROB);
  fprintf(fptrLOG, "%s %lf\n", temp, NETWORK_PROB);
  fscanf(fptr, "%s %d", temp, &CPU_MIN);
  fprintf(fptrLOG, "%s %d\n", temp, CPU_MIN);
  fscanf(fptr, "%s %d", temp, &CPU_MAX);
  fprintf(fptrLOG, "%s %d\n", temp, CPU_MAX);
  fscanf(fptr, "%s %d", temp, &DISK1_MIN);
  fprintf(fptrLOG, "%s %d\n", temp, DISK1_MIN);
  fscanf(fptr, "%s %d", temp, &DISK1_MAX);
  fprintf(fptrLOG, "%s %d\n", temp, DISK1_MAX);
  fscanf(fptr, "%s %d", temp, &DISK2_MIN);
  fprintf(fptrLOG, "%s %d\n", temp, DISK2_MIN);
  fscanf(fptr, "%s %d", temp, &DISK2_MAX);
  fprintf(fptrLOG, "%s %d\n", temp, DISK2_MAX);
  fscanf(fptr, "%s %d", temp, &NETWORK_MIN);
  fprintf(fptrLOG, "%s %d\n", temp, NETWORK_MIN);
  fscanf(fptr, "%s %d", temp, &NETWORK_MAX);
  fprintf(fptrLOG, "%s %d\n", temp, NETWORK_MAX);

  fclose(fptr);
}

int random_num_between_interval(int min, int max){
  return (min + rand() % (max-min+1)); // [min-max]
}


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

 // printf("At time %d, %s.\n", old->time, event_types[old->type]);

  struct event *new_process_arrival; 
  new_process_arrival = malloc(sizeof(struct event)); 
  new_process_arrival->type = PROCESS_ARRIVAL; 
  new_process_arrival->time = INIT_TIME; // 0  
  new_process_arrival->id = id_count; // 0 

  // ADD first job arrival (time 0) to queue, get the loop going ... 
  pushPQ(pQ, new_process_arrival); 
}

void handle_process_arrival(Queue *pQ, Queue *cpuQueue, struct event *old){
  // will generate itself 

  int dont_free = 0; 

  if(cpuQueue->status == 1 || (cpuQueue->counter > 0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQueue, old); 
    dont_free = 1; // dont free the old event 
  }

  else{
  //else if(cpuQueue->status == 0 && cpuQueue->counter == 0){
    // no cpuQueue required, generate NEXT event (PROCESS_ARRIVE_CPU)

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event));
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU;
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
  new_arrival->time = old->time + num; // new event time 

  pushPQ(pQ, new_arrival); // add to PQ 

  // case to free up old event, but if NOT IN QUEUE 
  if(dont_free == 0)
    free(old); 
}

void handle_process_arrive_cpu(Queue *pQ, struct event *old){
  // will generate PROCESS_EXIT_CPU event 

  //printf("At time %d, process %d, %s.\n", old->time, old->id, event_types[old->type]); 

  int num = random_num_between_interval(CPU_MIN, CPU_MAX); 
  
  struct event *new_exit_cpu; 
  new_exit_cpu = malloc(sizeof(struct event)); 
  new_exit_cpu->type = PROCESS_EXIT_CPU;  
  new_exit_cpu->id = old->id; 
  new_exit_cpu->time = old->time + num; // new event time 
  pushPQ(pQ, new_exit_cpu); 

  int diff = new_exit_cpu->time - old->time; // for util 
  cpu_util = cpu_util + diff; 
  
  free(old); 
}

void handle_process_exit_cpu(Queue *pQ, Queue *cpuQ, Queue *networkQ, Queue *disk1Q, Queue *disk2Q, struct event *old){
  // will generate PROCESS_EXIT_SYSTEM, or arrival to a disk or network 

 // printf("At time %d, process %d, %s.\n", old->time, old->id, event_types[old->type]); 

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

      struct event *new_arrive_network; 
      new_arrive_network = malloc(sizeof(struct event)); 
      new_arrive_network->type = PROCESS_ARRIVE_NETWORK; 
      new_arrive_network->id = old->id; 
      new_arrive_network->time = old->time; // right away 

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
     // disk queue is empty and disk is idle .. generate new event right away 
  
    struct event *new_arrive_disk1;
    new_arrive_disk1 = malloc(sizeof(struct event));
    new_arrive_disk1->type = PROCESS_ARRIVE_DISK1; 
    new_arrive_disk1->id = old->id; 
    new_arrive_disk1->time = old->time; // same time 

    pushPQ(pQ, new_arrive_disk1); 
    disk1Q->status = 1; // Disk1 BUSY 
  }

  else if(disk2Q->status == 0 && disk2Q->counter == 0){
    // disk queue is empty and disk is idle .. generate new event 

    struct event *new_arrive_disk2;
    new_arrive_disk2 = malloc(sizeof(struct event));
    new_arrive_disk2->type = PROCESS_ARRIVE_DISK2; 
    new_arrive_disk2->id = old->id; 
    new_arrive_disk2->time = old->time; 

    pushPQ(pQ, new_arrive_disk2); 
    disk2Q->status = 1; // Disk2 BUSY 
  }

  else{
    // push old event to SMALLER disk queue 

    if(disk1Q->counter > disk2Q->counter){
      //printf("disk2 queue SMALLER \n");
      enqueue(disk2Q, old); 
    }

    else if(disk1Q->counter == disk2Q->counter){
      // queues are equal in size, generate a random num to pick a queue 

      int num = random_num_between_interval(1, 10); 

      if(num > 5){
        enqueue(disk2Q, old); 
       // puts("on disk2 q");
      }
      else{
        enqueue(disk1Q, old); 
        //puts("on disk1 q");
      }
    }

    else{
      enqueue(disk1Q, old); 
     // puts("on disk1 q"); 
    }

    dont_free = 1; // don't free old event yet  
  }

  // AT THIS POINT the old event (PROCESS_EXIT_CPU) is handled, the CPU IS NOW OPEN TO THOSE IN CPUQUEUE (PROCESS_ARRIVAL)

  // ****** do this for other end events (eliminate the need to create a new event type) ***********
  if(cpuQ->counter > 0){

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event)); 
    new_arrive_cpu = dequeue(cpuQ); 
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; // change type from PROCESS_ARRIVAL to ... 
    new_arrive_cpu->time = old->time;

    pushPQ(pQ, new_arrive_cpu); 
    cpuQ->status = 1; // busy  
    dont_free = 1; 
  }

  if(dont_free == 0)
    free(old); 
}

void handle_process_exit_system(Queue *pQ, struct event *old){
  // free old struct event, dont generate any new events 

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

  int diff = new_exit_disk1->time - old->time; // for util 
  disk1_util = disk1_util + diff; 

  free(old); 
}

void handle_process_exit_disk1(Queue *pQ, Queue *cpuQ, Queue *disk1Q, struct event *old){
// back to start (CPU or queue), handle who uses disk1 next 


  disk1Q->status = 0; // disk1 now idle 
  int dont_free = 0; 

  // deal CPU 
  if(cpuQ->status == 1 || (cpuQ->counter > 0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQ, old); 
   // ++cpu_count; 
    dont_free = 1; // dont free the old event 
  }

  if(cpuQ->status == 0 && (cpuQ->counter == 0)){
    // cpu is IDLE and cpu queue is EMPTY, go directly to process_arrive_cpu

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event));
    //new_arrive_cpu = dequeue(cpuQ); 
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; // change type 
    new_arrive_cpu->id = old->id;
    new_arrive_cpu->time = old->time; 

    pushPQ(pQ, new_arrive_cpu);
    cpuQ->status = 1; 
    dont_free = 1; 
  }

  // deal disk1 q 
  if(disk1Q->counter > 0){
    // disk1Q non-empty 

    struct event *new_arrive_disk1; 
    new_arrive_disk1 = malloc(sizeof(struct event));
    new_arrive_disk1 = dequeue(disk1Q); 
    new_arrive_disk1->type = PROCESS_ARRIVE_DISK1; 
    new_arrive_disk1->time = old->time; 

    pushPQ(pQ, new_arrive_disk1);
    disk1Q->status = 1; 
  }

  if(dont_free == 0)
    free(old); 
}

void handle_process_arrive_disk2(Queue *pQ, Queue *disk2Q, struct event *old){
// generate PROCESS_EXIT_DISK2 event

 // printf("At time %d, process %d, %s.\n", old->time, old->id, event_types[old->type]);

  int num = random_num_between_interval(DISK2_MIN, DISK2_MAX); 

  struct event *new_exit_disk2; 
  new_exit_disk2 = malloc(sizeof(struct event)); 
  new_exit_disk2->type = PROCESS_EXIT_DISK2;  
  new_exit_disk2->id = old->id; 
  new_exit_disk2->time = old->time + num; 

  pushPQ(pQ, new_exit_disk2); 
  int diff = new_exit_disk2->time - old->time; // for util 
  disk2_util = disk2_util + diff; 
  free(old); 
}

void handle_process_exit_disk2(Queue *pQ, Queue *cpuQ, Queue *disk2Q, struct event *old){
// back to start (CPU or queue), handle who uses disk2 next 

  //printf("At time %d, process %d, %s.\n", old->time, old->id, event_types[old->type]);

  disk2Q->status = 0; // disk1 now idle 
  int dont_free = 0; 

  if(cpuQ->status == 1 || (cpuQ->counter > 0)){
    // if CPU is busy, or the cpuQueue is non-empty, add process to queue  

    enqueue(cpuQ, old); 
    dont_free = 1; // dont free the old event 
  }

  // else if {
  if(cpuQ->status == 0 && (cpuQ->counter == 0)){
    // cpu is IDLE and cpu queue is EMPTY .. old type now PROCESS_ARRIVE_CPU

    struct event *new_arrive_cpu; 
    new_arrive_cpu = malloc(sizeof(struct event));
    //new_arrive_cpu = dequeue(cpuQ); 
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; // change type 
    new_arrive_cpu->time = old->time; 
    new_arrive_cpu->id = old->id; 

    pushPQ(pQ, new_arrive_cpu);
    cpuQ->status = 1; 
  }

  // DISK2
  if(disk2Q->counter > 0){
    // disk2Q non-empty 

    struct event *new_arrival_disk2; 
    new_arrival_disk2 = malloc(sizeof(struct event));
    new_arrival_disk2 = dequeue(disk2Q); 
    new_arrival_disk2->type = PROCESS_ARRIVE_DISK2; 
    new_arrival_disk2->time = old->time; 
    
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
  int diff = new_exit_network->time - old->time; // for util 
  network_util = network_util + diff; 

  free(old); 
}

void handle_process_exit_network(Queue *pQ, Queue *cpuQ, Queue *networkQ, struct event *old){
  // back to start (CPU or queue), handle who uses network next 

  //printf("At time %d, process %d, %s.\n", old->time, old->id, event_types[old->type]);

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

int handle_end_simulation(Queue *pQ, struct event *old){
  // end the simulation b/c old->time == FIN_TIME
  // 0 will flip the control and the while loop will end, post-Sim stat put after loop 
  return 0; 
}






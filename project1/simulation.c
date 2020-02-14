/*
Project 1
Andreas Gagas
simulation.c - discrete event simulator       
This single file is rep. as two on github(main.c, queue.c)
*/

#include <stdio.h>
#include <stdlib.h>

typedef enum {START_SIM, PROCESS_ARRIVAL, PROCESS_ARRIVE_CPU, PROCESS_EXIT_CPU, PROCESS_EXIT_SYSTEM,
//                0             1                 2                  3                 4
PROCESS_ARRIVE_DISK1, PROCESS_EXIT_DISK1, PROCESS_ARRIVE_DISK2, PROCESS_EXIT_DISK2,
//      5                       6                 7                      8
PROCESS_ARRIVE_NETWORK, PROCESS_EXIT_NETWORK, END_SIM} TYPES;
//     9                        10               11

// global variables (for simplicity)
int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, CPU_MIN, CPU_MAX,
DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, NETWORK_MIN, NETWORK_MAX;
double QUIT_PROB, NETWORK_PROB;

// global variables (for stat)
int id_count, cpu_util, disk1_util, disk2_util, network_util;
int cpu_max_diff, disk1_max_diff, disk2_max_diff, network_max_diff;
int process_arrive_cpu_count, process_arrive_disk1_count, process_arrive_disk2_count, process_arrive_network_count;
int pq_count, max_pq_count, cpu_count, max_cpu_count, disk1_count, max_disk1_count, disk2_count, max_disk2_count, network_count, max_network_count; // for QUEUE counts

struct event{
// represent an event

  int id; // unique processID (each systemArrival is counted)
  TYPES type; // event type, defined above
  int time; // length of job, determined by randon num. between const min and max of type
};

struct node{
  // a node will hold the data/job and ref to next node in list (to right)

  struct event *e; // struct event data fields: int id, TYPES type, int time
  struct node *next;
};

typedef struct queue{

  int counter; // num of jobs
  int status; // currently handling an item ... when event is in CPU switch to 1 for busy
  struct node *front;
  struct node *rear;
}Queue;



Queue* initQueue();
void enqueue(Queue *queue, struct event *data); 
struct event* dequeue(Queue *queue); 
int isEmpty(Queue *queue);
void pushPQ(Queue *queue, struct event *e);
struct event* popPQ(Queue *queue);
void get_counts(Queue *pQ, Queue *cpuQ, Queue *disk1Q, Queue *disk2Q, Queue *networkQ);  

void read_config_file(FILE *fptrLOG);
int random_num_between_interval(int min, int max);
double DOUBLE_random_num_between_interval(double min, double max); 

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

void printPQ(Queue *pQ, char *event_types[]); // testing 


int main(int args, char *argv[]){

  // for reference, corresponds to enum TYPES value ... ex. printf("%s", event_types[PROCESS_EXIT_DISK2])) --> "exits the disk2"
  char *event_types[12] = {"simulations STARTS", "arrival", "arrives at cpu", "exits the cpu", "exits the SYSTEM",
  "arrives at disk1", "exits the disk1", "arrives at disk2", "exits the disk2", "arrives at network", "exits the network",
  "simulation ENDS"};

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
  int control, id_count = 1;

  // for STAT file, don't like formating it like this
  int events_popped = 0;
  pq_count, max_pq_count = 0, cpu_count, max_cpu_count, disk1_count, max_disk1_count, disk2_count, max_disk2_count, network_count, max_network_count = 0;
  cpu_util, disk1_util, disk2_util, network_util = 0;
  cpu_max_diff, disk1_max_diff, disk2_max_diff, network_max_diff = 0;
  process_arrive_cpu_count, process_arrive_cpu_count, process_arrive_disk1_count, process_arrive_disk2_count, process_arrive_network_count = 0;

  // MAIN LOOP
  while(control){

    // printPQ(pQueue, event_types);

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
          fprintf(fptrLOG, "At time %d, %s.\n", current->time, event_types[current->type]);
          control = handle_end_simulation(pQueue, current); // control set to 0, exit loop
          break;
    }

   get_counts(pQueue, cpuQueue, disk1Queue, disk2Queue, networkQueue); // every event drawn, get current count of queues

  }
  fclose(fptrLOG); // no more log data

  // STAT FILE
  FILE *fptrSTAT = NULL;
    fptrSTAT = fopen("math.stat", "w+");

  if(fptrSTAT == NULL){
    puts("main.c: could not open file math.stat");
    exit(1);
  }

  fprintf(fptrSTAT, "%s\n%s %lf\n", "---------------Queues----------------", "Average size of CPU Queue: ", ((double)cpu_count) / events_popped);
  fprintf(fptrSTAT, "%s %d\n", "Max Size of CPU Queue:", max_cpu_count);
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Disk1 Queue:", ((double)disk1_count) / events_popped);
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Disk1 Queue:", max_disk1_count);
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Disk2 Queue:", ((double)disk2_count) / events_popped);
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Disk2 Queue:", max_disk2_count);
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Network Queue:", ((double)network_count) / events_popped);
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Network Queue:", max_network_count);
  fprintf(fptrSTAT, "%s %lf\n", "Average Size of Event Queue:", ((double)pq_count) / events_popped);
  fprintf(fptrSTAT, "%s %d\n", "Max Size of Event Queue:", max_pq_count);

  fprintf(fptrSTAT, "\n%s\n", "---------------Utilization-------------");
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of CPU: ", ((double)cpu_util) / (FIN_TIME-INIT_TIME));
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of Disk 1: ", ((double)disk1_util) / (FIN_TIME-INIT_TIME));
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of Disk 2: ", ((double)disk2_util) / (FIN_TIME-INIT_TIME));
  fprintf(fptrSTAT, "%s %lf\n", "Utilization of Network: ", ((double)network_util) / (FIN_TIME-INIT_TIME));

  fprintf(fptrSTAT, "\n%s\n", "---------------Response Times-----------");
  fprintf(fptrSTAT, "%s %lf\n", "Average Response Time of CPU: ", ((double)cpu_util) / process_arrive_cpu_count);
  fprintf(fptrSTAT, "%s %d\n", "Max Response Time of CPU: ", cpu_max_diff);
  fprintf(fptrSTAT, "%s %lf\n", "Average Response Time of DISK 1: ", ((double)disk1_util) / process_arrive_disk1_count);
  fprintf(fptrSTAT, "%s %d\n", "Max Response Time of DISK 1: ", disk1_max_diff);
  fprintf(fptrSTAT, "%s %lf\n", "Average Response Time of DISK 2: ", ((double)disk2_util) / process_arrive_disk2_count);
  fprintf(fptrSTAT, "%s %d\n", "Max Response Time of DISK 2: ", disk2_max_diff);
  fprintf(fptrSTAT, "%s %lf\n", "Average Response Time of Network: ", ((double)network_util) / process_arrive_network_count);
  fprintf(fptrSTAT, "%s %d\n", "Max Response Time of Network: ", network_max_diff);

  fprintf(fptrSTAT, "\n%s\n", "---------------Throughput--------------");
  fprintf(fptrSTAT, "%s %lf %s\n", "Throughput of CPU: ", ((double)process_arrive_cpu_count) / (FIN_TIME-INIT_TIME), " Jobs per unit of time");
  fprintf(fptrSTAT, "%s %lf %s\n", "Throughput of DISK 1: ", ((double)process_arrive_disk1_count) / (FIN_TIME-INIT_TIME), " Jobs per unit of time");
  fprintf(fptrSTAT, "%s %lf %s\n", "Throughput of DISK 2: ", ((double)process_arrive_disk2_count) / (FIN_TIME-INIT_TIME), " Jobs per unit of time");
  fprintf(fptrSTAT, "%s %lf %s\n", "Throughput of Network: ", ((double)process_arrive_network_count) / (FIN_TIME-INIT_TIME), " Jobs per unit of time");

  fclose(fptrSTAT);


  // all thats left to free are the Queues (nodes and events within already freed)
  free(cpuQueue), free(disk1Queue), free(disk2Queue), free(networkQueue), free(pQueue);

  puts("\nDiscrete Event Simulator COMPLETED\n");
  puts("*** See math.stat, events.log, README.md, and RUNS.txt for simulation details ***\n");

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

double DOUBLE_random_num_between_interval(double min, double max){

  double res = (double)rand()/(double)(RAND_MAX/max);
  //printf("NUM: %lf\n", res);
  return res; 
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

  struct event *new_process_arrival;
  new_process_arrival = malloc(sizeof(struct event));
  new_process_arrival->type = PROCESS_ARRIVAL;
  new_process_arrival->time = INIT_TIME; // 0
  new_process_arrival->id = id_count; // 0

  // ADD first job arrival (time 0) to queue, get the loop going ...
  pushPQ(pQ, new_process_arrival);

  free(old);
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

  ++process_arrive_cpu_count;

  int num = random_num_between_interval(CPU_MIN, CPU_MAX);

  struct event *new_exit_cpu;
  new_exit_cpu = malloc(sizeof(struct event));
  new_exit_cpu->type = PROCESS_EXIT_CPU;
  new_exit_cpu->id = old->id;
  new_exit_cpu->time = old->time + num; // new event time

  pushPQ(pQ, new_exit_cpu);
  int diff = new_exit_cpu->time - old->time; // for util, and used for response time

  if(diff > cpu_max_diff)
    cpu_max_diff = diff;

  cpu_util = cpu_util + diff;

  free(old);
}

void handle_process_exit_cpu(Queue *pQ, Queue *cpuQ, Queue *networkQ, Queue *disk1Q, Queue *disk2Q, struct event *old){
  // will generate PROCESS_EXIT_SYSTEM, or arrival to a disk or network

  cpuQ->status = 0; // CPU now idle
  int dont_free = 0; // free old event is NOT IN QUEUE
  double num1 = DOUBLE_random_num_between_interval(0, 1); 
  double num2 = DOUBLE_random_num_between_interval(0, 1); 

  if(num1 < QUIT_PROB){
    // generate PROCESS_EXIT_SYSTEM event

    struct event *new_exit_system;
    new_exit_system = malloc(sizeof(struct event));
    new_exit_system->type = PROCESS_EXIT_SYSTEM;
    new_exit_system->id = old->id;
    new_exit_system->time = old->time; // same time

    pushPQ(pQ, new_exit_system);
  }

  else if(num2 < NETWORK_PROB){
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
      enqueue(disk2Q, old);
    }

    else if(disk1Q->counter == disk2Q->counter){
      // queues are equal in size, generate a random num to pick a queue
      
      int num = random_num_between_interval(1, 10);

      if(num > 5){
        enqueue(disk2Q, old);
      }
      else{
        enqueue(disk1Q, old);
      }
    }

    else{
      enqueue(disk1Q, old);
    }

    dont_free = 1; // don't free old event yet
  }

  // AT THIS POINT the old event (PROCESS_EXIT_CPU) is handled, the CPU IS NOW OPEN TO THOSE IN CPUQUEUE (PROCESS_ARRIVAL)
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

  ++process_arrive_disk1_count;
  int num = random_num_between_interval(DISK1_MIN, DISK1_MAX);

  struct event *new_exit_disk1;
  new_exit_disk1 = malloc(sizeof(struct event));
  new_exit_disk1->type = PROCESS_EXIT_DISK1;
  new_exit_disk1->id = old->id;
  new_exit_disk1->time = old->time + num; // new event time

  pushPQ(pQ, new_exit_disk1);

  int diff = new_exit_disk1->time - old->time; // for util

  if(diff > disk1_max_diff)
    disk1_max_diff = diff;

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
    dont_free = 1; // dont free the old event
  }

  if(cpuQ->status == 0 && (cpuQ->counter == 0)){
    // cpu is IDLE and cpu queue is EMPTY, go directly to process_arrive_cpu

    struct event *new_arrive_cpu;
    new_arrive_cpu = malloc(sizeof(struct event));

    new_arrive_cpu->type = PROCESS_ARRIVE_CPU;
    new_arrive_cpu->id = old->id;
    new_arrive_cpu->time = old->time;

    pushPQ(pQ, new_arrive_cpu);
    cpuQ->status = 1;
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

  ++process_arrive_disk2_count;
  int num = random_num_between_interval(DISK2_MIN, DISK2_MAX);

  struct event *new_exit_disk2;
  new_exit_disk2 = malloc(sizeof(struct event));
  new_exit_disk2->type = PROCESS_EXIT_DISK2;
  new_exit_disk2->id = old->id;
  new_exit_disk2->time = old->time + num;

  pushPQ(pQ, new_exit_disk2);

  int diff = new_exit_disk2->time - old->time; // for util

  if(diff > disk2_max_diff)
    disk2_max_diff = diff;

  disk2_util = disk2_util + diff;
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

  if(cpuQ->status == 0 && (cpuQ->counter == 0)){
    // cpu is IDLE and cpu queue is EMPTY .. old type now PROCESS_ARRIVE_CPU

    struct event *new_arrive_cpu;
    new_arrive_cpu = malloc(sizeof(struct event));
    new_arrive_cpu->type = PROCESS_ARRIVE_CPU; // change type
    new_arrive_cpu->time = old->time;
    new_arrive_cpu->id = old->id;

    pushPQ(pQ, new_arrive_cpu);
    cpuQ->status = 1;
    //dont_free = 1;
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

  ++process_arrive_network_count;
  int num = random_num_between_interval(NETWORK_MIN, NETWORK_MAX);

  struct event *new_exit_network;
  new_exit_network = malloc(sizeof(struct event));
  new_exit_network->type = PROCESS_EXIT_NETWORK;
  new_exit_network->id = old->id;
  new_exit_network->time = old->time + num;

  pushPQ(pQ, new_exit_network);

  int diff = new_exit_network->time - old->time; // for util

  if(diff > network_max_diff)
    network_max_diff = diff;

  network_util = network_util + diff;

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

  if(cpuQ->status == 0 && (cpuQ->counter == 0)){
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
    new_arrive_network->type = PROCESS_ARRIVE_NETWORK;
    new_arrive_network->time = old->time;

    pushPQ(pQ, new_arrive_network);
    networkQ->status = 1;
  }

  if(dont_free == 0)
    free(old);
}

int handle_end_simulation(Queue *pQ, struct event *old){
  // end the simulation b/c old->time == FIN_TIME
  // 0 will flip the control and the while loop will end, post-simulation stat put after loop

  free(old);

  return 0;
}



Queue* initQueue(){
  // create a queue and init values to 0 (empty)

  Queue *queue;
  queue = malloc(sizeof(Queue));
  //printf("sizeofQueue: %ld\n\n", sizeof(Queue));
  queue->front = 0;
  queue->rear = 0;
  queue->status = 0;
  queue->counter = 0;

  return queue;
}

void enqueue(Queue *queue, struct event *data){
  // add node to queue (append to end of list)

  struct node *n; // node to be added to queue
  n = malloc(sizeof(struct node));
  n->e = data; // node will store the event data (ex. 28, SYSTEM_ARRIVAL, 15);
  n->next = 0; // node.next = 0

  if(queue->front == 0){
    // empty queue, add first node/ element

    queue->front = n;
  }

  else{
    // prev rear element will ref the new rear element

    queue->rear->next = n;
  }

  queue->rear = n; // append to end, rear now ref to last element in queue
  queue->counter = (queue->counter + 1); // increment the counter for node/element in queue
}

struct event* dequeue(Queue *queue){

  if(queue->front == 0){
    // empty queue, return 0
    return 0;
  }

  struct event *retval;
  retval = queue->front->e; // data to be returned
  struct node *remove;
  remove = queue->front; // required to dellocate memory from removed node
  queue->front = queue->front->next; // front will now ref second element in queue (now front of queue)

  if(queue->front == 0){
    // queue is NOW empty, make rear ref 0
    queue->rear = 0;
  }

  queue->counter = (queue->counter - 1); // decremenent count nodes/ elements in queue
  free(remove);

  return retval;

}

int isEmpty(Queue *queue){
  // return 1 if queue is empty (needed in the case of when to start job immediately)

  if(queue-> front == 0)
    return 1;

  else
    return 0;
}

// ==========================================Priority-Queue =====================================================================================

// use struct node and struct queue
// main will pass an event and current queue
// always keepy the queue sorted by time, insertion of a new event is done easily, and popping the top event will require no further sorting

void pushPQ(Queue *queue, struct event *e){

  struct node *n; // rep the event data in a node
  n = malloc(sizeof(struct node));
  n->e = e;
  n->next = 0;

  struct node *traverse;
  traverse = malloc(sizeof(struct node));

  int timeEvent = n->e->time; // get the TIME of event

  if(queue->front == 0){
    // empty queue

    queue->front = n;
  }

  else if(timeEvent < queue->front->e->time){
    // insert directly to front if shorter job time than the current front's job time

    n->next = queue->front; // new front will ref old front
    queue->front = n; // queue front will ref new front
  }

  else{
    // insert in correct location
    // swap is currently ref the front node, traverse the list until find the first node with a greater time (stop 1 before)
    // insert node n in that position, list is currently sorted

    traverse = queue->front;

    while(traverse->next != 0 && traverse->next->e->time <= timeEvent){
      traverse = traverse->next;
    }

    n->next = traverse->next;
    traverse->next = n;
  }

  queue->counter = (queue->counter + 1); // increment count for priority queue


}

struct event* popPQ(Queue *queue){
  // list is sorted, pop will always remove the highest priority node, no further sorting required
  // never empty

  struct node *n;
  n = malloc(sizeof(struct node));

  n = queue->front; // "grab" the highest priority node
  queue->front = queue->front->next; // front will now ref the 2nd priority node, old 1st priority node no longer ref.

  queue->counter = (queue->counter - 1); // decrement pQueue count


  return (n->e); // returning a ptr, can free up struct later in main
}


void get_counts(Queue *pQ, Queue *cpuQ, Queue *disk1Q, Queue *disk2Q, Queue *networkQ){
// keep counts (global var) every time event is popped

  pq_count = pq_count + pQ->counter;
  cpu_count = cpu_count + cpuQ->counter;
  disk1_count = disk1_count + disk1Q->counter;
  disk2_count = disk2_count + disk2Q->counter;
  network_count = network_count + networkQ->counter;

  if(pQ->counter > max_pq_count){
    max_pq_count = pQ->counter;
    //printf("MAX %d\n", max_pq_count);
  }

  if(cpuQ->counter > max_cpu_count)
    max_cpu_count = cpuQ->counter;

  if(disk1Q->counter > max_disk1_count)
    max_disk1_count = disk1Q->counter;

  if(disk2Q->counter > max_disk2_count)
    max_disk2_count = disk2Q->counter;

  if(networkQ->counter > max_network_count){
    max_network_count = networkQ->counter;
    //printf("MAX  NET: %d\n", max_network_count);
  }

}

void printPQ(Queue *pQ, char *event_types[]){

  struct node *temp; 

    for(temp = pQ->front; temp != 0; temp = temp->next) {
      char *str = event_types[temp->e->type]; 
      printf("%d  %d %s \n", temp->e->time, temp->e->id, str);
    }
    
    puts(""); 
}




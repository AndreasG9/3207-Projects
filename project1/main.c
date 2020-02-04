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
int randomNumBetweenInterval(int min, int max);
void startSimulation(int INIT_TIME, int *clock, Queue *pQ); 
//void systemArrival(int ARRIVE_MIN, int ARRIVE_MAX, int *clock, int *counter, Queue *pQ); 
//void endSimulation(int FIN_TIME, int *clock, Queue *pQ);


// 1 for T ex. SYSTEM_EXIT 0 --> FALSE

// MOVE TO QUEUE.c            MOVE TO QUEUE.c   after completion event func. 

// typedef enum {START_SIM, SYSTEM_ARRIVAL, CPU_START, CPU_END, SYSTEM_EXIT, DISK1_ARRIVAL, DISK1_START, DISK1_END, 
//DISK2_ARRIVAL, DISK2_START, DISK2_END, NETWORK_ARRIVAL, NETWORK_START, NETWORK_END, END_SIM} TYPES;   

// struct event{
// // represent an event 

//   int id; // unique processID (each systemArrival is counted)
//   TYPES type; // event type, defined above 
//   int time; // length of job, determined by randon num. between const min and max of type 
// };

int main(int args, char *argv[]){
  
  // TEMP FIX FORMAT LATER 
  // const values (make const later), generate random num between their interval
  int SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, NETWORK_PROB, 
  CPU_MIN, CPU_MAX, DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, 
  NETWORK_MIN, NETWORK_MAX; 

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
  fscanf(fptr, "%*s %d", &QUIT_PROB);
  fscanf(fptr, "%*s %d", &NETWORK_PROB);
  fscanf(fptr, "%*s %d", &CPU_MIN);
  fscanf(fptr, "%*s %d", &CPU_MAX);
  fscanf(fptr, "%*s %d", &DISK1_MIN);
  fscanf(fptr, "%*s %d", &DISK1_MAX);
  fscanf(fptr, "%*s %d", &DISK2_MIN);
  fscanf(fptr, "%*s %d", &DISK2_MAX);
  fscanf(fptr, "%*s %d", &NETWORK_MIN);
  fscanf(fptr, "%*s %d", &NETWORK_MAX);

  fclose(fptr);

  srand(SEED); // seed is from config file 

  // int test = randomNumBetweenInterval(ARRIVE_MIN, ARRIVE_MAX);
  // printf("%d\n", test); 

  //Queue *cpuQueue;
  //cpuQueue = initQueue();               // ***** cpuQueue->status 1 for BUSY 0 for IDLE *****
  //Queue *disk1Queue;
  //disk1Queue = initQueue(); 
  //Queue *disk2Queue;
  //disk2Queue = initQueue();
  //Queue *networkQueue;
  //networkQueue = initQueue();

  // Priority queue 

  int clock = 0;  // will constantly change throughout 
  int arrivalCounter = 0; // systemArrival counter for unique processID 

  // clock 0, add to the priorityQueue : systemArrival job1 and endSimulation time  

  //startSimulation(INIT_TIME, &clock); 

  // main loop to call handler func (use switch case to select correct event generator/handler)
    // clock set to next event time       clock is now: ... 
    // poll and carry out highest priority event ... 

      // ************ 5 iterations a loop 

    // isBusy() change status 


  // TESTING PQ 
  Queue *q1; 
  q1 = initQueue(); 

  // testing 
  struct event *s; 
  s = malloc(sizeof(struct event)); 
  s->type = START_SIM; 
  s->time = 2; 

  struct event *s2;
  s2 = malloc(sizeof(struct event));
  s2->type = CPU_START;
  s2->time = 5; 
  s2->id = 3; 
 
  struct event *s3;
  s3 = malloc(sizeof(struct event));
  s3->type = CPU_END;
  s3->time = 15; 
  s3->id = 4; 
  

  struct event *s4;
  s4 = malloc(sizeof(struct event));
  s4->type = CPU_END;
  s4->time = 1; 
  s4->id = 1; 
  // enqueue(q1, s);
  // enqueue(q1, s2);

  // //printf("%d", q1->rear->e->type);

  // struct event *test;
  // test = malloc(sizeof(struct event));  
  // test = dequeue(q1); 

  // printf("%d\n", test->type); 

  // test = dequeue(q1); 
  // printf("%d\n", test->type); 

  // Queue *q5; 
  // q5 = initQueue(); 

  pushPQ(q1, s);
  pushPQ(q1, s2);
  pushPQ(q1, s3); 
  pushPQ(q1, s4);

  struct event *test = popPQ(q1);
  // then free

  printf("\n%d\n\n", test->time);
  //printf("%d\n", q1->counter); 

  //pushPQ(q1, s2); 

  //printf("\n%d\n", q1->front->e->time);

  struct node *n;

  for(n = q1->front; n != 0; n = n->next) {
      printf("%d ", n->e->time);
  }


  return 0; 
}

int randomNumBetweenInterval(int min, int max){
  return (min + rand() % (max-min+1)); // [min-max]
}

// handler func. for each event type (will actually change state of system)
// START_SIM, SYSTEM_ARRIVAL, CPU_START, CPU_END, SYSTEM_EXIT, DISK1_ARRIVAL, DISK1_START, DISK1_END, DISK2_ARRIVAL DISK2_START, DISK2_END, NETWORK_ARRIVAL, 
// NETWORK_START, NETWORK_END, END_SIM

void startSimulation(int INIT_TIME, int *clock, Queue *pQ){
  int start = INIT_TIME;
  *clock = start; // clock set to 0 

  struct event *s; 
  s = malloc(sizeof(struct event)); 
  s->type = START_SIM; 
  s->time = start; 
  //s->id = 0;  // does need an ID ??? 

  // ADD TO PRIORITY QUEUE ... THIS IS FIRST EVENT 
  pushPQ(pQ, s);
}

// void endSimulation(int FIN_TIME, int *clock, Queue *pQ){
  
//   struct event *e;
//   e = malloc(sizeof(struct event)); 
//   e->type = END_SIM; 
//   e->time = FIN_TIME; 


//   // ADD TO PRIORITY QUEUE ... SHOULD BE BOTTOM THROUGHOUT 

// }

// void systemArrival(int ARRIVE_MIN, int ARRIVE_MAX, int *clock, int *counter){
//   // generate a job arrival 
//   // increment count for each arrival 

// *counter = *counter+1; // increment 
// int num = randomNumBetweenInterval(ARRIVE_MIN, ARRIVE_MAX); 
// int nextEventTime = *clock + num; 

// struct event *sa; 
// sa = malloc(sizeof(struct event)); 
// sa->type = SYSTEM_ARRIVAL; 
// sa->time = num; // [ARRIVE_MIN - ARRIVE_MAX]
// sa->id = *counter; 

// // ADD TO PRIORITY QUEUE

// }



/*
Project 1
Andreas Gagas
main.c - discrete event simulator 
*/

#include <stdio.h>
#include <stdlib.h>
#include "queue.c" // queue implementation as linkedlist of nodes .. and added implementation of priority queue (min-heap)
// structs: node struct and queue struct ... 
// methods: Queue* initQueue(), void enqueue(Queue *queue), int dequeue(Queue *queue), int isEmpty(Queue *queue)
// ADD implementation of priority aueue in same file 

//void readConfigFile(); // const values based on thoses read from the configuration file 
int randomNumBetweenInterval(int min, int max);

// 1 for T ex. SYSTEM_EXIT 0 --> FALSE

typedef enum {START_SIM, SYSTEM_ARRIVAL, CPU_START, CPU_END, SYSTEM_EXIT, DISK1_START, DISK1_END, DISK2_START, DISK2_END, NETWORK_START, NETWORK_END, END_SIM} TYPES;
//                0         1         2          3      4       5           6           7           8           9                10        11             12

struct event{
// represent an event 

  int id; // unique processID (each systemArrival is counted)
  TYPES type; // event type, defined above 
  int time; // length of job
};

int main(int args, char *argv[]){
  
  // TEMP FIX FORMAT LATER 
  // const values (make const later), generate random num between their interval
  int SEED, INT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, NETWORK_PROB, 
  CPU_MIN, CPU_MAX, DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX, 
  NETWORK_MIN, NETWORK_MAX; 

  FILE *fptr = NULL;
  fptr = fopen("CONFIG1.conf", "r"); // change to command line input later 

  if(fptr == NULL)
    puts("main.c: could not open file");
  
  // read the int value from file (follows this format), store in corresponding event variable 
  fscanf(fptr, "%*s %d", &SEED);
  fscanf(fptr, "%*s %d", &INT_TIME);
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
  //cpuQueue = initQueue();
  //Queue *disk1Queue;
  //disk1Queue = initQueue(); 
  //Queue *disk2Queue;
  //disk2Queue = initQueue();
  //Queue *networkQueue;
  //networkQueue = initQueue();

  // Priority queue 
  // clock 0, add initial event (sim_start) to proprityQueue

  //int arrivalCounter = 0; // pass into event handler 

  // main loop to call handler func (use switch case)
    // clock set to next event time 
    // poll and carry out highest priority event ... 

  return 0; 
}

int randomNumBetweenInterval(int min, int max){
  return (min + rand() % (max-min+1)); // [min-max]
}

// handler func. for each event type (will actually change state of system)
// START_SIM, SYSTEM_ARRIVAL, CPU_START, CPU_END, SYSTEM_EXIT, DISK1_START, DISK1_END, DISK2_START, DISK2_END, NETWORK_START, NETWORK_END, END_SIM


// int systemArrival(int *counter, ...){
//   // increment count for each arrival 
// struct event *sa; 
// sa->
// ADD ADD ADD 
//   return 0; 
// }



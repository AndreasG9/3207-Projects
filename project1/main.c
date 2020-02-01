/*
Project 1
Andreas Gagas
main.c - discrete event simulator 
*/

#include <stdio.h>
#include <stdlib.h>
#include "queue.c" // queue implementation as linkedlist of nodes .. 
// structs: node struct and queue struct ... 
// methods: Queue* initQueue(Queue *queue), void enqueue(Queue *queue), int dequeue(Queue *queue), int isEmpty(Queue *queue)

//void readConfigFile(); // init values based on thoses read from the configuration file 


int main(int args, char *argv[]){
 
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

  // srand(seed); seed is from config file 

  Queue *q1; 
  q1 = initQueue();

  // for(int i=1; i<=1000; ++i){
  //   enqueue(q1, i);
  // }

  // printf("%d\n", q1->front->data);
  // printf("%d\n", q1->rear->data);

  // int data = dequeue(q1);

  // printf("%d\n", data); 
  
  // data = dequeue(q1); 

  // printf("%d\n", data); 

  //printf("%d\n", q1->rear->data);

  // data = dequeue(q1); 

  // printf("%d\n", data); 


  //int data = dequeue(q1); 

  //printf("Counter: %d\n", q1->counter);

  return 0; 
}






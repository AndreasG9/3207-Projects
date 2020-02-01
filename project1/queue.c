// queue implementation (similar to linked-list of nodes)

#include <stdio.h>
#include <stdlib.h>

struct node{
  // a node will hold the data and ref to next element in list (to right)

  int data; 
  struct node *next; 
};

typedef struct queue{

  int counter; // num of jobs 
  //int capacity; // when to reallocate ... DO NOT NEED
  int status; // currently handling???? an item  1|0  ADD 
  struct node *front;
  struct node *rear;
}Queue; 

Queue* initQueue(){
  // create and queue and init values to 0 (empty)

  Queue *queue; 
  queue = malloc(sizeof(Queue));
  //printf("sizeofQueue: %ld\n\n", sizeof(Queue));
  queue->front = 0;
  queue->rear = 0; 
  queue->status = 0; 
  queue->counter = 0; 

  return queue; 
}

void enqueue(Queue *queue, int value){
  // add element to queue (append to end of list)

  struct node *element; // node to be added to queue  
  element = malloc(sizeof(struct node));
  element->data = value; 
  element->next = 0; // node.next = 0 

  if(queue->front == 0){
    // empty queue, add first element 

    queue->front = element; 
  }

  else{
    // prev rear element will ref the new rear element 

    queue->rear->next = element; 
  }

  queue->rear = element; // append to end, rear now ref to last element in queue 
  queue->counter = (queue->counter + 1); // increment the counter for node/element in queue 
  //printf("counter now: %d\n", queue->counter);
}

int dequeue(Queue *queue){

  if(queue->front == 0){
    // empty queue, return 0 
    return 0; 
  }

  int data = queue->front->data; // data to be returned 
  struct node *remove; 
  remove = queue->front; // required to dellocate memory from removed node 
  queue->front = queue->front->next; // front will now ref second element in queue (now front of queue)
  
  if(queue->front == 0){
    // queue is NOW empty, make rear ref 0 
    queue->rear = 0; 
  }

  queue->counter = (queue->counter - 1); // decremenent count nodes/ elements in queue 
  free(remove); 
  
  return data; 
}

int isEmpty(Queue *queue){
  // return 1 if queue is empty (needed in the case of when to start job immediately)

  if(queue-> front == 0)
    return 1; 

  else
    return 0; 
}
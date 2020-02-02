/*
Project 1
Andreas Gagas
queue.c - queue implementation (linked-list of nodes) for 4 FIFO queues 
*/

// struct event data fields: int id, TYPES type, int time 

struct node{
  // a node will hold the data/job and ref to next node in list (to right)

  //int data; 
  struct event *e; // struct event data fields: int id, TYPES type, int time 
  struct node *next; 
};

typedef struct queue{

  int counter; // num of jobs 
  //int capacity; // when to reallocate ... DO NOT NEED
  int status; // currently handling an item ... 1 for busy 
  struct node *front;
  struct node *rear;
}Queue; 

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






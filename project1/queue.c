/*
Project 1
Andreas Gagas
queue.c - queue implementation (linked-list of nodes) for 4 FIFO queues 

priority queue not min heap, trying sorted list, no re-sorted required 
*/

typedef enum {START_SIM, SYSTEM_ARRIVAL, CPU_START, CPU_END, SYSTEM_EXIT, 
//                0             1             2        3        4
DISK1_ARRIVAL, DISK1_START, DISK1_END, DISK2_ARRIVAL, DISK2_START, DISK2_END, 
//      5            6           7          8              9           10
NETWORK_ARRIVAL, NETWORK_START, NETWORK_END, END_SIM} TYPES;
//      11               12           13        14
    

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
  //int capacity; // don't need this, each node memory is allocated
  int status; // currently handling an item ... when event is in CPU switch to 1 for busy  
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

  struct node *n; 
  n = malloc(sizeof(struct node));

  n = queue->front; // "grab" the highest priority node 
  queue->front = queue->front->next; // front will now ref the 2nd priority node, old 1st priority node no longer ref. 

  return (n->e); // returning a ptr, can free up struct later in main 
}




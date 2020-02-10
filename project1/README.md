
# Discrete Event Simulation				
Andreas Gagas

  This is an event driven simulation, events are defined by their unique ID, type, and time of when the event will occur. 
In general, events describe the current state of the process, such as process_exit_cpu, referencing the process is ready 
to exit the system, do I/O on disk, or require the assistance of a network. Simple to trace the span of a single process, 
this program will simulate the movements of a multitude of processes, requiring the use of FIFO queues and overall priority 
queue to keep the system in order. 

  The FIFO queues were implemented as a linked list of nodes, with each node containing a struct event (time, ID, type) 
and a reference to the next node, with a head/front reference to the first node in the list. An enqueue of an event, was 
appended to the end, and a dequeue of an event, results in the head ref. node referencing the next in queue node. 
A Queue -> node front -> nodes -> each node has struct event. 

  The priority queue was implemented as a SORTED linked-list of nodes, with the time of the event being the priority (lowest first). 
Insertion was simple, if the new event had the lowest time it was inserted directly to the front, otherwise you traverse the list and 
insert to the left of node who time is larger. Popping an event, only requires removing the first event, no re-sorting required. 

  As noted above, events have an ID (generated count with each NEW arrival in the system), type (used ENUM allowed for readability), 
and a time, which when popped from the priority queue, represents the current system time. With each event popped from the priority queue,
a specific event handler is called of the type popped. The handler is responsible for generating the next event (except for arrival, which 
generates itself, along with a unique ID), with a new event time, and generating other events if needed as queueing is handled. 
For example, handle_process_exit_cpu will determine the next state of the process, such as using the disk for I\O. During the event 
generation and its addition to the priority queue (or enqueuing onto a DISK queue), the CPU component is open for the next 
event in the cpu queue, and handle_process_exit_cpu is responsible for ALSO generating a process_arrive_cpu event. Also to be noted, an open component will have a status of IDLE (ex. cpuQueue->status == 0), allowing for the process to "change its state". 

“At time 354, process 9, exits the cpu.”
“At time 354, process 4, arrives at cpu.”
This technique is used for the other exit events. 

  Events will continue to be popped, and corresponding event handlers called until the system time matches the FIN_TIME, a const value 
read from a config file. At which, the loop will exit and a series of STAT data, based on the simulation, will be output to a file 
for analysis. 

  Overall, a long lab, event handlers took a while, as did figuring where to compute the stats, which resulted in a bunch of 
 global variables to track difference stats. For readability, those global variables are terrible, I didn’t want too many 
 parameters in my functions, as I ended up tracking maybe around 20 global variables, just to keep various counts and sums. 
 And debugging was also responsible for the bulk of the lab,  just with the length of the program and its various paths. 

/*
Project 4
Andreas Gagas
signaling_MP.c - Signaling with multi-processes
*/

#include <stdlib.h>
#include <stdio.h> 
#include <signal.h> 
#include <time.h> 
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h> 
#include <sys/times.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h> 
//#define _POSIX_C_SOURCE 1


void signal_generating(); 

void signal_handling_sigusr1();
void signal_handling_sigusr2();

void signal_handling_handler(int signal); 
void reporting(); 

void sleep_random_interval(double low, double high); 
int random_signal(); 
int time_details(); 

//void parent_function(pid_t child_pid); // TESTING 
//void signal_handling_handler2(int signal); 

void block_sigusr1();
void block_sigusr2();

struct shared_val{

  int sigusr1_recieved_counter; 
  int sigusr2_recieved_counter; 
  int sigusr1_sent_counter; 
  int sigusr2_sent_counter; 

  pthread_mutex_t lock_sigusr1_sent; 
  pthread_mutex_t lock_sigusr2_sent; 
  pthread_mutex_t lock_sigusr1_recieved; 
  pthread_mutex_t lock_sigusr2_recieved; 
  
} *shared_ptr; // CHILD can access shm_ptr 


void clean_up(int signal); 

// typedef unsigned long sigset_t; // kept getting sigset_t undefined, maybe this will work


int main (int argc, char *argv[]){


  // INIT shared_val struct
  // CHILD has access ptr to struct shared_val
  int shm_id; 
  pthread_mutexattr_t attr; 
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); 

  shm_id = shmget(IPC_PRIVATE, sizeof(struct shared_val), IPC_CREAT | 0666);

  if(shm_id < 0){
    puts("shmget() error");
    exit(1); 
  }

  shared_ptr = (struct shared_val*) shmat(shm_id, NULL, 0); // attach 

  if(shared_ptr == (struct shared_val *) -1){
    puts("shmat() error");
    exit(1); 
  }

  // init counters to 0 
  shared_ptr->sigusr1_sent_counter = 0; 
  shared_ptr->sigusr2_sent_counter = 0; 
  shared_ptr->sigusr1_recieved_counter = 0;
  shared_ptr->sigusr2_recieved_counter = 0; 

  // init locks with set attribute 
  pthread_mutex_init(&(shared_ptr->lock_sigusr1_sent), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr2_sent), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr1_recieved), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr2_recieved), &attr); 

  //printf("INIT %d %d %d %d\n", shared_ptr->sigusr1_sent_counter, shared_ptr->sigusr2_sent_counter, shared_ptr->sigusr1_recieved_counter, shared_ptr->sigusr2_recieved_counter);  




  // spawn child processes ... 

  pid_t pids[8]; 

  for(int i = 0; i<8; ++i){

    pids[i] = fork(); 

    if(pids[i] == -1){
      fprintf(stderr, "%s\n", "fork() failed");
      exit(1); 
    }

    else if(pids[i] == 0){
      // [0-1] sigusr1 handling 
      // [2-3] sigusr2 handling
      // [4-6] signal generating 
      // [7] reporting 

      if(i == 0 || i == 1)
        signal_handling_sigusr1(); 
      else if(i == 2 || i == 3)
        signal_handling_sigusr2(); 
      else if(i == 4 || i == 5 || i == 6)
        signal_generating(); 
      else if(i == 7)
          reporting(); 
    }

    else{
      if((i+1) == 8){
        wait(NULL); 
        puts("!"); 
        exit(1); 
      }

    }

  }



  signal(SIGINT, clean_up); // testing, ctrl-c 

  


  // VARIABLE, for testing parent wait x amount seconds then exit program 
  while(1){
    sleep(5);


    shmdt(shared_ptr); 
    exit(1); // testing 5 sec, exit  
  }  
  //shmdt(shared_ptr); 

  // SIGTERM HANDLER TODO, program done execution 

  // for(int i =0; i<8; ++i){
  //   kill(pids[i], ); 
  // }

}




void signal_generating(){
  // indefinitely loop, select SIGUSR1 or SIGUSR2 to send to processes
  // increment appropiate counter 

  printf("generating %d\n", getpid()); 

  int j = 0; 

  while(1){
    
    sleep_random_interval(.01, .1); // sleep [.01-.1] 
    int signal = random_signal(); 
    //printf("signal hello : %d\n", signal); 

    exit(1); 


    kill(0, signal); // send the signal to "its peers"/ all child processes (signal is either SIGUSR1 or SIGUSR2)

    if(signal == SIGUSR1){
      // increment global counter for sigusr1 sent 

      pthread_mutex_lock(&(shared_ptr->lock_sigusr1_sent)); 
      shared_ptr->sigusr1_sent_counter++; 
      pthread_mutex_unlock(&(shared_ptr->lock_sigusr1_sent)); 
    }

    else{
      // increment global counter for sigusr2 sent 

      pthread_mutex_lock(&(shared_ptr->lock_sigusr2_sent)); 
      shared_ptr->sigusr2_sent_counter++; 
      pthread_mutex_unlock(&(shared_ptr->lock_sigusr2_sent)); 
    }

    // shmdt(shared_ptr); 

  }

  printf("counter sigusr1: %d\n", shared_ptr->sigusr1_sent_counter); 
  printf("counter sigusr2: %d\n", shared_ptr->sigusr2_sent_counter); 
}

void sleep_random_interval(double low, double high){
  // generate random between low(.01) and high(.1) 
  // sleep for that time 
  
  int num = time(NULL) ^ getpid(); // unique seed 
  srand(num); 

  double random_double = (double)rand() * (low - high) / (double)RAND_MAX + high; // expected to be between [.01 - .1]  
  printf("%lf\n", random_double); 

  int milliseconds = (random_double * 1000); // get milliseconds 

  //sleep(random_double); 
  usleep(milliseconds); 
}

int random_signal(){
  // generate random number between 0 and 1, if 0 SIGUSR1, else SIGUSR2 

  srand(time(NULL)); 

  double random_int = rand() % 2; // [0-1]

  if(random_int == 0)
    return SIGUSR1;
  else
    return SIGUSR2; 
}

int time_details(){

  time_t system_time = time(NULL); // current system time  

  // save old system_time 


  // difference between system times 
  struct timespec start, stop;
  double difference; 
  
  // if(clock_gettime(CLOCK_REALTIME, &start)) == -1){
  //   puts("clock_gettime() error");
  //   exit(1); 
  // }

  // if(clock_gettime(CLOCK_REALTIME, &stop)) == -1){
  //   puts("clock_gettime() error");
  //   exit(1); 
  // }

  // calculate 

}


void signal_handling_sigusr1(){
  // indefinitely loop (will be 2 child processes in this function) ... 
  // 2 for SIGUSR1 (PID INDEXES 0 AND 1),
  // SIGNAL sent, goes to all 4, block other 

  printf("sigusr1 handling %d\n", getpid()); 

  block_sigusr2(); // block other signal type as signal generating sends signal to all child processes 
  signal(SIGUSR1, signal_handling_handler); // set signal 

  while(1){
    sleep(1); 
  }

}

void signal_handling_sigusr2(){
  // indefinitely loop (will be OTHER 2 child processes in this function) ... 
  //  2 for SIGUSR2 (PID INDEXES 2 AND 3)

  printf("sigusr2 handling %d\n", getpid()); 

  block_sigusr1(); // block other signal type 
  signal(SIGUSR2, signal_handling_handler); // set signal 

  while(1){
    sleep(1); 
  }

}




void block_sigusr1(){
  // 

  sigset_t sigset; 
  sigemptyset(&sigset); 
  sigaddset(&sigset, SIGUSR1); // add SIGUSR1 to set 

  int retval = sigprocmask(SIG_BLOCK, &sigset, NULL); // mask/ filter out SIGUSR1 

  if(retval == -1){
    puts("sigprocmask error");
    exit(1); 
  }
}

void block_sigusr2(){
  // identical to block sigusr1, but block sigusr2 

  sigset_t sigset; 
  sigemptyset(&sigset); 
  sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set 
  sigprocmask(SIG_BLOCK, &sigset, NULL); // mask/ filter out SIGUSR1 
}


void signal_handling_handler(int signal){

  if(signal == SIGUSR1){
    // signal arrived, increment counter in shared memory segment 

    pthread_mutex_lock(&(shared_ptr->lock_sigusr1_recieved));
    shared_ptr->sigusr1_recieved_counter++;
    pthread_mutex_unlock(&(shared_ptr->lock_sigusr1_recieved));

  }

  else if(signal == SIGUSR2){
    // signal arrived, increment counter 

    pthread_mutex_lock(&(shared_ptr->lock_sigusr2_recieved));
    shared_ptr->sigusr2_recieved_counter++;
    pthread_mutex_unlock(&(shared_ptr->lock_sigusr2_recieved));
  }

}


void reporting(){
  // single child process here
  // recieves both SIGUSR1 and SIGUSR2 
  // keep count 

  // every 10 signals, REPORT: system time, shared counters (SIGUSR1/SIGUSR2: sent, recieved), average time interval between signal types 

  printf("reporting %d\n", getpid()); 

  // FILE *fptr = fopen("data.log", "w"); 
  // fclose(fptr); 

  while(1){


  }



}

void clean_up(int signal){

  puts("detached from shared memory region"); 

  if(signal == SIGINT){
    shmdt(shared_ptr); 
  }
}

// void signal_handling_handler2(int signal){

//   //   if(signal == SIGUSR2){
//   //   // signal arrived, increment counter 

//   //   pthread_mutex_lock(&lock_sigusr2_recieved);
//   //   ++sigusr2_recieved_counter;
//   //   pthread_mutex_unlock(&lock_sigusr2_recieved); 
//   // }

// }
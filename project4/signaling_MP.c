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

// TODO 
// parent process
// 4 signal handling processes
// 3 signal generating processes
// 1 reporting process


void signal_generating(); 
void signal_handling(); 
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

void clean_up(int signal){

  puts("detached from shared memory region"); 

  if(signal == SIGINT){
    shmdt(shared_ptr); 
  }
}



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


  // testing 
  //signal(SIGUSR1, signal_generating_handler1); 
  // spawn 3 signal generating processes 

  // spawn child processes ... 
  pid_t pids[8]; 

  for(int i =0; i<8; ++i){

    if(pids[i] == -1){
      fprintf(stderr, "%s\n", "fork() failed");
      exit(1); 
    }

    else if(pids[i] == 0){
      
    }



  }



  // for(int j = 0; j<3; ++j){

  //   pid_t pid = fork(); 

  //   if(pid == -1){
  //     fprintf(stderr, "%s\n", "signal generating: could not fork()");
  //     exit(1); 
  //   }

  //   else if(pid == 0){
  //     // CHILD, set function to handle signal 
  //     signal(SIGUSR1, signal_handling_handler1);
  //     signal(SIGUSR2, signal_handling_handler2); 
  //     //signal_generating_handler(); // 3 child processes will sit here and generate signals  

  //     while(1){
  //       sleep(1); 
  //     }

  //   }

  //   else{
  //     // PARENT, will send signal to child 
  //     sleep(1);
  //     parent_function(pid); 
  //   }
  // } 

  // spawn 4 signal handling processes 
  // for(int i = 0; i<4; ++i){
    
  // }

  // spawn reporting process

  // control execution duration ... 


  signal(SIGINT, clean_up); 

  while(1){
    sleep(1); 
  }

  //shmdt(shared_ptr); 

}


// void parent_function(pid_t child_pid){
//   // TESTING 

//   kill(child_pid, SIGUSR1); 
//   puts("sent SIGUSR1 to child"); 
//   waitpid(child_pid, NULL, 0); 
// }



void signal_generating(){
  // indefinitely loop, select SIGUSR1 or SIGUSR2 to send to processes
  // increment appropiate counter 

  printf("hello i am %d\n", getpid()); 

  int j =0; 

  //while(1){
    while(j<10){

    
    sleep_random_interval(.01, .1); // sleep [.01-.1] 
    int signal = random_signal(); 
    printf("signal hello : %d\n", signal); 

    exit(1); 


    kill(0, signal); // send the signal to "its peers"/ all child processes (signal is either SIGUSR1 or SIGUSR2)

    if(signal == SIGUSR1){
      // increment global counter for sigusr1 sent 

      pthread_mutex_lock(&(shared_ptr->lock_sigusr1_sent)); 
      shared_ptr->sigusr1_sent_counter++; 
      pthread_mutex_unlock(&(shared_ptr->lock_sigusr1_sent)); 

      // pthread_mutex_lock(&lock_sigusr1_sent);
      // ++sigusr1_sent_counter;
      //pthread_mutex_unlock(&lock_sigusr1_sent);
    }

    else{
      // increment global counter for sigusr2 sent 

      pthread_mutex_lock(&(shared_ptr->lock_sigusr2_sent)); 
      shared_ptr->sigusr2_sent_counter++; 
      pthread_mutex_unlock(&(shared_ptr->lock_sigusr2_sent)); 
    }


    ++j; 

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

void signal_handling(){
  // indefinitely loop (will be 4 child processes in this function) ... 
  // 2 for SIGUSR1, 2 for SIGUSR2
  // SIGNAL sent, goes to all 4, block other 

  int i =0; 

 // while(1){

   while(i<10){

    // block other, call signal_handling 


    //signal_handling_handler(signal); 

    
     ++i; 
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

  
  sigset_t sigset; 
  sigemptyset(&sigset); 
  sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set 
  sigprocmask(SIG_BLOCK, &sigset, NULL); // mask/ filter out SIGUSR1 

}




void signal_handling_handler(int signal){

  if(signal == SIGUSR1){
    // signal arrived, increment counter 

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

  FILE *fptr = fopen("data.log", "w"); 

  while(1){




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
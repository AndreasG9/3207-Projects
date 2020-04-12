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
#include <unistd.h>


// TODO 
// parent process
// 4 signal handling processes
// 3 signal generating processes
// 1 reporting process

pthread_mutex_t lock_sigusr1_sent; 
pthread_mutex_t lock_sigusr2_sent; 

pthread_mutex_t lock_sigusr1_recieved; 
pthread_mutex_t lock_sigusr2_recieved; 

void signal_handling_handler1(int sig); 
void signal_handling_handler2(int sig); 

int sigusr1_recieved_counter; 
int sigusr2_recieved_counter; 

void sleep_random_interval(double low, double high); 
void signal_generating_handler1(int sig); 

int sigusr1_sent_counter; 
int sigusr2_sent_counter; 


int main (int argc, char *argv[]){

  sigusr1_sent_counter = 0;
  sigusr2_sent_counter = 0; 
  sigusr1_recieved_counter = 0;
  sigusr2_recieved_counter = 0; 

  pthread_mutex_init(&lock_sigusr1_sent, NULL); 
  pthread_mutex_init(&lock_sigusr2_sent, NULL); 
  pthread_mutex_init(&lock_sigusr1_recieved, NULL); 
  pthread_mutex_init(&lock_sigusr2_recieved, NULL); 
 

  // spawn 3 signal generating processes 
  for(int j = 0; j<3; ++j){

  }

  // spawn 4 signal handling processes 
  for(int i = 0; i<4; ++i){
    
  }

  // spawn reporting process

  // control execution duration ... 

}


void signal_generating_handler1(int sig){
// indefinitely loop, select SIGUSR1 or SIGUSR2 to send to processes. 

  while(1){

    sleep_random_interval(.01, .1); // sleep [.01-.1] 
    

  }
}

void sleep_random_interval(double low, double high){
  // generate random between low(.01) and high(.1) 
  // sleep for that time 

  srand(time(NULL)); 
  double random_double = (double)rand() * (low - high) / ((double)RAND_MAX + high); 
  // printf("%lf", random_double); 

  sleep(random_double); 
}


void signal_handling_handler1(int sig){

  if(sig == SIGUSR1){
    // signal arrived, increment counter 

    pthread_mutex_lock(&lock_sigusr1_recieved);
    ++sigusr1_recieved_counter;
    pthread_mutex_unlock(&lock_sigusr1_recieved); 
  }

}

void signal_handling_handler2(int sig){

    if(sig == SIGUSR2){
    // signal arrived, increment counter 

    pthread_mutex_lock(&lock_sigusr2_recieved);
    ++sigusr2_recieved_counter;
    pthread_mutex_unlock(&lock_sigusr2_recieved); 
  }

}
/*
Project 4
Andreas Gagas
signaling_MT.c - Signaling with multi-threads 
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
#include <sys/types.h>
#define NUM_THREADS 8

void* generating_thread(void* arg);
void* signal_handling_sigusr1(void *arg);
void* signal_handling_sigusr2(void *arg);


void sleep_random_interval(double low, double high); 
int random_signal(); 


int sigusr1_recieved_counter; 
int sigusr2_recieved_counter; 
int sigusr1_sent_counter; 
int sigusr2_sent_counter;  

pthread_mutex_t lock_sigusr1_sent; 
pthread_mutex_t lock_sigusr2_sent; 
pthread_mutex_t lock_sigusr1_recieved; 
pthread_mutex_t lock_sigusr2_recieved; 

pthread_t threads[NUM_THREADS];


int main (int argc, char *argv[]){


  // INIT 
  sigusr1_recieved_counter = 0;
  sigusr2_recieved_counter = 0; 
  sigusr1_sent_counter = 0;
  sigusr2_sent_counter = 0; 

  



  // create threads 
  // pthread_t threads[NUM_THREADS];

  for(int i = 0; i<NUM_THREADS; ++i){


    if(i == 0 || i == 1){
      // assigned SIGUSR1 handling 
      if(pthread_create(&threads[i], NULL, signal_handling_sigusr1, NULL) != 0){
        fprintf(stderr, "%s\n", "Failed to SIGUSR1 handling thread"); 
        exit(1); 
      }
    }

    if(i == 2 || i == 3){
      // assigned SIGUSR2 handling 
      if(pthread_create(&threads[i], NULL, signal_handling_sigusr2, NULL) != 0){
        fprintf(stderr, "%s\n", "Failed to SIGUSR2 handling thread"); 
        exit(1); 
      }
    }

    if(i == 4 || i == 5 || i == 6){
      // 3 generating threads 
      if(pthread_create(&threads[i], NULL, generating_thread, NULL) != 0){
        fprintf(stderr, "%s\n", "Failed to generating thread"); 
        exit(1); 
      }
    }

  }

    // while(1){

    //   if(getchar())
    //     break; 
    // }

}


void* generating_thread(void* arg){

  //while(1){
    
  //  sleep_random_interval(.01, .1); // sleep [.01-.1] 

    //int signal = random_signal(); 

    puts("gen");

    // // send the signal to ALL other threads 
    // if(signal == 1){
      
    //   for(int i =0; i<NUM_THREADS; ++i){
    //     pthread_kill(threads[i],SIGUSR1);

    //     puts("sent 1");
    //   }
    // }

    // else{
    //   // send SIGUSR2

    //   for(int i =0; i<NUM_THREADS; ++i){
    //     pthread_kill(threads[i],SIGUSR2);

    //     puts("sent 2");
    //   }
    // }

 // }

}

void* signal_handling_sigusr1(void *arg){
  //puts("sig1");




}


void* signal_handling_sigusr2(void *arg){
  //puts("sig2");



}





void sleep_random_interval(double low, double high){
  // generate random between low(.01) and high(.1) 
  // sleep for that time 
  
  int num = time(NULL) ^ getpid(); // unique seed 
  srand(num); 

  double random_double = (double)rand() * (low - high) / (double)RAND_MAX + high; // expected to be between [.01 - .1]  
 // printf("%lf\n", random_double); 

  double microseconds = (random_double * 1000000); // get microseconds

  //printf("I (%d) will sleep for %d microseconds\n", getpid(), microseconds); 
  struct timespec start, stop, sleeep, rem;

  sleeep.tv_sec = 0; // [.01-.1] seconds, always under 1  
  sleeep.tv_nsec = microseconds * 1000; // now nanoseconds 

  // printf("I (%d) will sleep for %ld nanoseconds\n", getpid(), sleeep.tv_nsec); 
  //printf("I (%d) will sleep for %lf seconds\n", getpid(), (sleeep.tv_nsec/1e9)); 

  nanosleep(&sleeep, &rem);

}

int random_signal(){
  // generate random number between 0 and 1, if 0 SIGUSR1, else SIGUSR2 

  int seed = getpid() ^ time(NULL); 
  srand(seed); 

  int random_int = (rand() % 2) + 1; // [1-2]

  if(random_int == 1)
    return 1; 
  else
    return 2; 
  
}






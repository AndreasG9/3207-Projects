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
void* reporting_thread(void *arg);


void sleep_random_interval(double low, double high); 
int random_signal(); 

void block_both();
void block_signal(int signal);

int sigusr1_recieved_counter; 
int sigusr2_recieved_counter; 
int sigusr1_sent_counter; 
int sigusr2_sent_counter;  

pthread_mutex_t lock_sigusr1_sent; 
pthread_mutex_t lock_sigusr2_sent; 
pthread_mutex_t lock_sigusr1_received; 
pthread_mutex_t lock_sigusr2_received; 

pthread_t threads[NUM_THREADS];

int main (int argc, char *argv[]){

  // INIT 
  sigusr1_recieved_counter = 0;
  sigusr2_recieved_counter = 0; 
  sigusr1_sent_counter = 0;
  sigusr2_sent_counter = 0; 

  int res = pthread_mutex_init(&lock_sigusr1_sent, NULL);
  int res2 = pthread_mutex_init(&lock_sigusr2_sent, NULL);
  int res3 = pthread_mutex_init(&lock_sigusr1_received, NULL);
  int res4 = pthread_mutex_init(&lock_sigusr2_received, NULL);

  if(res == -1 || res2 == -2 || res3 == -1 || res4 == -1){
    puts("pthread_mutex_init error");
    exit(1);
  }


  // create threads 
  // pthread_t threads[NUM_THREADS];

  for(int i = 0; i<NUM_THREADS; ++i){

    if(i == 0 || i == 1){
      // assigned SIGUSR1 handling thread 
      if(pthread_create(&threads[i], NULL, signal_handling_sigusr1, NULL) != 0){
        fprintf(stderr, "%s\n", "Failed to create SIGUSR1 handling thread"); 
        exit(1); 
      }
    }

    if(i == 2 || i == 3){
      // assigned SIGUSR2 handling thread 
      if(pthread_create(&threads[i], NULL, signal_handling_sigusr2, NULL) != 0){
        fprintf(stderr, "%s\n", "Failed to create SIGUSR2 handling thread"); 
        exit(1); 
      }
    }

    if(i == 4 || i == 5 || i == 6){
      // 3 generating threads 
      if(pthread_create(&threads[i], NULL, generating_thread, NULL) != 0){
        fprintf(stderr, "%s\n", "Failed to gcreate enerating thread"); 
        exit(1); 
      }
    }

    if(i == 7){
      // reporting thread
      if(pthread_create(&threads[i], NULL, reporting_thread, NULL) != 0){
        fprintf(stderr, "%s\n", "Failed to create reporting thread"); 
        exit(1); 
      }
    }

  }

    //block_both(); // "PARENT"/ main thread, block both usr SIGNALS 

  puts("main thread");
  block_both();

  // sigset_t sigset; 
  // sigemptyset(&sigset); 
  // sigaddset(&sigset, SIGUSR1);
  // sigaddset(&sigset, SIGUSR2);
  // pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    // while(1){

    //   if(getchar())
    //     break; 
    // }

    while(1){
      sleep(1);
    }

}


void* generating_thread(void* arg){

  block_both();

  // sigset_t sigset; 
  // sigemptyset(&sigset); 
  // sigaddset(&sigset, SIGUSR1);
  // sigaddset(&sigset, SIGUSR2);
  // pthread_sigmask(SIG_BLOCK, &sigset, NULL);

  int j = 0; 
  puts("gen");

  while(j<2){
    ++j; 
    
    sleep_random_interval(.01, .1); // sleep [.01-.1] 

    int signal = random_signal(); 

    signal = 1; 

    // send the signal to ALL other threads (to mirror processes approach)
    if(signal == 1){
      
      for(int i =0; i<NUM_THREADS; ++i)
        pthread_kill(threads[i], SIGUSR1);  


      // pthread_kill(threads[0], SIGUSR1);     
      // pthread_kill(threads[1], SIGUSR1);    
      // pthread_kill(threads[7], SIGUSR1); 


     // }


      // safely increment counter for sigusr1 sent 
      pthread_mutex_lock(&lock_sigusr1_sent); 
      ++sigusr1_sent_counter;
      pthread_mutex_unlock(&lock_sigusr1_sent);

      printf("SIG1: %d\n", sigusr1_sent_counter);
    }

    else{
      // send SIGUSR2

      for(int i =0; i<NUM_THREADS; ++i)
        pthread_kill(threads[i],SIGUSR2);


      //safely increment counter for sigusr1 sent 
      pthread_mutex_lock(&lock_sigusr2_sent); 
      ++sigusr2_sent_counter;
      pthread_mutex_unlock(&lock_sigusr2_sent);

      printf("SIG2: %d\n", sigusr1_sent_counter);
    }

  }




}

void* signal_handling_sigusr1(void *arg){
  // 2 threads here 
  // sigwait() until get a SIGUSR1 signal

  block_both(); // block both usrs signals 

  // sigset_t sigset; 
  // sigemptyset(&sigset); 
  // sigaddset(&sigset, SIGUSR2);
  // pthread_sigmask(SIG_BLOCK, &sigset, NULL);

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1); 
  int return_val = 0;
  int signal;

  while (1){

	  return_val = sigwait(&sigset, &signal);

	  if (signal == SIGUSR1){
	    //puts("thread handling recieved SIGUSR1");


      
	  }


  }



}

void* signal_handling_sigusr2(void *arg){
  // 2 threads here 
  // sigwait() until get a SIGUSR2 signal

  block_both(); // block both usr signals 

  // sigset_t sigset; 
  // sigemptyset(&sigset); 
  // sigaddset(&sigset, SIGUSR1);
  // pthread_sigmask(SIG_BLOCK, &sigset, NULL);


  while(1){
    sleep(1);
  }



}

void* reporting_thread(void *arg){

  block_both();
  // sigset_t sigset; 
  // sigemptyset(&sigset); 
  // sigaddset(&sigset, SIGUSR1);
  // sigaddset(&sigset, SIGUSR2);
  // pthread_sigmask(SIG_BLOCK, &sigset, NULL);


  while(1){
    sleep(1);
  }
}

void sleep_random_interval(double low, double high){
  // generate random between low(.01) and high(.1) 
  // sleep for that time 
  
  int num = time(NULL) ^ pthread_self(); // unique seed 
  srand(num); 

  double random_double = (double)rand() * (low - high) / (double)RAND_MAX + high; // expected to be between [.01 - .1]  
 // printf("%lf\n", random_double); 

  double microseconds = (random_double * 1000000); // get microseconds

  //printf("I (%d) will sleep for %d microseconds\n", getpid(), microseconds); 
  struct timespec start, stop, sleeep, rem;

  sleeep.tv_sec = 0; // [.01-.1] seconds, always under 1  
  sleeep.tv_nsec = microseconds * 1000; // now nanoseconds 

  //printf("I will sleep for %lf seconds\n", (sleeep.tv_nsec/1e9)); 

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



void block_both(){
  // "PARENT"/ main thread will want to block both 

  sigset_t sigset; 
  sigemptyset(&sigset); 
  sigaddset(&sigset, SIGUSR1);
  sigaddset(&sigset, SIGUSR2);

  pthread_sigmask(SIG_BLOCK, &sigset, NULL);
}

void block_signal(int signal){
  // block either SIGUSR1 or SIGUSR2

  sigset_t sigset; 
  sigemptyset(&sigset); 
  sigaddset(&sigset, signal);

  pthread_sigmask(SIG_BLOCK, &sigset, NULL);
}






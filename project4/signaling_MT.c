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
  sigusr1_sent_counter = 0;
  sigusr2_sent_counter = 0; 
  sigusr1_recieved_counter = 0;
  sigusr2_recieved_counter = 0; 


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

  int j = 0; 

  while(j<2){
    ++j; 
    
    sleep_random_interval(.01, .1); // sleep [.01-.1] 

    int signal = random_signal(); 
   // printf("SENT SIGNAL: %d\n", signal);

    signal = 1; 

    // send the signal to ALL other threads (to mirror processes approach)
    if(signal == 1){
      
      for(int i =0; i<NUM_THREADS; ++i)
        pthread_kill(threads[i], SIGUSR1);  


      // safely increment counter for sigusr1 sent 
      pthread_mutex_lock(&lock_sigusr1_sent); 
      ++sigusr1_sent_counter;
      pthread_mutex_unlock(&lock_sigusr1_sent);

      //printf("SIG1 S: %d\n", sigusr1_sent_counter);
    }

    else{
      // send SIGUSR2

      for(int i =0; i<NUM_THREADS; ++i)
        pthread_kill(threads[i],SIGUSR2);

      //safely increment counter for sigusr1 sent 
      pthread_mutex_lock(&lock_sigusr2_sent); 
      ++sigusr2_sent_counter;
      pthread_mutex_unlock(&lock_sigusr2_sent);

      //printf("SIG2 S: %d\n", sigusr2_sent_counter);
    }

  }




}

void* signal_handling_sigusr1(void *arg){
  // 2 threads here 
  // sigwait() until get a SIGUSR1 signal

  block_both(); // block both usrs signals 

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1); // wait SIGUSR1
  int retval = 0;
  int signal;

  while (1){

	  retval = sigwait(&sigset, &signal);

	  if(signal == SIGUSR1){
	    // received signal, increment count 

      pthread_mutex_lock(&lock_sigusr1_received); 
      ++sigusr1_recieved_counter; 
      pthread_mutex_unlock(&lock_sigusr1_received);

      //printf("SIG1 R: %d\n", sigusr1_recieved_counter);
	  }

  }



}

void* signal_handling_sigusr2(void *arg){
  // 2 threads here 
  // sigwait() until get a SIGUSR2 signal

  block_both(); // block both usr signals 

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR2); // wait SIGUSR2
  int retval = 0;
  int signal;

  while (1){

	  retval  = sigwait(&sigset, &signal);

	  if(signal == SIGUSR2){
      // received signal, increment count 

      pthread_mutex_lock(&lock_sigusr2_received); 
      ++sigusr2_recieved_counter; 
      pthread_mutex_unlock(&lock_sigusr2_received);

      //printf("SIG2 R: %d\n", sigusr2_recieved_counter);
	  }

  }


  while(1){
    sleep(1);
  }

}

void* reporting_thread(void *arg){

  block_both();

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1);
  sigaddset(&sigset, SIGUSR2); // wait SIGUSR1 && SIGUSR2
  int retval = 0;
  int signal;
  int report_count = 0; 

  struct timespec prev_sigusr1;
  struct timespec prev_sigusr2;

  int report_sig1= 0;
  int report_sig2 = 0; 

  
  double sigusr1_differences[10]; 
  double sigusr2_differences[10]; 
  int sigusr1_index_differences = 0; 
  int sigusr2_index_differences = 0; 


  while(1){

	  retval  = sigwait(&sigset, &signal);
    ++report_count; 

    struct timespec current;

    // if(clock_gettime( CLOCK_MONOTONIC, &current) == -1 ){
    //   fprintf(stderr, "%s", "clock_gettime error");
    //   exit(1);
    // }

  //  printf("CURRENT: %ld\n", current.tv_nsec);

	  if(signal == SIGUSR1){
      // received signal, increment count 
      //puts("SIGUSR1 test");

    if(clock_gettime( CLOCK_MONOTONIC, &current) == -1 ){
      fprintf(stderr, "%s", "clock_gettime error");
      exit(1);
    }

      ++report_sig1;

      if(report_sig1 == 1){
        // need min 2 signals 
        prev_sigusr1.tv_nsec = current.tv_nsec;
        prev_sigusr1.tv_sec = current.tv_sec;
        continue;
      }

     // printf("START: %ld\n", prev_sigusr1.tv_nsec);
     // printf("STOP %ld\n", current.tv_nsec); 

      double difference = (current.tv_sec - prev_sigusr1.tv_sec) * 1e9; 
      difference = (difference + (current.tv_nsec - prev_sigusr1.tv_nsec)) * 1e-9; 

      sigusr1_differences[sigusr1_index_differences] = difference; 

     // printf("sig 1 diff: %lf\n",  sigusr1_differences[sigusr1_index_differences]);

      prev_sigusr1.tv_nsec = current.tv_nsec; 
      prev_sigusr1.tv_sec = current.tv_sec;

      ++sigusr1_index_differences;
      
	  }

    else if (signal == SIGUSR2){
      //puts("SIGUSR2 test");

      ++report_sig2;

      if(report_sig2 == 1){
        // need min 2 signals 
        prev_sigusr2.tv_nsec = current.tv_nsec;
        prev_sigusr2.tv_sec = current.tv_sec;
        continue;
      }

      double difference = (current.tv_sec - prev_sigusr2.tv_sec) * 1e9; 
      difference = (difference + (current.tv_nsec - prev_sigusr2.tv_nsec)) * 1e-9; 

      sigusr2_differences[sigusr2_index_differences] = difference; 

      printf("sig 2 diff: %lf\n",  sigusr2_differences[sigusr2_index_differences]);

      prev_sigusr2.tv_nsec = current.tv_nsec; 
      prev_sigusr2.tv_sec = current.tv_sec;

      ++sigusr2_index_differences;
    }


    if(report_count % 10 == 0){
      // INCOMING REPORT

    // struct timespec current;

    // if(clock_gettime( CLOCK_MONOTONIC, &current) == -1 ){
    //   fprintf(stderr, "%s", "clock_gettime error");
    //   exit(1);
    // }

    }

    //printf("TOTAL COUNT%d\n", report_count);

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

  struct timespec start, stop, sleeep, rem;

  sleeep.tv_sec = 0; // [.01-.1] seconds, always under 1  
  sleeep.tv_nsec = microseconds * 1000; // now nanoseconds 

 // printf("I will sleep for %lf seconds\n", (sleeep.tv_nsec/1e9)); 

  //clock_gettime(CLOCK_MONOTONIC, &start);

  if(nanosleep(&sleeep, &rem) != 0){
    puts("nanosleep error");
    exit(1);
  }

  // clock_gettime(CLOCK_MONOTONIC, &stop);

  // double difference = (stop.tv_sec - start.tv_sec) * 1e9; 
  // difference = (difference + (stop.tv_nsec - start.tv_nsec)) * 1e-9; 

  // printf("DIFF: %lf\n", difference);


}

int random_signal(){
  // generate random number between 0 and 1, if 0 SIGUSR1, else SIGUSR2 

  int seed = pthread_self() ^ time(NULL); 
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






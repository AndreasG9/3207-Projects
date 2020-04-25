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
#include <sys/types.h>
#include <string.h>

void signal_generating(); 
void signal_handling_sigusr1();
void signal_handling_sigusr2();
void signal_handling_handler(int signal); 
void signal_handling_handler2(int signal);
void reporting(); 
void reporting_handler(int signal); 
void block_sigusr1();
void block_sigusr2();
void block_both();
void sleep_random_interval(double low, double high); 
int random_signal(); 
double calc_average(double diff[10]); 
//double time_diff(struct timespec start, struct timespec stop); 


//void clean_up(int signal); 


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



// FOR REPORTING PROCESS ONLY (not valid in any other process)
// ********* put into struct later *********************
int reporting_counter; 
int report_sigusr1;
int report_sigusr2; 

double sigusr1_differences[10]; // up to 10 
double sigusr2_differences[10];

struct timespec prev_sigusr1;
struct timespec prev_sigusr2; 

int sigusr1_index_differences; 
int sigusr2_index_differences; 



int main (int argc, char *argv[]){

  reporting_counter = 0; 
  report_sigusr1 = 0;
  report_sigusr2 = 0; 

  // INIT shared_val struct
  // ALL children have access to this ptr to struct shared_val
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
  shared_ptr->sigusr1_recieved_counter = 0; // FIX SPELLING RECEIEVED 
  shared_ptr->sigusr2_recieved_counter = 0; 

  // init locks with set attribute 
  pthread_mutex_init(&(shared_ptr->lock_sigusr1_sent), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr2_sent), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr1_recieved), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr2_recieved), &attr); 

 // printf("INIT %d %d %d %d\n", shared_ptr->sigusr1_sent_counter, shared_ptr->sigusr2_sent_counter, shared_ptr->sigusr1_recieved_counter, shared_ptr->sigusr2_recieved_counter);   

  pid_t parent; 

  // spawn child processes ... 
  pid_t pids[8]; 
  int status; 

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

      if(i == 0 || i == 1){
        signal_handling_sigusr1(); 
      }
      else if(i == 2 || i == 3){
        signal_handling_sigusr2();
      } 
      else if(i == 4 || i == 5 || i == 6){

        // GENERARTING PROCESS should block both SIGNALS, dont want the process to terminate unexpectedly ?? double check ... 
        // signal(SIGUSR1, temp);
        // signal(SIGUSR2, temp);
        block_both();
        signal_generating(); 
        //  printf("counter SENT sigusr1: %d\n", shared_ptr->sigusr1_sent_counter); 
        //  printf("counter SENT sigusr2: %d\n", shared_ptr->sigusr2_sent_counter); 

      }
      else if(i == 7){
        reporting(); 

        while(1){
          sleep(1);
        }
      }
    }

    //else if (pids[i] > 0){
    else{
      // PARENT 
      if((i) != 7)
        continue; 

      // int status;
      // waitpid(pids[i], &status, 0); 
     // wait(NULL); 
      puts("parent, other processes forked"); 
      parent = getpid();

      // PARENT, block both signals 
      // signal(SIGUSR1, block_later);
      // signal(SIGUSR2, block_later);
      block_both();

    }

  }


  //signal(SIGINT, clean_up); // testing, ctrl-c 

  // VARIABLE, for testing parent wait x amount seconds then exit program 
 // while(1){
   //sleep(1); 

 
  // shmdt(shared_ptr); // cant use any more! 

  while(1){
    sleep(1);


    // if((shared_ptr->sigusr1_sent_counter) + (shared_ptr->sigusr2_sent_counter) >= 10)
    //   break; 
    // if((shared_ptr->sigusr1_recieved_counter) + (shared_ptr->sigusr2_recieved_counter) == 20)
    //   break; 

    // if((shared_ptr->sigusr1_recieved_counter) + (shared_ptr->sigusr2_recieved_counter) == 11)
    //   break;

  
  }

  // puts("PARENT");
  // printf("SENT sig1  %d\n", shared_ptr->sigusr1_sent_counter);
  // printf("SENT sig2 %d\n", shared_ptr->sigusr2_sent_counter);
  
  // printf("RECIEVE sig1 %d\n", shared_ptr->sigusr1_recieved_counter);
  // printf("RECIEVE sig2 %d\n", shared_ptr->sigusr2_recieved_counter);

  // printf("TOTAL RECIEVED %d\n", shared_ptr->sigusr1_recieved_counter + shared_ptr->sigusr2_recieved_counter);
  // puts("\n");
  // fflush(stdout);

    // if((shared_ptr->sigusr1_recieved_counter + shared_ptr->sigusr2_recieved_counter) != 30)
    //   puts("SHIEEEEEEEEEEEEEEEEEEEET");
    // else{
    //   puts("GOOD");
    // }

    // printf("sig1 counter: %d\n", report_sigusr1);
    // printf("sig2 counter: %d\n", report_sigusr2);
  

  shmdt(shared_ptr); // cant use any more! 

  while(1){
    sleep(1);
  }



  // shmdt(shared_ptr); // cant use any more! 
  //puts("see data.log for results ...");

  // while(1){
  //   sleep(1);
  // }
 

  // SIGTERM HANDLER TODO, program done execution 
  // for(int i =0; i<8; ++i){
  //   kill(pids[i], ); 
  // }

}

void signal_generating(){
  // indefinitely loop, select SIGUSR1 or SIGUSR2 to send to processes (all)
  // increment appropriate counter 

  printf("generating pid %d\n", getpid()); 


  int j = 0; 
 // while(1){

   while(j<7){

    sleep_random_interval(.01, .1); // sleep [.01-.1] 
    //printf("%d is back\n", getpid()); 

    int signal = random_signal();       

    //printf("%d will SEND SIGUSR: %d\n", getpid(), signal); 

    // send the signal to "its peers"/ all child processes (signal is either SIGUSR1 or SIGUSR2)
    if(signal == 1){
     if( kill(0, SIGUSR1) == -1){
        puts("failed to send signal SIGUSR1");
        exit(0); 
     }
     else{

        //increment counter for sigusr1 sent 
        pthread_mutex_lock(&(shared_ptr->lock_sigusr1_sent)); 
        shared_ptr->sigusr1_sent_counter++; 
        pthread_mutex_unlock(&(shared_ptr->lock_sigusr1_sent)); 

       // printf("counter SENT sigusr1: %d\n", shared_ptr->sigusr1_sent_counter); 
     }
   }
     
    else{
      if( kill(0, SIGUSR2) == -1){
        puts("failed to send signal SIGUSR2");
        exit(0); 
      }
      else{

       // increment counter for sigusr2 sent 
        pthread_mutex_lock(&(shared_ptr->lock_sigusr2_sent)); 
        shared_ptr->sigusr2_sent_counter++; 
        pthread_mutex_unlock(&(shared_ptr->lock_sigusr2_sent)); 

       // printf("counter SENT sigusr2: %d\n", shared_ptr->sigusr2_sent_counter); 
      }
    }

    ++j; 
     
  }



  while(1){
    sleep(1);
  }

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

  // if(random_int == 1)
  //   return SIGUSR1;
  // else
  //   return SIGUSR2; 

  if(random_int == 1)
    return 1; 
  else
    return 2; 
  
}

void signal_handling_sigusr1(){
  // indefinitely loop (will be 2 child processes in this function) ... 
  // 2 for SIGUSR1 (PID INDEXES 0 AND 1),
  // SIGNAL sent, goes to all 4, block other 

  printf("sigusr1 handling pid %d\n", getpid()); 
  fflush(stdout);

  signal(SIGUSR1, signal_handling_handler); // set signal for THIS process 
  block_sigusr2(); // block other signal type as signal generating sends signal to all child processes 

  while(1){
    sleep(1);
  }
 
  // fflush(stdout);
  // return;    


}

void signal_handling_sigusr2(){
  // indefinitely loop (will be OTHER 2 child processes in this function) ... 
  //  2 for SIGUSR2 (PID INDEXES 2 AND 3)

  printf("sigusr2 handling pid %d\n", getpid()); 

  signal(SIGUSR2, signal_handling_handler2); // set signal
  block_sigusr1(); // block other signal type  

  signal(SIGUSR1, signal_handling_handler2);

  // for (;;) pause();

  while(1){
    sleep(1); 
   // pause();
  }

}

void signal_handling_handler(int signal){
  // RECIEVED COUNT (of each signal) always DOUBLE the sent count (of each signal)
  // ex. sigusr1_sent_counter = 50 .... sigusr1_recieved_counter = 100 

  if(signal == SIGUSR1){
    // signal arrived, increment counter in shared memory segment 

    //printf("%d handler SIGUSR1\n", getpid()); 

    pthread_mutex_lock(&(shared_ptr->lock_sigusr1_recieved));
    shared_ptr->sigusr1_recieved_counter++;
    pthread_mutex_unlock(&(shared_ptr->lock_sigusr1_recieved));
    
    //printf("counter RECIEVED sigusr1: %d\n", shared_ptr->sigusr1_recieved_counter); 
    //fflush(stdout);
  }

}

void signal_handling_handler2(int signal){

  if(signal == SIGUSR2){
    // signal arriv ed, increment counter 

   // printf("%d handler SIGUSR2\n", getpid());
    
    pthread_mutex_lock(&(shared_ptr->lock_sigusr2_recieved));
    shared_ptr->sigusr2_recieved_counter++;
    pthread_mutex_unlock(&(shared_ptr->lock_sigusr2_recieved));

  // printf("counter RECIEVED sigusr2: %d\n", shared_ptr->sigusr2_recieved_counter); 
  // fflush(stdout);
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

void block_both(){
  // PARENT, GENERATING processes, will want to block both signal (prevent early termination)

  sigset_t sigset; 
  sigemptyset(&sigset); 
  sigaddset(&sigset, SIGUSR1); // add SIGUSR2 to set 
  sigaddset(&sigset, SIGUSR2); // add SIGUSR2 to set 
  sigprocmask(SIG_BLOCK, &sigset, NULL); // mask/ filter out SIGUSR1 

}

void reporting(){
  // single child process here
  // recieves both SIGUSR1 and SIGUSR2 (dont block)
  // keep counts, every 10 signals REPORT ... 

  // register, able to recieve both usr SIGNAL types 
  signal(SIGUSR1, reporting_handler);
  signal(SIGUSR2, reporting_handler);

  printf("reporting pid %d\n", getpid()); 

  struct timespec prev_sigusr1;
  struct timespec prev_sigusr2;

  prev_sigusr1.tv_sec = 0; 
  prev_sigusr1.tv_nsec = 0; 
  prev_sigusr2.tv_sec = 0; 
  prev_sigusr2.tv_nsec = 0; 

  sigusr1_index_differences = 0; 
  sigusr2_index_differences = 0; 

  int i = 0; 

  while(1){ 

    pause(); // will "unpause" when handler is called, ... 
   // printf("%d\n", reporting_counter);
    ++i; 

    //if(i == 10){
      //INCOMING REPORT 
    if(reporting_counter % 10 == 0){
      
      i = 0; 

    // PUT INTO SEPERATE LOG FILE 
      
      struct timespec current;

      if(clock_gettime( CLOCK_MONOTONIC, &current) == -1 ){
      fprintf(stderr, "%s", "clock_gettime error");
      exit(1);
      }

      puts("");
      printf("SYSTEM TIME AT REPORT: %ld\n", current.tv_nsec); 

      printf("SIGUSR1 S: %d\n", shared_ptr->sigusr1_sent_counter);
      printf("SIGUSR2 S: %d\n", shared_ptr->sigusr2_sent_counter);
  
      printf("SIGUSR1 R: %d\n", shared_ptr->sigusr1_recieved_counter);
      printf("SIGUSR2 R: %d\n", shared_ptr->sigusr2_recieved_counter);

      printf("SIGUSR1 REPORT SENT counter: %d\n", report_sigusr1);
      printf("SIGUSR2 REPORT SENT counter: %d\n", report_sigusr2);
      printf("SIGUSR2 REPORT TOTAL counter: %d\n", reporting_counter);

  
      double avg; 

      avg = calc_average(sigusr1_differences); 
      printf("SIGUSR1 avg time between receptions: %lf SECONDS\n", avg);

      avg = calc_average(sigusr2_differences); 
      printf("SIGUSR2 avg time between receptions: %lf SECONDS\n", avg);

      // RESET, 10 signals reached 
      // i = 0; 
    // prev_sigusr1.tv_nsec = 0; 
    // prev_sigusr1.tv_sec = 0; 
    // prev_sigusr2.tv_nsec = 0; 
    // prev_sigusr1.tv_sec = 0; 
      sigusr1_index_differences = 0; 
      sigusr2_index_differences = 0; 
      memset(sigusr1_differences, 0, sizeof(sigusr1_differences)); 
      memset(sigusr2_differences, 0, sizeof(sigusr2_differences)); 
      
    } 

   // pause();

  }


  while(1){
    sleep(1);
  }

}

void reporting_handler(int signal){
  // recieves both SIGUSR1 && SIGUSR2 
  // increment personal counters (SIGUSR1, SIGUSR2, and BOTH)
  // Track current system time,  to get difference between a pair of calls (same signal),
  // store in an array of differences (one for SIGUSR1, one for SIGUSR2) ... 
  // After 10 signals, calc. the average of those differences, report 

  // struct timespec current;

  // if(clock_gettime( CLOCK_MONOTONIC, &current) == -1 ){
  //   fprintf(stderr, "%s", "clock_gettime error");
  //   exit(1);
  // }

  //printf("CURRENT SYSTEM TIME NSECONDS : %ld\n", current.tv_nsec);

   ++reporting_counter; // global counter, only 1 reporting process



  if(signal == SIGUSR1){

    struct timespec current;

    if(clock_gettime( CLOCK_MONOTONIC, &current) == -1 ){
      fprintf(stderr, "%s", "clock_gettime error");
      exit(1);
    }

    //puts("SIGUSR1"); 
    ++report_sigusr1;   

    if(report_sigusr1 == 1){
      // need min 2 signals 
      prev_sigusr1.tv_nsec = current.tv_nsec;
      prev_sigusr1.tv_sec = current.tv_sec;
      return;
    }

    // double difference = (((prev_sigusr1.tv_sec - current.tv_sec) * 1e9) + ((current.tv_nsec - prev_sigusr1.tv_nsec))) * 1e-9; // reported in seconds (better visually)
    double difference = (current.tv_sec - prev_sigusr1.tv_sec) * 1e9; 
    difference = (difference + (current.tv_nsec - prev_sigusr1.tv_nsec)) * 1e-9; 

     sigusr1_differences[sigusr1_index_differences] = difference; 

    //sigusr1_differences[sigusr1_index_differences] = time_diff(prev_sigusr1, current); // with "last-occurance" time, calc difference, store that value 
      
    //new prev time = current time 
    prev_sigusr1.tv_nsec = current.tv_nsec; 
    prev_sigusr1.tv_sec = current.tv_sec;

    ++sigusr1_index_differences;
  }
    
  else if(signal == SIGUSR2){
    // identical to previous block 

    struct timespec current;

    if(clock_gettime( CLOCK_MONOTONIC, &current) == -1 ){
      fprintf(stderr, "%s", "clock_gettime error");
      exit(1);
   }

    ++report_sigusr2;    

    if(report_sigusr2 == 1){
      // need min 2 signals 
      prev_sigusr2.tv_nsec = current.tv_nsec;
      prev_sigusr2.tv_sec = current.tv_sec;
      return;
    }

    double difference = (current.tv_sec - prev_sigusr2.tv_sec) * 1e9; 
    difference = (difference + (current.tv_nsec - prev_sigusr2.tv_nsec)) * 1e-9; 

    // double difference = (((prev_sigusr2.tv_sec - current.tv_sec) * 1e9) + ((current.tv_nsec - prev_sigusr2.tv_nsec))) * 1e-9;

    sigusr2_differences[sigusr2_index_differences] = difference; 
    //sigusr2_differences[sigusr2_index_differences] = time_diff(prev_sigusr2, current); 

    // new prev time = current time 
    prev_sigusr2.tv_nsec = current.tv_nsec; 
    prev_sigusr2.tv_sec = current.tv_sec;

    ++sigusr2_index_differences;
  }
  
}

double calc_average(double diff[10]){
    // calc. avg based on diffs in sigusr1_differences[10] and sigusr2_diferences[10]

  int j = 0;
  double sum = 0; 
  double avg; 

  while(diff[j]){
    sum += diff[j];
    ++j; 
  }

  if(sum == 0){
    // possible recent 10 signals, this recieved none (does happen, occasionally)
    avg = 0; 
  }
  else{
    avg = (sum / j); 
  }

  return avg; 

}





// void clean_up(int signal){

//   puts("detached from shared memory region"); 

//   if(signal == SIGINT){

//     shmdt(shared_ptr); 
//   }

// } 


// void signal_handling_handler2(int signal){

//   //   if(signal == SIGUSR2){
//   //   // signal arrived, increment counter 

//   //   pthread_mutex_lock(&lock_sigusr2_recieved);
//   //   ++sigusr2_recieved_counter;
//   //   pthread_mutex_unlock(&lock_sigusr2_recieved); 
//   // }

// }


// // testing 
// double time_diff(struct timespec start, struct timespec stop){
//   // start = previous time 
//   // stop = current time (from report_handler)

//   // struct timespec stop;

//   // if(clock_gettime( CLOCK_MONOTONIC, &stop) == -1 ){ // get CURRENT system time 
//   //   fprintf(stderr, "%s", "clock_gettime error");
//   //   exit(1);
//   // }

//   // printf("start: %lf\n", start.tv_nsec/1e9);
//   // printf("stop: %lf\n", stop.tv_nsec/1e9);

//   // difference between two times
//   double difference = (((stop.tv_sec - start.tv_sec) * 1e9) + ((stop.tv_nsec - start.tv_nsec))) * 1e-9; // reported in seconds (better visually)

//   //printf("DIFFERENCE: %lf\n", difference);

//   return difference; 
// }
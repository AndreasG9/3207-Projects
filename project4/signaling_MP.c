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
//#define _POSIX_C_SOURCE 1

void signal_generating(); 
void signal_handling_sigusr1();
void signal_handling_sigusr2();
void signal_handling_handler(int signal); 
void signal_handling_handler2(int signal);
void block_sigusr1();
void block_sigusr2();

// void block both .... 

void reporting(); 
void sleep_random_interval(double low, double high); 
int random_signal(); 
int time_details(); 

void reporting_handler(int signal); 
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


void block_later(int signal){

  // parent process, block SIGUSR1 SIGUSR2, for now do nothing 
 // puts("parent ignores both");
}


void temp(int signal){

  //printf("hello: %d\n", getpid());
}


// put in the struct? 
int reporting_counter; 
int report_sigusr1;
int report_sigusr2; 

int main (int argc, char *argv[]){

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
  shared_ptr->sigusr1_recieved_counter = 0;
  shared_ptr->sigusr2_recieved_counter = 0; 

  // init locks with set attribute 
  pthread_mutex_init(&(shared_ptr->lock_sigusr1_sent), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr2_sent), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr1_recieved), &attr); 
  pthread_mutex_init(&(shared_ptr->lock_sigusr2_recieved), &attr); 

 // printf("INIT %d %d %d %d\n", shared_ptr->sigusr1_sent_counter, shared_ptr->sigusr2_sent_counter, shared_ptr->sigusr1_recieved_counter, shared_ptr->sigusr2_recieved_counter);   


  // signal(SIGUSR1, signal_handling_handler); // set signal 
  // signal(SIGUSR2, signal_handling_handler); // set signal 
  pid_t parent; 

  reporting_counter = 0; 
  report_sigusr1 = 0;
  report_sigusr2 = 0; 

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
        signal(SIGUSR1, temp);
        signal(SIGUSR2, temp);

        signal_generating(); 
        //  printf("counter SENT sigusr1: %d\n", shared_ptr->sigusr1_sent_counter); 
        //  printf("counter SENT sigusr2: %d\n", shared_ptr->sigusr2_sent_counter); 

      }
      else if(i == 7){
        reporting(); 
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
      signal(SIGUSR1, block_later);
      signal(SIGUSR2, block_later);

    }

  }


  //signal(SIGINT, clean_up); // testing, ctrl-c 

  // VARIABLE, for testing parent wait x amount seconds then exit program 
 // while(1){
   //sleep(1); 

  shmdt(shared_ptr); // cant use any more! 

  while(1){
    sleep(1);
  }

  // sleep(1);

  //  int sentcount, rcount = 0; 

  //  if(parent == getpid()){
  //   sentcount = (shared_ptr->sigusr1_sent_counter) + (shared_ptr->sigusr1_sent_counter);
  //   rcount =  (shared_ptr->sigusr1_recieved_counter) + (shared_ptr->sigusr1_recieved_counter);

  //   printf("\n\nHELLO: %d\n%d\n", sentcount, rcount);

  //   shmdt(shared_ptr);
  //  }


  // shmdt(shared_ptr); // cant use any more! 

  // while(shared_ptr->lock_sigusr1_recieved !=0 5){
      // parent will wait, until this counter reached  

  //}
  // exit(0); 

  // SIGTERM HANDLER TODO, program done execution 
  // for(int i =0; i<8; ++i){
  //   kill(pids[i], ); 
  // }

}

void signal_generating(){
  // indefinitely loop, select SIGUSR1 or SIGUSR2 to send to processes
  // increment appropiate counter 

  printf("generating pid %d\n", getpid()); 

  int j = 0; 

 // while(1){
   while(j<2){

    sleep_random_interval(.01, .1); // sleep [.01-.1] 
    //printf("%d is back\n", getpid()); 

    int signal = random_signal();      
    printf("%d will SEND SIGUSR: %d\n", getpid(), signal); 


    // send the signal to "its peers"/ all child processes (signal is either SIGUSR1 or SIGUSR2)
    if(signal == 1){
     if( kill(0, SIGUSR1) == -1){
        puts("failed to send signal SIGUSR1");
        exit(0); 
     }
     else{

        //increment global counter for sigusr1 sent 
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

       // increment global counter for sigusr2 sent 
        pthread_mutex_lock(&(shared_ptr->lock_sigusr2_sent)); 
        shared_ptr->sigusr2_sent_counter++; 
        pthread_mutex_unlock(&(shared_ptr->lock_sigusr2_sent)); 

      //  printf("counter SENT sigusr2: %d\n", shared_ptr->sigusr2_sent_counter); 
      }
    }
     

     ++j; 

  }

  while(1){
    sleep(1);
  }

  // printf("counter sigusr1: %d\n", shared_ptr->sigusr1_sent_counter); 
  // printf("counter sigusr2: %d\n", shared_ptr->sigusr2_sent_counter); 
}

void sleep_random_interval(double low, double high){
  // generate random between low(.01) and high(.1) 
  // sleep for that time 
  
  int num = time(NULL) ^ getpid(); // unique seed 
  srand(num); 

  double random_double = (double)rand() * (low - high) / (double)RAND_MAX + high; // expected to be between [.01 - .1]  
 // printf("%lf\n", random_double); 

  //double microseconds = (random_double * 1000000); // get microseconds
  useconds_t microseconds = (random_double * 1000000); // get microseconds

  //printf("I (%d) will sleep for %d microseconds\n", getpid(), microseconds); 

  usleep(microseconds); 
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

  signal(SIGUSR1, signal_handling_handler); // set signal for THIS process 
  block_sigusr2(); // block other signal type as signal generating sends signal to all child processes 
  
  // for (;;) pause();

  while(1){ 
    //pause();
    sleep(1);
    //printf("here %d\n", getpid());
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
  // RECIEVED COUNT (of each signal) always DOUBLE the sent count (of each signal)
  // ex. sigusr1_sent_counter = 50 .... sigusr1_recieved_counter = 100 

  if(signal == SIGUSR1){
    // signal arrived, increment counter in shared memory segment 

    //printf("%d handler SIGUSR1\n", getpid()); 

    pthread_mutex_lock(&(shared_ptr->lock_sigusr1_recieved));
    shared_ptr->sigusr1_recieved_counter++;
    pthread_mutex_unlock(&(shared_ptr->lock_sigusr1_recieved));
    
     printf("counter RECIEVED sigusr1: %d\n", shared_ptr->sigusr1_recieved_counter); 
  }

}

void signal_handling_handler2(int signal){

  if(signal == SIGUSR2){
    // signal arriv ed, increment counter 

   // printf("%d handler SIGUSR2\n", getpid());
    
    pthread_mutex_lock(&(shared_ptr->lock_sigusr2_recieved));
    shared_ptr->sigusr2_recieved_counter++;
    pthread_mutex_unlock(&(shared_ptr->lock_sigusr2_recieved));

    printf("counter RECIEVED sigusr2: %d\n", shared_ptr->sigusr2_recieved_counter); 
  }

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

void reporting(){
  // single child process here
  // recieves both SIGUSR1 and SIGUSR2 (dont block)
  // keep count 

  // register, able to recieve both usr SIGNAL types 
  signal(SIGUSR1, reporting_handler);
  signal(SIGUSR2, reporting_handler);

  // every 10 signals, REPORT: system time, shared counters (SIGUSR1/SIGUSR2: sent, recieved), average time interval between signal types 

  printf("reporting pid %d\n", getpid()); 

  //int private_counter = 0; // ?? how tell handler called 


  // if(reporting_counter % 10 == 0){
  //   // every 10 signals 

  // }
  
  //for (;;) pause();


  while(1){
    //sleep(1); 
    pause();
  }

 // while(1){}

  // FILE *fptr = fopen("data.log", "w"); 
  // fclose(fptr); 

  // while(1){
  // }

}

void reporting_handler(int signal){
  // recieves both SIGUSR1 && SIGUSR2 

  // ++reporting_counter; 

  // if(signal == SIGUSR1){
    
  //  // ++report_sigusr1; 
  //   puts("SIGUSR1");
  // }
    
  // else if(signal == SIGUSR2){

  //   //++report_sigusr2; 
  //   puts("SIGUSR2");
  // }
    
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
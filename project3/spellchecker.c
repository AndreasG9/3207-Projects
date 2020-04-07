/*
Project 3
Andreas Gagas
spellchecker.c - Network Spell Checker 
*/

#include <stdlib.h>
#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <ctype.h> 
#include <signal.h>

#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8888
#define SIZE 50 // MAX QUEUE SIZE 
#define NUM_WORKERS 50 // support up to 50 w_threads 

// network 
int socket_desc;
int port_number; 

// dictionary 
char *dictionary; // default is "dictionary.txt" 
char **dictionary_stored_here; // every string in dictionary, stored here, accessible by all threads 
int dictionary_count; // too allocate enough memory for double ptr (each string ref by ptr)

// producer-consumer, connection queue 
int buffer_connection_Q[SIZE]; 
int connection_q_count; 
int add_index, remove_index; 
int thread_current; 
pthread_mutex_t connection_q_lock; 
pthread_cond_t safe_to_add; // condition, wait until space available to add to queue 
pthread_cond_t safe_to_remove; // condition, wait until you can remove data from queue 

// producer-consumer, log queue 
char *buffer_log_Q[SIZE]; 
int log_q_count; 
int add_index2, remove_index2; 
pthread_mutex_t log_q_lock;
pthread_cond_t safe_to_add2;  
pthread_cond_t safe_to_remove2; 

// main thread 
void fill_dictionary_structure(); 
void init_some_vars(); 
int check_dictionary(char *word); 
void add_to_connection_queue(int socket); 
void create_threads(); 
void clean_up(void); 

// worker threads 
void* worker_thread(void* id); 
int remove_from_connection_queue(); 
void add_to_log_queue(char *result); 
pthread_attr_t detach;
int pthread_detach(pthread_t thread); 

// log thread 
FILE *fptr; 
void* log_thread(void *arg);
char* remove_from_log_queue(); 

// properly free heap allocated structures when program killed  
void sigint_handler(int sig_num); 


int main (int argc, char *argv[]){
  // "main thread"  
  
  if(argc == 1){
    // no dictionary or port specified, use default values

    dictionary = DEFAULT_DICTIONARY;
    port_number = DEFAULT_PORT; 
  }

  else if (argc == 2){
    // determine if the arg passed was a word (dictionary) or number (port)

    if((access(argv[1], F_OK) != -1)){
      // your dictionary file exists, passed first arg 
      dictionary = argv[1]; 
      port_number = DEFAULT_PORT; 
    }
    else if(isdigit(*argv[1])){
      // port number was passed in first arg 
      port_number = atoi(argv[1]);
      dictionary = DEFAULT_DICTIONARY; 
    }
    else{
      // file not found, use defaults 
      dictionary = DEFAULT_DICTIONARY;
      port_number = DEFAULT_PORT; 
    }
  }

  else if(argc == 3){
    // same implementation as previous, to determine which arg is a word ... bit more if else-ing 
    if((access(argv[1], F_OK) != -1) && (isdigit(*argv[2]))){
      // your dictionary file exists, passed first arg and second arg is a num
      dictionary = argv[1]; 
      port_number = atoi(argv[2]); 
    }
    else if((access(argv[2], F_OK) != -1) && (isdigit(*argv[1]))){
      // your dictionary file exists, passed second arg and first arg is a num 
      dictionary = argv[2]; 
      port_number = atoi(argv[1]); 
    }
    if((access(argv[1], F_OK) != -1) && (isdigit(*argv[2]))){
      // your dictionary file exists, passed first arg and second arg is a num
      dictionary = argv[1]; 
      port_number = atoi(argv[2]); 
    }
    else if((access(argv[1], F_OK) != -1) && (!(isdigit(*argv[1])))){
      // your dictionary file exists, passed second arg which is NOT a number
      // instance where both args are files 
      dictionary = argv[1]; 
      port_number = DEFAULT_PORT; 
    }
    else if((access(argv[1], F_OK) == -1) && (isdigit(*argv[2]))){
      // file doesn't exist, but port num could work 
      port_number = atoi(argv[2]);
      dictionary = DEFAULT_DICTIONARY; 
    }
  
    else{
       // tested all instances (I think), this is just a back-up 
      dictionary = DEFAULT_DICTIONARY;
      port_number = DEFAULT_PORT; 
    }
  }
  
  else{
    // to many args
    fprintf(stderr, "%s", "TOO MANY ARGS\n"); 
    exit(1); 
  }

   printf("file: %s\n", dictionary);
   printf("port num: %d\n", port_number); 

  // INIT 
  connection_q_count, add_index, remove_index = 0; 
  log_q_count, add_index2, remove_index2 = 0; 
  thread_current = 0;  

  fill_dictionary_structure(); // char **dictionary_stored_here, hold a string for each line in dictionary file  
  init_some_vars();   // init condition vars and mutexes/locks 
  signal(SIGINT, sigint_handler); 

  // spawn worker threads (and log thread) who remove data from queue (consumer), will write to socket and log the word + OK/MISSPELLED
  // bulk of work done within these thread/s 
  create_threads(); 
  

  // NETWORK setup (given code)
  int new_socket, c;

  struct sockaddr_in server, client;
  char *message;
  // Create socket (create active socket descriptor)
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == -1){ 
	  puts("Error creating socket!");
	  exit(1);
  }

  // prepare sockaddr_instructure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY; // defaults to 127.0.0.1
  server.sin_port = htons(port_number);
  // Bind (convert the server's socket address to the socket descriptor)
  int bind_result = bind(socket_desc, (struct sockaddr*)&server, sizeof(server));
  if (bind_result < 0){
	  fprintf(stderr, "%s","Error: failed to Bind.");
	  exit(1);
  }

  puts("Bind done.");

  // Listen (converts active socket to a LISTENING socket which can accept connections)
  listen(socket_desc, 3);
  puts("Waiting for incoming connections...");
 
  while (1){
    // MAIN LOOP 

	  c = sizeof(struct sockaddr_in);
	  new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);

	  if (new_socket < 0){
	    fprintf(stderr, "%s","Error: Accept failed");
	    continue;
	  }
    //fprintf(stdout, "%s","Connection accepted");
    fprintf(stdout, "%s%d%s%d\n", "Thread ", thread_current+1, " grabbed socket: ", new_socket); 

    add_to_connection_queue(new_socket); // add new socket to the q, which signals the worker threads, ... chain reaction 
    ++thread_current; // coordinate to w_id arg passed to w_thread 

  }
   
  return 0; 
}

void fill_dictionary_structure(){
  // open dictionary file, store strings in double ptr / array of ptrs to strings 

  FILE *fptr = fopen(dictionary, "r");

  if(fptr == NULL){
    printf("%s not found", dictionary); 
    exit(0); 
  }

  dictionary_count = 0; 
  char buffer[100]; // temp 

  while(fgets(buffer, 100, fptr) != NULL)
    ++dictionary_count;


  // allocate memory for n amount of string entries, each ref by ptr, reset file ptr 
  //dictionary_stored_here = (char **)malloc(sizeof(char*) * dictionary_count); 
  dictionary_stored_here = (char **)malloc((dictionary_count+1) * sizeof(*dictionary_stored_here)); 
  rewind(fptr);
  int i = 0, j = 0; 

  char *line = NULL;
  size_t len = 0; 
  ssize_t read = -1; 
  char del[] = " \n\t";  
  char *token = NULL; 

  while(read = getline(&line, &len, fptr) != -1){
    // fill the array 

    token = strtok(line, del); // solve issue, extra space being added 

    dictionary_stored_here[i] = token;
    ++i; 
    line = NULL; // reset 
    token = NULL; 
  }

  dictionary_stored_here[i] = NULL;  
  free(line); 
  free(token);
  fclose(fptr); 
}

void init_some_vars(){
  // initilizate condition vars and mutexes 

  int res = pthread_mutex_init(&connection_q_lock, NULL);
  int res2 = pthread_mutex_init(&log_q_lock, NULL);
  int res3 = pthread_cond_init(&safe_to_add, NULL); 
  int res4 = pthread_cond_init(&safe_to_add2, NULL);
  int res5 = pthread_cond_init(&safe_to_remove, NULL); 
  int res6 = pthread_cond_init(&safe_to_remove2, NULL); 
  int res7 = pthread_attr_init(&detach);
  int res8 = pthread_attr_setdetachstate(&detach, PTHREAD_CREATE_DETACHED);

  if(res == -1 || res2 == -1 || res3 == -1 || res4 == -1 || res5 == -1 || res6 == -1 || res7 == -1 || res8 == -1){
    fprintf(stdout, "%s", "Error in init. of pthread locks and C.V.s, or pthread attribute");
    exit(1);
  } 
}

int check_dictionary(char *word){
  // sorted dictionary in dictionary.txt 
  // implement binary search to find a specific word

  char del[] = " \n\t\r"; // possible trailing characters, now works when test with nc AND telnet ('\r')
  word = strtok(word, del); // remove extra space/s / multiple words, ONLY CHECK FIRST WORD  

  //init indexes 
  int left, middle, right, target; 
  left = 0, 
  right = (dictionary_count-1); 

  while(left <= right){

    middle = (left + right) / 2;// cont. half the "array", until find term, or -1 

    target = strcmp(word, dictionary_stored_here[middle]); 

    if(target == 0){
      // found string, return index or TRUE / 0  

      return 0; // TRUE 
    }

    else if(target < 0){
      // less, check "left side" next halving 
      right = middle - 1; 
    }

    else{
      // greater, check "right side" 
      left = middle + 1; 
    }

  } 

  return 1; // FALSE / not found   
}

void add_to_connection_queue(int socket){
  // char buffer_connection_Q[SIZE]

  if(pthread_mutex_lock(&connection_q_lock) != 0){ // get lock
     fprintf(stdout, "%s", "failed to acquire lock");
     exit(1); 
  }  

  while(connection_q_count == SIZE){
    // while queue is full, release lock and wait, for condition (can add to queue) to be satisfied. 
    pthread_cond_wait(&safe_to_add, &connection_q_lock); 
  }

  // at this point, lock was re-aquired, safe to add to queue  
  buffer_connection_Q[add_index] = socket; // socket to add 

  add_index = (add_index + 1) % SIZE; 
  ++connection_q_count; 

  pthread_cond_signal(&safe_to_remove); // wake the consumer, safe to remove data from queue now 

   // release lock, for consumer to re-aquire 
  if(pthread_mutex_unlock(&connection_q_lock) != 0){
    fprintf(stdout, "%s", "failed to release lock");
    exit(1); 
  }
}

int remove_from_connection_queue(){
  // consumer 

  pthread_mutex_lock(&connection_q_lock); // get lock 

  while(connection_q_count == 0){
    // while queue is empty, release lock and wait, for condiition (can remove data from queue)
    pthread_cond_wait(&safe_to_remove, &connection_q_lock); 
  }

  // safe to remove data from queue 
  int temp = buffer_connection_Q[remove_index]; 
  remove_index = (remove_index + 1) % SIZE; 
  --connection_q_count; 


  pthread_cond_signal(&safe_to_add); // wake up producer, safe to add data 
  pthread_mutex_unlock(&connection_q_lock); 

  return temp; 
}

void create_threads(){
  // create thread pool of worker threads, who will "live inside" the worker_thread function 
  // in addition to single log thread, who will "live inside" the log_thread function 

  pthread_t w_threads[NUM_WORKERS]; // w_threads_count determined by constant NUM_WORKERS (set it to 50)

  long int id = 0; // w_thread id  

  for(int i = 0; i<NUM_WORKERS; ++i){

    ++id; 

    if(pthread_create(&w_threads[i], &detach, &worker_thread, (void*)id) != 0){
      // each thread created will begin execution in worker_thread() func. 

      fprintf(stderr, "%s\n", "Failed to create worker thread"); 
      exit(1); 
    }

  }

  // spawn log thread 
  pthread_t logger; 
  int res = pthread_create(&logger, NULL, &log_thread, NULL); // log thread, write results to data.log (results from all clients)

  if(res == -1){
    fprintf(stderr, "%s\n", "Failed to create log thread"); 
    exit(1); 
  }

}

void* worker_thread(void* id){
  // job is to remove a socket des. from connection queue (consumer) 
  // "read" a word passed through socket 
  // use check_dictionary(char *word) to check spelling
  // add result to log queue 
  // loop, until client leaves, close socket 

  char word[30]; // I don't think a word will be longer 
  int res; 
  char *welcome = "Welcome to Network Spell Checker ...\n"; 

  while(1){
    int sd = remove_from_connection_queue(); // remove sd from queue   
    write(sd, welcome, strlen(welcome));

    while(read(sd, word, 30) > 0){
    // loop until read/write fails / client disconnects 

      res = check_dictionary(word);  // use binary search, on already sorted dictionary structure 
       
      char word_plus_status[100] = ""; // strcat, word + space + status 
      strcat(word_plus_status, word); 
      strcat(word_plus_status, " "); 

      if(res == 0){
        // found word, write "word + OK" to socket 

        strcat(word_plus_status, "OK"); 
        strcat(word_plus_status, "\n"); 
      }

      else{
        // not found, write "word + MISSPELLED" to socket 

        strcat(word_plus_status, "MISSPELLED");  
        strcat(word_plus_status, "\n"); 
      }

      printf("Value to be written: %s\n", word_plus_status); 
      ssize_t bytes_written = write(sd, word_plus_status, strlen(word_plus_status)); // communicate response back to client 

      if(bytes_written == -1){
        fprintf(stderr, "%s", "Failed to write word"); 
        break; 
      }

      add_to_log_queue(word_plus_status);  // add word_plus_status to log queue 
     
    } // error / client disconected 
    
    long w_thread_id = (long int) id; // arg passed from pthread create func. 
    fprintf(stdout, "%s%ld%s%d\n", "Thread ", w_thread_id, " closed socket: ", sd);

    // Note: don't want to kill thread, new clients can use this thread now ... 

    close(sd); // close socket   
  }
}

void add_to_log_queue(char *result){
  // worker thread (producer)
  // same implementation as add_to_connection_queue(int socket) ... 
  // char *buffer_log_Q[SIZE];  

  if(pthread_mutex_lock(&log_q_lock) != 0){ // get lock
    fprintf(stdout, "%s", "failed to acquire lock");
    exit(1); 
  }  

  while(log_q_count == SIZE){
    // while queue is full, relase lock and wait, for condition (can add to queue) to be satisfied. 
    pthread_cond_wait(&safe_to_add2, &log_q_lock); 
  }

  ++log_q_count; 

  // at this point, lock was re-aquired, safe to add to queue   
  buffer_log_Q[add_index2] = result; // result (WORD + OK/MISSPELLED) to add   

  //printf("add to logQ: %s at index: %d\n", buffer_log_Q[add_index2], add_index2);

  add_index2 = (add_index2 + 1) % SIZE; // circular buffer, when "come full circle", start at 0 again 
 
  pthread_cond_signal(&safe_to_remove2); // wake the consumer, safe to remove data from queue now 

  // release lock, for consumer to re-aquire 
  if(pthread_mutex_unlock(&log_q_lock) != 0){
    fprintf(stdout, "%s", "failed to release lock");
    exit(1); 
  }

}

char* remove_from_log_queue(){
  // log thread (consumer)
  // same implementation as remove_from_connection_queue() ... 

   if(pthread_mutex_lock(&log_q_lock) != 0){ // get lock
    fprintf(stdout, "%s", "failed to acquire lock");
    exit(1); 
  } 

  while(log_q_count == 0){
    // while queue is empty, release lock and wait, for condiition (can remove data from queue)
    pthread_cond_wait(&safe_to_remove2, &log_q_lock); 
  }

  --log_q_count;  

  // safe to remove data from queue 
  char *temp = buffer_log_Q[remove_index2]; 

  //printf("removed from logQ: %s at index %d\n", temp, remove_index2);

  remove_index2 = (remove_index2 + 1) % SIZE; 
  
  pthread_cond_signal(&safe_to_add2); // wake up producer, safe to add data 

  if(pthread_mutex_unlock(&log_q_lock) != 0){ // release lock 
    fprintf(stdout, "%s", "failed to release lock");
    exit(1); 
  }
  
  return temp; 
}

void* log_thread(void *arg){
  // take string message (word + OK/MISSPELLED), write to log file 
  // all clients connect, data is stored to a single log file --> "data.log"

  fptr = fopen("data.log", "w"); 

  char *add = NULL; 

  while(1){
    // if log_q is empty, will wait on a condition ...
    // non-empty, removes the "first" ( circular buffer, first is the current index remove_index2) string from the buffer 
    // add to the log file, fflushing the buffer/stream to ensure no data loss. 

    add = remove_from_log_queue();
    fprintf(fptr, "%s", add);
    fflush(fptr); 
 }

  free(add);  
  fclose(fptr); 
}


void sigint_handler(int sig_num) {   
  // ctrl-c, if there were not multiple threads and all the possible blocking, could use a flag
  // and do clean_up outside main loop, but there are so I focus on cleaning up heap memory
   
  for(int i = 0; i<dictionary_count; ++i){
    // free each string 
    if(dictionary_stored_here[i] == NULL)
      continue; 

    free(dictionary_stored_here[i]); 
  } 

  free(dictionary_stored_here); // finally, free the structure 
  fclose(fptr); 

  exit(1); 
}


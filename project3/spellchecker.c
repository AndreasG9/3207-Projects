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

#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8888
#define SIZE 10 // ??? whats a good size fix/ increase later 

// network 
int socket_desc;
int port_number; 

// dictionary 
char *dictionary; // default is "dictionary.txt" 
char **dictionary_stored_here; // every string in dictionary, stored here, accessible by all threads 
int dictionary_count; // too allocate enough memory for double ptr (each string ref by ptr)

// producer-consumer, connection queue 
int buffer_connection_Q[SIZE]; 
int q_count; 
int add_index, remove_index; 
int w_threads_count; 
pthread_mutex_t lock; 
pthread_cond_t safe_to_add; // condition, wait until space available to add to queue 
pthread_cond_t safe_to_remove; // condition, wait until you can remove data from queue 

// producer-consumer, log queue 
char *buffer_log_Q[SIZE]; 
int q_count2; 
int add_index2, remove_index2; 
pthread_mutex_t lock2;
pthread_cond_t safe_to_add2;  
pthread_cond_t safe_to_remove2; 

// main thread 
void fill_dictionary_structure(); 
void init_some_vars(); 
int check_dictionary(char *word); 
void add_to_connection_queue(int socket); 
void create_worker_threads(); 

// worker threads 
void* worker_thread(void* arg); 
int remove_from_connection_queue(); 
void add_to_log_queue(char *result); 

// log thread 
void* log_thread();
char* remove_from_log_queue(); 

FILE *fptr2; 

void print_queue(); // testing 

int main (int argc, char *argv[]){
  // main thread 
  
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
    else if((access(argv[2], F_OK) == -1) && (isdigit(*argv[1]))){
      // file doesn't exist, but port num could work 
      port_number = atoi(argv[1]);
      dictionary = DEFAULT_DICTIONARY; 
    }
    else if((access(argv[1], F_OK) == -1) && (isdigit(*argv[2]))){
      // file doesn't exist, but port num could work 
      port_number = atoi(argv[2]);
      dictionary = DEFAULT_DICTIONARY; 
    }
    else{
       // tested all instances, this is just a back-up 
      dictionary = DEFAULT_DICTIONARY;
      port_number = DEFAULT_PORT; 
    }
  }
  
  else{
    // to many args

    fprintf(stderr, "%s", "TOO MANY ARGS"); 
    exit(1); 
  }

  printf("file: %s\n", dictionary);
  printf("port num: %d\n", port_number); 

  // INIT 
  q_count, add_index, remove_index = 0; 
  w_threads_count = 0; 
  q_count2, add_index2, remove_index2 =0; 

  fptr2 = fopen("data.log", "w"); 

  char *word = malloc(sizeof(char*) * 1024); // FREE LATER FREE LATER (after close socket)

  fill_dictionary_structure(); // char **dictionary_stored_here, hold a string for each line in dictionary file  
  init_some_vars();   // init condition vars and mutexes/locks 

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

  //puts("Bind done.");

  // Listen (converts active socket to a LISTENING socket which can accept connections)
  listen(socket_desc, 3);
  //puts("Waiting for incoming connections...");
  while (1){
    // MAIN LOOP 

	  c = sizeof(struct sockaddr_in);
	  new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);

	  if (new_socket < 0){
	    fprintf(stderr, "%s","Error: Accept failed");
	    continue;
	  }
	  fprintf(stdout, "%s","Connection accepted");

    add_to_connection_queue(new_socket); // add the socket des. to the buffer 

    // spawn worker threads who remove data from queue (consumer), will write to socket the word + OK/MISSPELLED
    // bulk of work done within these thread/s 
    ++w_threads_count; 
    create_worker_threads(); 

    log_thread(); // spawn log thread, write results to data.log 


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
  dictionary_stored_here = malloc(sizeof(char*) * dictionary_count); 
  rewind(fptr);
  int i = 0, j = 0; 

  char *line = NULL;
  size_t len = 0; 
  ssize_t read = -1; 
  char del[] = " \n\t";  

  while(read = getline(&line, &len, fptr) != -1){
    // fill the array 

    char *token = strtok(line, del); // solve issue, extra space being added 

    dictionary_stored_here[i] = token;
    ++i; 
    line = NULL; // reset 

  }

  dictionary_stored_here[i] = NULL;  

  fclose(fptr); 
}

void init_some_vars(){
  // initilizate condition vars and mutexes 

  pthread_mutex_init(&lock, NULL);
  pthread_mutex_init(&lock2, NULL);
  pthread_cond_init(&safe_to_add, NULL); 
  pthread_cond_init(&safe_to_add2, NULL);
  pthread_cond_init(&safe_to_remove, NULL); 
  pthread_cond_init(&safe_to_remove2, NULL); 
}


int check_dictionary(char *word){
  // sorted dictionary in dictionary.txt 
  // implement binary search to find a specific word

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

  pthread_mutex_lock(&lock); // get lock 

  while(q_count == SIZE){
    // while queue is full, release lock and wait, for condition (can add to queue) to be satisfied. 
    pthread_cond_wait(&safe_to_add, &lock); 
  }

  // at this point, lock was re-aquired, safe to add to queue  
  buffer_connection_Q[add_index] = socket; // socket to add 

  //++add_index; 
  add_index = (add_index + 1) % SIZE; 
  ++q_count; 

  // if add_index count == SIZE/ MAX, reset to 0, signal consumer as Queue is filled 

  pthread_cond_signal(&safe_to_remove); // wake the consumer, safe to remove data from queue now 
  pthread_mutex_unlock(&lock); 
}

int remove_from_connection_queue(){
  // consumer 

  pthread_mutex_lock(&lock); // get lock 

  while(q_count == 0){
    // while queue is empty, release lock and wait, for condiition (can remove data from queue)
    pthread_cond_wait(&safe_to_remove, &lock); 
  }

  // safe to remove data from queue 
  int temp = buffer_connection_Q[remove_index]; 
  remove_index = (remove_index + 1) % SIZE; 
  --q_count; 

  //

  pthread_cond_signal(&safe_to_add); // wake up producer, safe to add data 
  pthread_mutex_unlock(&lock); 

  return temp; 
}

void create_worker_threads(){
  // create thread pool of worker threads 
  
  pthread_t threads[w_threads_count]; // w_threads_count determined by # of sockets added to queue in main thread 

  for(int i = 0; i<w_threads_count; ++i){

    if(pthread_create(&threads[i], NULL, &worker_thread, NULL) != 0){
      // each thread created will begin execution in worker_thread() func. 

      fprintf(stderr, "%s", "Failed to create worker thread"); 
      exit(1); 
    }
  }

}

void* worker_thread(void* arg){
  // job is to remove a socket des. from connection queue (consumer) 
  // "read" a word passed through socket 
  // use check_dictionary(char *word) to check spelling
  // add result to log queue 
  // loop, until client leaves, close socket 
  puts("\nin worker_thread() function"); 

  char word[75]; // I don't think a word will be longer 

  int sd = remove_from_connection_queue(); // remove sd from queue 

  while(read(sd, word, 75) > 0){
    // loop until read/write fails / client disconnects 

    //ssize_t bytes_read = read(sd, word, 75); 


    // if(bytes_read == -1){
    //   fprintf(stderr, "%s", "Failed to read word"); 
    // }



    // remove extra space/s / multiple words, ONLY CHECK FIRST WORD 
    char del[] = " \n\t";  
    char *check_word = strtok(word, del); 

    //printf("Word to br written: %s\n", check_word); 

    int res = check_dictionary(check_word);  // used binary search, on already sorted dictionary structure 

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

    ssize_t bytes_written = write(sd, word_plus_status, strlen(word_plus_status)); 

    if(bytes_written == -1){
      fprintf(stderr, "%s", "Failed to write word"); 
      break; 
    }

    add_to_log_queue(word_plus_status);  // add word_plus_status to log queue 

  } // error / client disconected 

  close(sd); // close socket
}

void add_to_log_queue(char *result){
  // worker thread (producer)
  // same implementation as add_to_connection_queue(int socket) ... 
  // char *buffer_log_Q[SIZE]; 

  pthread_mutex_lock(&lock2); // get lock 

  while(q_count2 == SIZE){
    // while queue is full, relase lock and wait, for condition (can add to queue) to be satisfied. 
    pthread_cond_wait(&safe_to_add2, &lock2); 
  }

  // at this point, lock was re-aquired, safe to add to queue  
  buffer_log_Q[add_index2] = result; // result (WORD + OK/MISSPELLED) to add  

  add_index2 = (add_index2 + 1) % SIZE; 
  ++q_count2; 

  // if add_index count == SIZE/ MAX, reset to 0, signal consumer as Queue is filled 

  pthread_cond_signal(&safe_to_remove2); // wake the consumer, safe to remove data from queue now 
  pthread_mutex_unlock(&lock2);

}

char* remove_from_log_queue(){
  // log thread (consumer)
  // same implementation ass remove_from_connection_queue() ... 

  pthread_mutex_lock(&lock2); // get lock 

  while(q_count2 == 0){
    // while queue is empty, release lock and wait, for condiition (can remove data from queue)
    pthread_cond_wait(&safe_to_remove2, &lock2); 
  }

  // safe to remove data from queue 
  char *temp = buffer_log_Q[remove_index2]; 
  remove_index2 = (remove_index2 + 1) % SIZE; 
  --q_count2; 

  pthread_cond_signal(&safe_to_add2); // wake up producer, safe to add data 
  pthread_mutex_unlock(&lock2); 
  
  return temp; 
}

void* log_thread(){
  // take string message (word + OK/MISSPELLED), write to log file 

  fopen("data.log", "a+"); // MAIN 

  while(1){

    char *append = remove_from_log_queue();

    printf("value to add log: %s\n", append); 
 
    // add to log file 
    // close program with ctrl + c, wont lose data now 
    fputs(append, fptr2); 
    fflush(fptr2); 
  }

  fclose(fptr2); // close, after log a word, prevent loss of data 

}


void print_queue(){
  // TESTING 

  for(int i = 0; i<SIZE; ++i)
    printf("%d", buffer_connection_Q[i]); 

}



 


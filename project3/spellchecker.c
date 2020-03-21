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

#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8888
#define SIZE 10 // ??? whats a good size fix/ increase later 

// global vars
char *dictionary; // default is "dictionary.txt" 
char **dictionary_stored_here; // every string in dictionary, stored here, accessible by all threads 
int dictionary_count; // too allocate enough memory for double ptr (each string ref by ptr)


// producer-consumer, connection queue 
int buffer_connection_Q[SIZE]; 
int q_count; 
int add_index, remove_index; 

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t safe_to_add = PTHREAD_COND_INITIALIZER; // condition, wait until space available to add to queue 
pthread_cond_t safe_to_remove = PTHREAD_COND_INITIALIZER; // condition, wait until you can remove data from queue 
//pthread_cond_init(safe_to_add); 
int w_threads_count; 

// producer-consumer, log queue 
int q_count2; 
int add_index2, remove_index2; 
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t safe_to_add2 = PTHREAD_COND_INITIALIZER;  
pthread_cond_t safe_to_remove2 = PTHREAD_COND_INITIALIZER;


// main thread 
void fill_dictionary_structure(); 
int check_dictionary(char *word); 
void add_to_connection_queue(int socket); 
void create_worker_threads(); 

// worker threads 
void* worker_thread(); 
int remove_from_connection_queue(); 
void print_queue(); // testing 

// log thread 




int main (int argc, char *argv[]){
  // main thread 

  /*
  if(argc == 1){
    // no dicitonary or port specified, use default values
    dictionary = DEFAULT_DICTIONARY;
    port = 401; 
  }
  else if(argc > 3){
    // too many args 
    fprintf(stderr, "%s", "takes 0, 1, or 2 args"); 
    exit(1); 
  }
  else if(argc == 2){
    // isdigit to see if dictionary excluded, port number included 

  }
  */ 

  int port_number;  

  // use defaults for now
  dictionary = NULL; 
  dictionary = DEFAULT_DICTIONARY;
  port_number = DEFAULT_PORT; 
  q_count = 0, add_index = 0, remove_index = 0; 
  w_threads_count = 0; 

  char *word = malloc(sizeof(char*) * 1024); 
  // printf("Testing! Enter a word: ");
  // scanf("%s", word); 
  
  fill_dictionary_structure(); // char **dictionary_stored_here, hold a string for each line in dictionary file 
  // int res = check_dictionary(word); 

  // if(res == 0)
  //   puts("TESTING: FOUND");
  // else
  //   puts("TESTING: NO"); 

  
  // main thread (load dictionary, create other threads, init network connection, wait clients to connect, place socket descriptor of given connection on queue)

  // ** setup socket, accept connection, put socket in queue (queue will contain connection fd, producer: main, consumer: worker)

  
  // NETWORK setup (given code)
  int socket_desc, new_socket, c;

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
	  fprintf(stderr, "%s","Connection accepted");


    add_to_connection_queue(new_socket); // add the socket to the queue, which is the fixed circular char buffer_connection_Q[SIZE]
    ++w_threads_count; 
    create_worker_threads(); // spawn worker threads who remove data from queue (consumer)
    // spawn log thread 

    // repeat loop ... 
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
    // while queue is full, relase lock and wait, for condition (can add to queue) to be satisfied. 
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

void* worker_thread(){
  // job is to remove a socket des. from connection queue (consumer) 
  // "read" a word passed through socket 
  // use check_dictionary(char *word) to check spelling
  // add result to log queue 
  // loop, until client leaves, close socket 
   puts("\nin worker_thread() function"); 

  char word[75]; // I don't think a word will be longer 
  
  int sd = remove_from_connection_queue();  // remove sd from queue 


  while(1){
  ssize_t bytes_read = read(sd, word, 75); 

  if(bytes_read == -1){
    fprintf(stderr, "%s", "Failed to read word"); 
  }

  // remove extra space added, 
  char del[] = " \n\t";  
  char *check_word = strtok(word, del); 

  //printf("Word to br written: %s\n", check_word); 


  int res = check_dictionary(check_word);  // used binary search, on already sorted dictionary structure 

  char word_plus_status[100] = ""; // strcat, word + space + status 
  strcat(word_plus_status, word); 
  strcat(word_plus_status, " "); 

  if(res == 0){
    // found word, write "OK" to socket 

    //puts("test: found"); 

    strcat(word_plus_status, "OK"); 
    printf("%s\n", word_plus_status); 

    ssize_t bytes_written = write(sd, strcat, strlen(word_plus_status)); 

    if(bytes_read == -1){
      fprintf(stderr, "%s", "Failed to read word"); 
    }
  }

  else{
    // not found, write "MISPELLED" to socket 

   // puts("test: no"); 

    strcat(word_plus_status, "MISSPELLED"); 
    printf("%s\n", word_plus_status); 

    ssize_t bytes_written = write(sd, strcat, strlen(word_plus_status)); 

    if(bytes_read == -1){
      fprintf(stderr, "%s", "Failed to read word"); 
    }
  }

  // add word_plus_status to log queue 


  } // error, or client disconected 


}



void print_queue(){
  // TESTING 

  for(int i = 0; i<SIZE; ++i)
    printf("%d", buffer_connection_Q[i]); 

}



 


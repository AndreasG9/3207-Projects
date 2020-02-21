/*
Project 2
Andreas Gagas
myshell.c - Developing a Linux Shell
*/

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>

char** get_user_input(); 
char ** read_batch_file(); 
void quick_error_check(char **input); 
void trim_spaces(char **input); 
int is_built_in_command(char **input, char *my_commands); 


//int check_for_redirection(char **input, char *new_input, char *new_output); 
 
int input_argc; 

int main(int argc, char *argv[]){

  char *commands[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "path"}; 
  char *input_file, *output_file, *p_input, *p_output = NULL; 

  char **input = NULL; 
  input_argc = 0; 
  
  int batch_present = 0; 

  if(argc == 2)
    batch_present = 1; 
    
  if(argc > 2)
     exit(1); 

  // START LOOP 
  while(1){

    if(batch_present){
      // batch mode 
      input = read_batch_file(); 
    }

    else{
      //interactive mode 

      printf("%s", "myshell> "); 
      input = get_user_input(); 


      char **print = input; 

      while(*print){
        printf("%s\n", *(print)); 
        ++print; 
      }
   }

    printf("%d\n", input_argc);

    break; 

    }

  return 0; 
}



char** get_user_input(void){

  char *line, *res = NULL;
  size_t len = 0; 
  ssize_t read = -1; 

  char del[] = " \n\t";  
  char **tokens = (char **)malloc(sizeof(char*) * 50);
  int i = 0; 

  //while((read = getline(&line, &len, stdin)) != -1){
  read = getline(&line, &len, stdin); 

  tokens[i] = strtok(line, del);

  while(tokens[i] != NULL){
    //printf("\n%s\n", tokens[i]); 
    ++input_argc; 
    ++i; 
    tokens[i] = strtok(NULL, del);  
  }

  return tokens; 
}

char ** read_batch_file(){

  return 0; 
}

void quick_error_check(char **input){
// Quickly iterate through the array, input[0] and input[n-1] can't have any redirection, control, or pipe symbol. 
// Global variable input_argc should have the count. 

int first = 0;
int last = input_argc; 
//char cant_have[] = {'|', '>', '<', '&'}; // '>>' will be caught by '>' 





}

void trim_spaces(char **input){
  // remove blank entries, and modify the count of the input/ argc (global variable)
}

int is_built_in_command(char **input, char *my_commands){
  // filter through the input indexes for a match 



  return 0; 
}

int check_for_redirection(char **input, char *new_input, char *new_output){ 
  // 

  return 0; 
}



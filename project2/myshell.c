/*
Project 2
Andreas Gagas
myshell.c - Developing a Linux Shell
*/

#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <ctype.h> 
#include <unistd.h>
#include <dirent.h> 
#include <sys/types.h>
#include<sys/wait.h> 
// ... 

char** get_user_input(); 
char** read_batch_file(); 
int quick_error_check(char **input); 
void trim_spaces(char **input); 
int is_built_in_command(char **input); 
int cd(char **input); 
int clr(char **input); 
int dir(char **input); 
int environ(char **input); 
int echo(char **input); 
int help(char **input); 
int pause_(char **input); 
int quit(char **input);
// path 
void group_command_argument(char **input); 
int check_for_redirection(char **input, char *new_input, char *new_output); 


void run_external_command(char **input); // add all those params 


char *commands[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "path"}; 

 
int input_argc; 

int main(int argc, char *argv[]){

  //char *commands[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "path"}; 
  char *new_input_file, *new_output_file, *p_input, *p_output = NULL; 

  char **input = NULL; 
  input_argc = 0; 
  
  int batch_present = 0; 

  if(argc == 2)
    batch_present = 1; 
    
  if(argc > 2)
     exit(1); 

  // START LOOP 
  while(1){

    if(batch_present)
      input = read_batch_file(); 

    else{
      //interactive mode 

      printf("%s", "myshell> "); 

      input = get_user_input(); 

      char **print = input; 

      while(*print){
       // printf("%s\n", *(print)); 
        ++print; 
      }
   }

    int quick_error = quick_error_check(input); // a return of 1 if the first or last input contained an invalid command, error msg already printed 

    if(quick_error){
      // skip the rest of the loop body, user will see "myshell> ", indicating them to use the shell again 
      continue; 
    }

   // trim_spaces(input); // remove blank entries if present 

    int built_in =  is_built_in_command(input); 
    //printf("built-in: %d\n", built_in); 

    int successful; // 0 for YES 

    // call built in function if present 
    if(strcmp(input[0], "cd") == 0)
      successful = cd(input);  

    else if(strcmp(input[0], "clr") == 0)
      successful = clr(input); 

    else if(strcmp(input[0], "dir") == 0)
      successful = dir(input); 

    else if(strcmp(input[0], "environ") == 0)
      successful = environ(input); 

    else if(strcmp(input[0], "echo") == 0)
      successful = echo(input); 

    else if(strcmp(input[0], "help") == 0)
      successful = help(input); 

    else if(strcmp(input[0], "pause") == 0)
      successful = pause_(input); 

    else if(strcmp(input[0], "quit") == 0)
      successful = quit(input); 

    // built-in call not succesful, continue
    if(successful == 1)
      continue; 

    int redirection_present = check_for_redirection(input, new_input_file, new_output_file); 

    // execute external command 

    //run_external_command(input); 


    break; 

  }

  return 0; 
}


char** get_user_input(void){

  //printf("%s", "myshell> "); 

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
// TODO 

  return 0; 
}

int quick_error_check(char **input){
  // Quickly iterate through the array, input[0] and input[n-1] can't have any redirection, control, or pipe symbol. 
  // Global variable input_argc should have the count. 
  
  int first = 0;
  int last = input_argc; 
  char *cant_have[5] = {"|", ">", "<", ">>", "&"};

  for(int i = 0; i<5; ++i){

    if(strcmp(input[0], cant_have[i]) == 0){
      fprintf(stderr, "%s %s\n", "-myshell: syntax error near unexpected token: ", cant_have[i]); 
      return 1; 
    }

    if(strcmp(input[input_argc-1], cant_have[i]) == 0){
      fprintf(stderr, "%s %s\n", "-myshell: syntax error near unexpected token: ", cant_have[i]); 
      return 1; 
    }
  }
  return 0; 
}

void trim_spaces(char **input){
  // shift the non-empty strings to the left, empty strings will be ignored. DECREMENT INPUT ARGC COUNT THEN 

  // TODO 

  for(int i=0; i<5; ++i){

    if(isspace(*input[i])){

      input[i] = input[i+1];
      input[i+1] = "";
      --input_argc; 
      // not really removing them, just shifting 
    }
  }

}

int is_built_in_command(char **input){
  // filter through the input indexes for a match 

  for(int i = 0; i<(sizeof(commands)/sizeof(commands[0])); ++i){
    // loop through my_commands, if input[0] equals a command, return 1 
    // Note: errors will be checked inside the respective function, so if the user inputs "help clear", too many args will be found inside the 
    // the help function. 

    if(strcmp(input[0], commands[i]) == 0)
      return 1; 
  }

  return 0; 
}

int clr(char **input){
  // built-it command to clear the screen

  if(input_argc > 1){
    fprintf(stderr, "%s \n", "-myshell: error, clr takes no arguments");
    return 1; 
  }

  printf("\033[H\033[2J"); 

  return 0;    
}

int cd(char **input){
  // cd <directory> 
  // also change the pwd 

  if(input_argc == 1){
    //fprintf(stderr, "%s \n", "-myshell: error, cd has too few arguments"); 
    // print the current directory 
    char buffer[250]; 
    printf("%s\n", getcwd(buffer, sizeof(buffer)));

    return 0; 
  }

  int retval = 0;  

  retval = chdir(input[1]); 

  if(retval == -1){
    fprintf(stderr, "%s \n", "-myshell: error, directory does not exist"); 
    return 1; 
  }

  return 0; 
}

int dir(char **input){
  // dir <directory> 
  // supports output redirection 

  char *name = malloc(sizeof(char) * 150); 
  //char name[2] = "/"; 

  if(input_argc > 2){
    fprintf(stderr, "%s \n", "-myshell: error, too many args");
    return 1; 
  }

  if(input_argc == 1){
    // print current dir 

    getcwd(name, 150); 
    //printf("%s\n", name); 
  }

  else if(input_argc == 2){
    // used passed argument
    name = input[1]; 
  }

  DIR *directory = NULL;
	struct dirent *directory_entry = NULL; 
	//directory = opendir(name); 
  directory = opendir(name); 

  // redirection later ! (just implement your pseudocode)

  if(directory == NULL)
    fprintf(stderr, "%s \n", "-myshell: error, <directory> not found");

  // read from directory
  while((directory_entry = readdir(directory)) != NULL)
    printf("%s\t", directory_entry->d_name); 


return 0; 
}


int environ(char **input){
  // list env strings 
  // supports output redirection 

  if(input_argc > 0){
    fprintf(stderr, "%s \n", "-myshell: error, environ takes no args");
    return 1; 
  }
  
  // check for redirection 

  char *envar1, *envar2, *envar3 = NULL; 
  envar1 = getenv("USER");
  envar2 = getenv("PATH"); 
  envar3 = getenv("HOME"); 

  printf("USER: %s\n\nPATH: %s\n\nHOME: %s\n\n", envar1, envar2, envar3); 

  return 0; 
}

int echo(char **input){
  // echo <comment> 
  // support output redirection 

  int output_file; 

  // check for redirection 

  // else{ 
  for(int i = 1; i<(input_argc); ++i){
    printf("%s ", input[i]); 
  }

  puts(""); 

  return 0; 
}

int help(char **input){
  // print help.txt 
  // supports output redirection 

  

  // check for redirection 

  // else 

  return 0; 
}


int pause_(char **input){
  // "pause" shell, until user hits enter ('\n')

  int retval; 

  while(retval = getchar() != '\n')
	  continue; 

  return 0; 
}

int quit(char **input){
  // exit the shell 

  free(input); 
  exit(0); 

  return 0; 
}

void group_command_argument(char **input){
  // ... 

  
}

int check_for_redirection(char **input, char *new_input, char *new_output){
  // "<", ">", ">>"

  // fix return value 

  //char *symbols[] = {"<", ">", ">>"}; 

  for(int i=1; i<input_argc; ++i){
    // iterate through inputs

    if(strcmp(input[i], "<") == 0){
      // store the next arg (to the right)
      new_input = input[i+1]; 

      if((strcmp(new_input, ">") == 0 || strcmp(new_input, ">") == 0) || strcmp(new_input, ">>") == 0 || 
         strcmp(new_input, "|") == 0 || strcmp(new_input, "&") == 0){

         fprintf(stderr, "%s \n", "-myshell: error, < requires input file");
          }
    }

     else if(strcmp(input[i], ">") == 0){
         new_output = input[i+1]; 

         if((strcmp(new_input, ">") == 0 || strcmp(new_input, ">" == 0) || strcmp(new_input, ">>" == 0 || 
           strcmp(new_input, "|") == 0 || strcmp(new_input, "&") == 0){

          fprintf(stderr, "%s \n", "-myshell: error, > requires input file");
           }

      else if(strcmp(input[i], ">>") == 0){
          new_output = input[i+1]; 

    }

    //else if(strcmp(input[0], "clr") == 0)

return 0; 
}


void run_external_command(char **input){
// quick version 

  int pid = fork(); 

  if(pid == -1)
    puts("error"); 

  else if(pid == 0)
    execvp(input[0], input); 

  else{
    int wc = wait(NULL);  
  }

}









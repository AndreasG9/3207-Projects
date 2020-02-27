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
#include <sys/wait.h> 
//#include <sys/stat.h>
//#include <fcntl.h>
// ... 

char** get_user_input(); 
char** read_batch_file(); 
int quick_error_check(char **input); 
int is_built_in_command(char **input); 
int cd(char **input); // fix fix fix 
int clr(char **input); 
int dir(char **input); 
int environ(char **input); 
int echo(char **input); 
int help(char **input); 
int pause_(char **input); 
int quit(char **input);
// path 
void group_command_option(char **input); 
//int check_for_redirection(char **input, char *new_input, char *new_output); 
int check_for_redirection(char **input); // 0 for < || 1 for > || 2 for ERORR || 3 for >> || 4 for BOTH || 5 for NONE
// check for single pipe
// check for control & 

void run_external_command(char **input); // add all those params 

// some global var 
char *commands[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "path"}; 
char *input_file, *output_file, *p_input, *p_output; 
int input_argc; 


int main(int argc, char *argv[]){

  //char *commands[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "path"}; 
  //char *new_input_file, *new_output_file, *p_input, *p_output = NULL; 
  //new_input_file, new_output_file, p_input, p_output = NULL;

  input_file = NULL, output_file = NULL, p_input = NULL, p_output = NULL;  
  input_argc = 0; 

  char **input = NULL; // input argc[]
  
  int batch_present = 1; 

  if(argc == 2)
    batch_present = 0; 
    
  if(argc > 2) // mode not recognized 
     exit(1); 

  // START LOOP 
  while(1){

    if(batch_present == 0)
      input = read_batch_file(); 

    else{
      //interactive mode 

      printf("%s", "myshell> "); 

      input = get_user_input(); 

      char **print = input; 

      while(*print){
        //printf("%s\n", *(print)); 
        ++print; 
      }

     // printf("input_args: %d\n", input_argc); 
   }

    int quick_error = quick_error_check(input); // a return of 1 if the first or last input contained an invalid command, error msg already printed 

    if(quick_error){
      // skip the rest of the loop body, user will see "myshell> ", indicating them to use the shell again 
      continue; 
    }


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


    // ADD ANOTHER LOOP HERE ADD ANOTHER LOOP HERE ADD ANOTHER LOOP HERE ADD ANOTHER LOOP HERE  ADD ANOTHER LOOP HERE 
    // for(int i=0; i<num args; ++i){}

    //int redirection_present = check_for_redirection(input, new_input_file, new_output_file); 

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

    if(isspace(tokens[i])){
    // if the token contains an "empty" string (\n, \t, " "), decrement the count of i to replace that string 
      --i; 
    }
    
    tokens[i] = strtok(NULL, del); 
  }

  tokens[i] = '\0';  // added null terminator 

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

  if(input_argc > 2){
    fprintf(stderr, "%s \n", "-myshell: error, cd takes 1 argument"); 
    return 1; 
  }

  char buffer[250]; 

  if(input_argc == 1){
    // print the current directory 

    printf("%s\n", getcwd(buffer, sizeof(buffer)));

    return 0; 
  }

  
  int retval = 0;  

  retval = chdir(input[1]); 
  printf("%s\n", getcwd(buffer, sizeof(buffer)));

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
  name = NULL; 

  int output_redirection = check_for_redirection(input);

  printf("%d\n", output_redirection);

  FILE *fptr = NULL; 

  if(output_redirection == 1)
    fptr = fopen(output_file, "w"); // write to specified output file 
  
  if(output_redirection == 3)
    fptr = fopen(output_file, "a"); // append to specified outputfile 


  if(output_redirection == 1 || output_redirection == 3){
    if(fptr == NULL){
      fprintf(stderr, "%s \n", "-myshell: error, could not open output file");
      return 1; 
    }
  }


  if((input_argc > 2) && output_redirection != 1 && output_redirection != 3){ // fix later, multi-lined 
    fprintf(stderr, "%s \n", "-myshell: error, too many args");
    return 1; 
  }


  if(input_argc != 2) // print contents of the current dir 
    name = getcwd(name, 150); 

  else if(input_argc == 2) // user passed argument
    name = input[1]; 

  
  DIR *directory = NULL;
	struct dirent *directory_entry = NULL; 
  directory = opendir(name); 

  if(directory == NULL)
    fprintf(stderr, "%s \n", "-myshell: error, <directory> not found");

  // read from directory, print to screen or redirection output 
  while((directory_entry = readdir(directory)) != NULL){

   if(output_redirection == 1 || output_redirection == 3) // specified file is already opened to "w" or "a", just need to write to that file 
        fprintf(fptr, "%s    ", directory_entry->d_name); 
   else
     printf("%s    ", directory_entry->d_name); // print to stdout 
  }


  if(output_redirection == 1 || output_redirection == 3){
    // newline and close the fptr 
    fprintf(fptr, "%s", "\n"); 
    fclose(fptr);
  }

  return 0; // success 
}

int environ(char **input){
  // list env strings, supports output redirection 

  int output_redirection = check_for_redirection(input); 
  //printf("%s", outut_redirection); 
  
  if((input_argc > 1) && output_redirection != 1 && output_redirection != 3){
    fprintf(stderr, "%s \n", "-myshell: error, environ takes no args");
    return 1; 
  }

  if(output_redirection == 2) // error warning already printed 
    return 1; 

  char *envar1, *envar2, *envar3 = NULL; 
  envar1 = getenv("USER");
  envar2 = getenv("PATH"); 
  envar3 = getenv("HOME"); 

  if(output_redirection == 5)
    printf("USER: %s\n\nPATH: %s\n\nHOME: %s\n\n", envar1, envar2, envar3); 

  else{
    // output redirection

    FILE *fptr = NULL; 

    if(output_redirection == 1)
      fptr = fopen(output_file, "w"); 

    if(output_redirection == 3)
      fptr = fopen(output_file, "a"); 

    if(fptr == NULL){
      fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
      return 1; 
    }

    fprintf(fptr, "%s%s\n\n%s%s\n\n%s%s\n\n", "USER: ", envar1, "PATH: ", envar2, "HOME: ", envar3); // write to file regardless if > or >> 
    fclose(fptr); 
  }
  return 0; 
}

int echo(char **input){
  // echo <comment> 
  // support output redirection 

  int output_redirection = check_for_redirection(input); 

  if(output_redirection == 5){
    // print to stdout 

  for(int i = 1; i<(input_argc); ++i)
    printf("%s ", input[i]); 

  puts(""); 
  }

  else{
    // output redirection 

    FILE *fptr = NULL;
    char *symbol = NULL; // signal for for loop to end 

    if(output_redirection == 1){
      fptr = fopen(output_file, "w"); 
      symbol = ">"; 
    }

    if(output_redirection == 3){
      fptr = fopen(output_file, "a");  
      symbol = ">>"; 
    }

    if(fptr == NULL){
      fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
      return 1; 
    }

    //for(int i = 1; strcmp(input[i], ">") != 0; ++i)  
    for(int i = 1; strcmp(input[i], symbol) != 0; ++i) // print <comments> until we encouter the redirection symbol 
      fprintf(fptr, "%s ", input[i]); 
  

    fprintf(fptr, "%s", "\n"); 
    fclose(fptr); 
  }

  return 0; 
}

int help(char **input){
  // print help.txt 
  // supports output redirection 

  FILE *fptr = fopen("help.txt", "r");

    if(fptr == NULL){
      fprintf(stderr, "%s \n", "-myshell: error, could not open \"help.txt\"");
      return 1; 
    }

  char buffer[250]; // enough to print a line in help.txt 
  puts(""); 

  int output_redirection = check_for_redirection(input); 

  if(output_redirection == 5){
    // print to stdout 

    while(fgets(buffer, 250, fptr) != NULL)
      printf("%s", buffer); 

  }

  else{
    // redirect the output to specified file 

    FILE *fptr2 = NULL; 

      if(output_redirection == 1)
      fptr2 = fopen(output_file, "w"); 

    if(output_redirection == 3)
      fptr2 = fopen(output_file, "a");  
 
      if(fptr2 == NULL){
        fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
        return 1; 
      }

      while(fgets(buffer, 250, fptr) != NULL)
        fprintf(fptr2, "%s", buffer); 
  }

  fclose(fptr); 

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
  // doesn't really need a return value 

  free(input); 
  exit(0); 

  return 0; 
}

void group_command_option(char **input){
  // ... 
  // {"ls", "-la", ...} --> {"ls -la", ...} 

  
}

int check_for_redirection(char **input){ 
//int check_for_redirection(char **input, char *new_input, char *new_output){
  // "<", ">", ">>"

  // fix return value 

  //char *symbols[] = {"<", ">", ">>"}; 


  for(int i=1; i<input_argc; ++i){
    // iterate through inputs

    // if(strcmp(input[i], "<") == 0){
    //   // store the next arg (to the right)
    //   //new_input = input[i+1]; 
      
    //   //printf("inside redirection, new in: %s\n", new_input); 

    //   if((strcmp(new_input, ">") == 0 || strcmp(new_input, ">") == 0) || strcmp(new_input, ">>") == 0 || 
    //      strcmp(new_input, "|") == 0 || strcmp(new_input, "&") == 0){

    //      fprintf(stderr, "%s \n", "-myshell: error, < requires input file");
    //      return 2; 
    //       }
    //   return 0; 
    // }

     if(strcmp(input[i], ">") == 0){
         output_file = input[i+1]; 
        //printf("inside redirection, new out: %s\n", output_file); 

      if((strcmp(output_file, ">") == 0 || strcmp(output_file, ">") == 0) || strcmp(output_file, ">>") == 0 || 
         strcmp(output_file, "|") == 0 || strcmp(output_file, "&") == 0){

          fprintf(stderr, "%s \n", "-myshell: error, > requires output file");
          return 2; 
          }

        return 1; 
     }

    else if(strcmp(input[i], ">>") == 0){
        output_file = input[i+1]; 
        //printf("inside redirection, new out APPEND: %s\n", output_file); 

      if((strcmp(output_file, ">") == 0 || strcmp(output_file, ">") == 0) || strcmp(output_file, ">>") == 0 || 
         strcmp(output_file, "|") == 0 || strcmp(output_file, "&") == 0){

          fprintf(stderr, "%s \n", "-myshell: error, > requires output file");
          return 2; 
          }

        return 3; 
     }


  }
  return 5; // no redirection 
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









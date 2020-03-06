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
#include <sys/stat.h>
#include <fcntl.h>
// ... 

char** get_user_input(); 
char** read_batch_file(FILE *file_ptr); 
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
int path(char **input); 

void print_this(char **print_this); 
void parse(char **input); // trying this parse instead of shifting, I couldnt get & to work properly w/ shift 
//int check_for_redirection(char **input); // 0 for < || 1 for > || 2 for ERORR || 3 for >> || 4 for BOTH || 5 for NONE
int check_for_invalid_file(char *file); // ex. output redirection is present, input[i+1] is stored, check that input[i+1] is not another symbol, needs to be a file  
int check_for_pipe(char **input); // check for single pipe
int check_for_background(char **input); 

int check_for_input_redirection(char **input); 
int check_for_output_redirection(char **input); 

int run_external_command(char **input, int redirection, int background, int pipe_present); 
int run_external_cmd_pipe(int pipe_present); // run_external_command was too cluttered, simplifed viewing 

int path_check(); 

// basic global var 
char *commands[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "path"}; 
int input_argc; 
char error_message[30] = "An error has occured\n";  // **** change all error messages to this (maybe leave some specific ones)
int start_here; // index to start parser from input 
int custom_msg; // signal to use std error msg 

// for redirection  
char *input_file, *output_file; 
int append; // quick fix 

// for pipe 
char *write_side[4]; 
char  *read_side[4]; 
int pipe_index; 

// get input, store in input, parse, store in parsed
char **parsed; 

char **shell_path; 


int main(int argc, char *argv[]){

  FILE *fptr = NULL;

  shell_path = malloc(sizeof(char) * 200); 

  // UNDO LATER 
  shell_path[0] = "/bin"; // program starts with one directory 

  start_here = 0; 
  
  int batch_present = 1; 

  if(argc == 2){
    // open the batch file, read line by line inside loop 
    batch_present = 0; 
    fptr = fopen(argv[1], "r"); 
  }
    
  if(argc > 2) // mode not recognized 
     exit(1); 

  // START LOOP 
  while(1){

    // reset var 
    char **input = NULL;  
    input_argc = 0, start_here = 0, custom_msg = 0; 
    input_file = NULL, output_file = NULL; 
    parsed = NULL; 

    if(batch_present == 0){ 
      puts(""); // space between outputs 
      input = read_batch_file(fptr); // get a single line 
       //print_this(input); 

      if(input == NULL)
        exit(0); // reached EOF 
    }

    else{
      //interactive mode 
      
      char buffer[100]; 
      printf("%s  ", getcwd(buffer, sizeof(buffer)));
      printf("%s", "myshell> "); 
    
      input = get_user_input(); 

      if(input == NULL)
        continue; // no input 
    }
  
      int quick_error = quick_error_check(input); // a return of 1 if the first or last input contained an invalid command, custom error msg already printed 

      if(quick_error){
        // skip the rest of the loop body, user will see "myshell> ", indicating them to use the shell again (in interactive mode)
        continue; 
      }
    
      int built_in =  is_built_in_command(input);   

      int successful; // 0 for YES 

      if(built_in == 0){
        // call built in function if present 
        // don't need to parse, direct implementation inside func. handlers 

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

        else if(strcmp(input[0], "path") == 0)
          successful = path(input);

        else if(strcmp(input[0], "quit") == 0){
          // variables that were malloced: input, parsed, and shell_path 
          int already_null, already_null2, already_null3 = 1;  
          for(int i= 0; i<input_argc; ++i){

            if(input[i])
              already_null = 0; 
            
            if(parsed[i])
              already_null2 = 0;

            if(shell_path[i])
              already_null3 = 0; 

            if(already_null)
              free(input[i]);
            if(already_null2)   
              free(parsed[i]); 
            if(already_null3)
              free(shell_path[i]); 
          }

          free(input); 
          free(parsed); 
          free(shell_path); 
          successful = quit(input); 
        }

     if(successful == 1 && custom_msg == 0) // built-in call not succesful, print standard msg if custom msg not already printed 
       write(STDERR_FILENO, error_message, strlen(error_message)); // have custom messages for BUILT IN, will delete 

      continue; // continue regardless
    } // OUTSIDE BUILT_IN


    int input_redirection = check_for_input_redirection(input); 

    if(input_redirection == 2)
      continue; // erorr

    int output_redirection = check_for_output_redirection(input); 

    if(output_redirection == 2)
      continue; // error 

    int background = check_for_background(input); 
    int pipe_present = check_for_pipe(input);  

    // only need to parse once (non-background, pipe write and read is stored seperately), store in **parsed 
    // even if no redirection is found, still want to store input in parsed 
    parse(input); 

    // check to see if cmd is accessible
    int ace = path_check(); 

    if(ace == 1){
      fprintf(stderr, "%s \n", "-myshell: error, unix command or program not found\n**Check that /bin is part of path**");
      continue; 
    }


    int redirection; 

    if((output_redirection == 1 && input_redirection == 0) || (output_redirection == 3 && input_redirection == 0))
      redirection = 4; // signal BOTH are present, (> or >> WITH <) 
    else if(input_redirection == 0)
      redirection = 0; // input redirection
    else if(output_redirection == 1)
      redirection = 1; // output redirection
    else if(output_redirection == 3)
      redirection = 3; // append output redirection
    else if(output_redirection == 5 && input_redirection == 5){
      redirection = 5; // none 
    }

      int success;
      success  = run_external_command(input, redirection, background, pipe_present); 

      if(background == 0){
        // background supports mutiple programs to run in a single input 
        // parse and exec, parse and exec ... until empty 
        parse(input); 

        while(parsed[0] != NULL){
          success = run_external_command(input, redirection, background, pipe_present); 
          parse(input); 
        }
    } 

    if(success == 1 && custom_msg == 0) // ERROR, print standard msg, if custom not already printed 
       write(STDERR_FILENO, error_message, strlen(error_message));

    
  } // repeat loop 

  return 0; 
}


char** get_user_input(){

  char *line = NULL;
  size_t len = 0; 
  ssize_t read = -1; 

  char del[] = " \n\t";  
  char **tokens = NULL; 
  tokens = (char **)malloc(sizeof(char*) * 100);
  int i = 0; 

  input_argc = 0; 

  read = getline(&line, &len, stdin); 

  if(read == -1)
    return NULL; 

  tokens[i] = strtok(line, del); 

  if(line[0] == '\n')
    return NULL; 
    
     
  while(tokens[i] != NULL){
    //printf("\n%s\n", tokens[i]); 
    ++input_argc; 
    ++i; 

    // FIX THIS 

    // if((isspace(tokens[i]) != 0)){  
    // // if the token contains an "empty" string (\n, \t, " "), decrement the count of i to replace that string 
    // // every string added to tokens[i] will be a nonspace 
    //   --i; 
    // }
   

    tokens[i] = strtok(NULL, del);  
  }
 
  //tokens[i] = '\0';  // added null terminator, just for reassurance 
  tokens[i] = NULL; 

  int count = 0; 
  
  char **print = tokens; 
  while(*print){
   //printf("%s\n", *(print)); 
    ++count; 
    ++print; 
  }

  input_argc = count; 
  //printf("%d\n", input_argc); 

  return tokens; 
}

char ** read_batch_file(FILE *file_ptr){
  // get input from batch file 
  // READ a line, execute that cmd, read next line, execute that cmd, ...  

  char *line = NULL;
  size_t len = 0; 
  ssize_t read = -1; 

  //char **input = NULL; 
  char del[] = " \n\t";  
  char **tokens = (char **)malloc(sizeof(char*) * 50);

  read = getline(&line, &len, file_ptr);

  if(read == -1){
    // EOF or error, NULL with signal reset loop 
    return NULL; 
  }

  int i = 0; 

  tokens[i] = strtok(line, del); 

  while(tokens[i] != NULL){
    //printf("\n%s\n", tokens[i]); 
    ++input_argc; 
    ++i; 

    // if(isspace(tokens[i])){ 
    // // if the token contains an "empty" string (\n, \t, " "), decrement the count of i to replace that string 
    // // every string added to tokens[i] will be a nonspace
    //   --i; 
    // }
    
    tokens[i] = strtok(NULL, del); 
  }

  //tokens[i] = '\0';  // added null terminator, just for reassurance 
  tokens[i] = NULL; 

  return tokens;
  //return input; 
}

int quick_error_check(char **input){
  // Quickly iterate through the array, input[0] and input[n-1] can't have any redirection, control, or pipe symbol. 
  // Global variable input_argc should have the count. 
  
  char *cant_start[5] = {"|", ">", "<", ">>", "&"};
  char *cant_end[4] = {"|", ">", "<", ">>"}; // can end with "&"

  for(int i = 0; i<5; ++i){

    if(strcmp(input[0], cant_start[i]) == 0){
      // leading string 
      fprintf(stderr, "%s %s\n", "-myshell: syntax error near unexpected token: ", cant_start[i]); 
      return 1; 
    }

    if(strcmp(input[input_argc-1], cant_end[i]) == 0){
      // trailing string 
      fprintf(stderr, "%s %s\n", "-myshell: syntax error near unexpected token: ", cant_end[i]); 
      return 1; 
    }
  }
  return 0; 
}

int is_built_in_command(char **input){
  // filter through the input indexes for a match 

  for(int j = 0; j<(input_argc-1); ++j){
    // don't have direct implementation of piping for a built-in,
    // just testing, treat as external
    // echo hello | wc -c 
    if(input[j] == NULL)
      continue; 

    if(strcmp(input[j], "|") == 0){
      return 1; 
    }
  }


  for(int i = 0; i<(sizeof(commands)/sizeof(commands[0])); ++i){
    // loop through my_commands, if input[0] equals a command, return 1 
    // Note: errors will be checked inside the respective function, so if the user inputs "help clear", too many args will be found inside the 
    // the help function. 
    if(strcmp(input[0], commands[i]) == 0)
      return 0; 
  }

  return 1; // not built_in
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
    // print the current directory (I already have it displayed)

    printf("%s\n", getcwd(buffer, sizeof(buffer)));
    return 0; // success 
  }

  int retval = 0;  


  retval = chdir(input[1]); 
  //printf("%s\n", getcwd(buffer, sizeof(buffer)));


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

  int output_redirection = check_for_output_redirection(input); // store output file if redirection is present 

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

  if((input_argc > 2) && (output_redirection != 1) && (output_redirection != 3)){ // no redirection present, too many args 
    fprintf(stderr, "%s \n", "-myshell: error, too many args");
    return 1; 
  }


  if(input_argc == 1) // will print contents of the current dir 
    name = getcwd(name, 150); 

  else if(input_argc > 1) // user passed argument
    name = input[1]; 
   

  DIR *directory = NULL;
	struct dirent *directory_entry = NULL; 
  directory = opendir(name); 

  if(directory == NULL){
    fprintf(stderr, "%s \n", "-myshell: error, <directory> not found");
    return 1; 
  }

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

  printf("\n"); 

  return 0; // success 
}

int environ(char **input){
  // list env strings, supports output redirection 

  int output_redirection = check_for_output_redirection(input); 
  
  if((input_argc > 1) && output_redirection != 1 && output_redirection != 3){
    fprintf(stderr, "%s \n", "-myshell: error, this command environ takes no args");
    return 1; 
  }

  if(output_redirection == 2) // error warning already printed 
    return 1; 

  char *envar1, *envar2, *envar3, *envar4 = NULL; 
  // grab the most important strings for users 
  envar1 = getenv("USER");
  envar2 = getenv("PWD");
  envar3 = getenv("HOME");
  envar4 = getenv("PATH"); 
   

  if(output_redirection == 5) // print to stdout 
    printf("USER=%s\nPWD=%s\nHOME=%s\nPATH=%s\n", envar1, envar2, envar3, envar4); 

  else{
    // output redirection
    FILE *fptr = NULL;  

    if(output_redirection == 1)
      fptr = fopen(output_file, "w"); 

    if(output_redirection == 3)
      fptr = fopen(output_file, "a"); 

    if(fptr == NULL){
      //fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
      return 1; 
    }

    fprintf(fptr, "%s%s\n%s%s\n%s%s\n%s%s\n", "USER=", envar1, "PWD=", envar2, "HOME=", envar3, "PATH=", envar4); // write to file regardless if > or >> 
    fclose(fptr); 
  }

  return 0; // success 
}

int echo(char **input){
  // echo <comment> 
  // support output redirection 

  int output_redirection = check_for_output_redirection(input); 

  if(output_redirection == 5){
    // print to stdout 

    for(int i = 1; i<(input_argc); ++i)
      printf("%s ", input[i]); 

  puts(""); 
  }

  else{
    // output redirection 

    FILE *fptr = NULL;
    //char *symbol = NULL; // signal for for loop to end 

    if(output_redirection == 1){
      fptr = fopen(output_file, "w"); 
      //symbol = ">"; 
    }

    if(output_redirection == 3){
      fptr = fopen(output_file, "a");  
      //symbol = ">>"; 
    }

    if(fptr == NULL){
      fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
      return 1; 
    }

    //for(int i = 1; strcmp(input[i], symbol) != 0; ++i) // print <comments> until we encouter the redirection symbol 
    for(int i = 1; input[i] != NULL; ++i){
      // check_for_output_redirection() replaces the symbol with a NULL
      // print <comments>, starting after cmd, until we encounter that null 
      fprintf(fptr, "%s ", input[i]); 
    }
  
    fprintf(fptr, "%s", "\n"); 
    fclose(fptr); 
  }

  return 0; // success 
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

  int output_redirection = check_for_output_redirection(input); 

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
      
    fclose(fptr2); 
  }

  fclose(fptr); 

  return 0; // success 
}

int pause_(char **input){
  // "pause" shell, until user hits enter ('\n')

  int retval; 

  while((retval = getchar()) != '\n')
	  continue; 

  return 0; 
}

int quit(char **input){
  // already freed malloc vars 
  // exit the shell 
  // doesn't really need a return value 

  exit(0); 

  return 0; 
}

int path(char **input){
  // path_check() will base its searching of whats added to the path here 
  // /bin already added when the program is first run 
  // OVERWRITE EACH CALL 
 

  // get current size of path
  char **counter = shell_path;
  int count = 0;  

  while(*counter != NULL){
    ++count;
    ++counter; 
  }

  // reset PATH to overwrite   
  for(int i = 0; i<count; ++i){
    shell_path[i] = NULL;  
  }


  if(input_argc == 1){
    // leave PATH empty 
    puts("WARNING, you set the path to empty, to undo call path /bin"); 
    return 0; 
  }
    
  int j = 0; 

  for(int i = 1; i<input_argc; ++i){
    // OVERWRITE add strings to path 
    shell_path[j] = input[i]; 
    //printf("%s", shell_path[j]); 
    ++j; 
  }

  printf("%s", "Shell Path Now ... \n");
  print_this(shell_path);  

  return 0; 
}

int check_for_input_redirection(char **input){

  for(int i=1; i<(input_argc-1); ++i){
  // iterate through inputs/ argc, start at [1] b/c already checked for invalid symybols that lead 

    if(input[i] == NULL)
      continue; // dont want to pass null to strcmp, iterate to next string 

    if(strcmp(input[i], "<") == 0){
      input_file = input[i+1]; 
      //printf("inside redirection, new in: %s\n", input_file); 

      int invalid = check_for_invalid_file(input_file); 
      if(invalid == 1)
        return 2; // erorr warning already printed return to to run_external_command, which will return you back to the main loop to start over


      input[i] = NULL; 
      input[i+1] = NULL; 

      // b/c myshell supports both redirection, dont want to parse yet, we will parse only before execution  

      return 0; // input redirection present 
    }

  } 
  return 5; // none 
}

int check_for_output_redirection(char **input){
  // identical to check_for_input_redirection(char **input)
  // two for loops, once for ">", other for ">>", myshell supports only 1 instance of output redirection a line 

    for(int i = 1; i<(input_argc-1); ++i){

      if(input[i] == NULL)
        continue; // dont want to pass null to strcmp, iterate to next string 

      if(strcmp(input[i], ">") == 0){
      output_file = input[i+1]; 
      //printf("inside redirection, new out: %s\n", output_file); 
      
      int invalid = check_for_invalid_file(output_file); 

      if(invalid == 1)
        return 2; // error 

      input[i+1] = NULL;
      input[i] = NULL; 

    return 1; // output redirection present
    }

  }

  for(int i = 1; i<(input_argc-1); ++i){

    if(input[i] == NULL)
      continue; 

    if(strcmp(input[i], ">>") == 0){
    output_file = input[i+1]; 
      
    int invalid = check_for_invalid_file(output_file); 

    if(invalid == 1)
      return 2;

    input[i+1] = NULL;
    input[i] = NULL; 

    append = 1; // flag for open() to O_APPEND
         

    return 3; // append output redirection present 
    }

  }

  return 5; // at this point, both for loops didnt return a value, this indicates NO output redirection is present 
}

int check_for_invalid_file(char *file){
  // input or output file string is ref. in char *file, less redundancy 

  if(file == NULL)
    return 0;   
  
  if((strcmp(file, ">") == 0 || strcmp(file, "<") == 0) || strcmp(file, ">>") == 0 || 
    strcmp(file, "|") == 0 || strcmp(file, "&") == 0){

      fprintf(stderr, "%s %s\n", "-myshell: error, unexecpted token: ", file);
      return 1; 
      }
  
  return 0; 
}

void parse(char **input){
  // parse cmd and args from input (a line of input could have mutiple processes (&), so you can parse a single input mutiple times)
  // don't know how many args (echo cmd you could have a long comment), use malloc 

  // ls -la & echo hello & 
  // parsed = {"ls", "-la", "null"}; 
  // run that 
  // parsed = {"echo", "hello", "null"}; 
  // run that 

  free(parsed); // reset 
  parsed = (char **)malloc(sizeof(char*) * 50);
  
  int i; 
  int j = 0; 

  for(i = start_here; input[i] != NULL; ++i){
    //stop when null is reached in input, for ex. if there is a & symbol it has been replaced with a NULL,
    // and other possible input could be to the right of that, start_here will track where to begin (+1 to start index to right of the null) 

    parsed[j] = input[i]; 
    ++j; 
  }

  parsed[j] = NULL; 

  start_here = i+1;  // ingore the null for the next parse, otherwise this gets reset to 0 at reset of loop 

  //print_this(parsed); 
}

void print_this(char **print_this){
  // for testing 

  char **print = print_this; 

  while(*print){
    printf("%s\n", *(print)); 
    ++print; 
  }

}

int check_for_pipe(char **input){
  // myshell will support piping between 2 processes
  // we need to execute two sets of cmds, will make use of *read_side[] and *write_side[], making execution easier 

  for(int i=1; i<input_argc; ++i){

    if(input[i] == NULL)
      continue; 

    if(strcmp(input[i], "|") == 0){
      // already error checked for incorrect leading and trailing symbols 
      // pipe_index is global, as we need to store the input before (write) and than after the pipe symbol (read)
      pipe_index = i; 

      input[i] = NULL; 
 

     // SLIGHTY DIFFERENT PARSER, IT WORKS! 
      int j, k; 

      for(j=0; j<=pipe_index; ++j){
        // L side 
        // start at 0, store strings until you reach the pipe symbol 

        write_side[j] = input[j];  
      }

      write_side[j] = NULL; 


      int m = 0; 
      for(k = pipe_index; k<input_argc; ++k){
        // R side 
        // start at 0, store strings until you reach the next null (end of input)

        read_side[m] = input[pipe_index+m+1];
        ++m; 
        }

      read_side[m] = NULL; 

      return 0; // pipe present, stored L and R side to exec seperately 
    }
  }

  return 1; // no pipe present 
}

int check_for_background(char **input){
  // note: can have mutiple & instances in a single line 

  int background = -1; 

    for(int i = 1; i<input_argc; ++i){
      // start at 1, b/c already checked input[0] for invalid input

      if(input[i] == NULL)
        continue; 

      if(strcmp(input[i], "&") == 0){
        // found &, shift input to remove "&" (dont want to shift here), return 1
        input[i] = NULL; 
        background = 1; 
      } 

    }
  
  if(background == -1)
    return 1; // no control & operator 

  return 0; // & present 
}

int run_external_command(char **input, int redirection, int background, int pipe_present){
  //void run_external_command(char **input, int redirection, char *in, char *out){
  // user needs to add ./ to specify location of executable ex. ./program 

  int file_des, file_des_two; // for use with both redirections 

  int pipe_found = pipe_present; 
 
  if(pipe_found == 0){
    int success = run_external_cmd_pipe(pipe_found); // make use of *write_side[] and *read_side[] 

    if(success == 1)
      return 1; // error 

    wait(NULL); 
    return 0; // success, both sides pipe exec properly 
  }

  int pid = fork(); 
  int execute; 

  if(pid == -1){
    return 1; 
  }

  if(pid == 0){ 
 
    // ============================ REDIRECTION ================================================= 

    if(redirection == 0){
      // input redirection < 

      file_des = open(input_file, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO); 
      //dup2(file_des, 0); 

      // newfd is stdin, for which dup2 will make a copy of. input is redirected to the "input_file"
      if(dup2(file_des, 0) == -1){
        close(file_des);
        return 1; 
      }  

    }

    else if(redirection == 1){
      // output redirection > 

      file_des = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

     // dup2(file_des, 1); 

      // newfd is stdout, for which dup2 will make a copy of. output is redirected to the "output_file"
      if(dup2(file_des, 1) == -1){
        close(file_des);
        return 1; 
      }   
    }

    else if(redirection == 2){
      custom_msg = 1; 
      return 1;  //error message already printed, when you return already at end of the loop body 
    }

    else if(redirection == 3){
      // output redirection APPEND >>  
      file_des = open(output_file, O_WRONLY | O_CREAT  | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); // dont truncate file 

      // newfd is stdout, for which dup2 will make a copy of. output is redirected to the "output_file"
      if(dup2(file_des, 1) == -1){
        close(file_des);
        return 1; 
      } 

    }

    else if(redirection == 4){
      // both, input_file and output_file have strings relating to the specified file 
      // < and (> or >>) 
      // (> or >>) and <  

      file_des = open(input_file, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
      if(dup2(file_des, 0) == -1){
        close(file_des);
        return 1; 
      }

      //dup2(file_des, 0); 

      close(file_des); 
 
      if(append == 1){
        // >>  
        file_des_two = open(output_file, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); 
      }
     else{
       // > 
      file_des_two = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);  
      }

      //dup2(file_des_two, 1); 

      if(dup2(file_des, 1) == -1){
        close(file_des);
        return 1; 
      } 

      close(file_des_two); 
    }

    if(file_des == -1 || file_des_two == -1){
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      return 1; 
    }
 
    // close fd, and execute 
    close(file_des); 
    execute = execvp(parsed[0], parsed);

    if(execute == -1){
      //perror("execvp");
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      exit(EXIT_FAILURE); // otherwise child will keeps running even WITH erorr  
    }

  }
  
  else{
    // PARENT 
    waitpid(pid, NULL, 0); // wait for child to finish, dont want myshell> to appear before output 
  }

  return 0; // success 
}

int run_external_cmd_pipe(int pipe_present){
  // run_external_command() too cluttered 

  //print_this(write_side);
  //print_this(read_side); 

  int pipe_fd[2]; 

  if(pipe(pipe_fd) == -1){
    // pipe_present, create a pipe, if unsuccessful print error 

    fprintf(stderr, "%s \n", "-myshell: error, could not create pipe");
    custom_msg = 1; 
    return 1; 
  } 

  int pid = fork(); // 1st fork()
  int execute; 

  if(pid == -1)
    return 1; 

  if(pid == 0){
    // close read side of pipe
    // stdout replaced with write side of pipe
    // close write side of pipe 
    // execute 

    close(pipe_fd[0]); 
    dup2(pipe_fd[1], 1); 
    close(pipe_fd[1]); 
    execute = execvp(write_side[0], write_side); 

    if(execute == -1){
      //perror("execvp");
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      exit(EXIT_FAILURE); // otherwise child will keeps running even WITH erorr  
    }
  }


  int pid2 = fork(); // fork again, write_side won't return, exec the read_side to complete pipe transaction 

  if(pid == -1){
    return 1; 
  }

  if(pid2 == 0){
    // close write side of pipe
    // stdout replaced with read side of pipe
    // close read side of pipe 
    // execute 
      
    close(pipe_fd[1]); 
    dup2(pipe_fd[0], 0); 
    close(pipe_fd[0]);  
    execute = execvp(read_side[0], read_side);

    if(execute == -1){
      //perror("execvp");
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      exit(EXIT_FAILURE); // otherwise child will keeps running even WITH erorr  
    }
  }

  else{ 
    waitpid(-1, NULL, 0); // wait for both  
    // double check, close 
    close(pipe_fd[0]);
    close(pipe_fd[1]); 
  }

  return 0; // success 
}

int path_check(){
  // PATH starts with /bin  
  // if enter cmd ls
  // need to concat /bin/ with ls --> /bin/ls to use access()
  // if user program, not in /bin, different loop use access() with ./program 
  // mutiple ways to get confirmation, test cmd against all directories 


  if(shell_path[0] == NULL){
    // shell_path empty, can only run built-ns 
    return 1; 
  }

  int yes; 

  char pathname[150]; 
  pathname[0] = '\0'; 

  int path_size = 1; 

  // WILL FIX after I do path function, this is just testing 
  for(int i=0; i<path_size; ++i){
    strcpy(pathname, shell_path[i]); 
    strcat(pathname, "/");
    strcat(pathname, parsed[0]);

    int ace = access(pathname, X_OK); 

    if(ace == 0)
      yes = 1; 

    memset(pathname, 0, sizeof(pathname)); 
  }


  memset(pathname, 0, sizeof(pathname)); 
  // a program wont need / 
  strcpy(pathname, parsed[0]);
  int ace = access(pathname, X_OK);

  if(ace == 0)
    yes = 1;  
      
  //printf("pathname: %s\n", pathname); 

  if(yes == 1)
    return 0; // command is available 
  else
    return 1; // command not found 
  
}
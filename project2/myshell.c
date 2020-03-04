/*
Project 2
Andreas Gagas
myshell.c - Developing a Linux Shell
*/

// free at end 

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
// path 

void print_this(char **print_this); 
void shift(char **input_shift, int start); // remove uneeded symbol and file in input, if present, also adjust the input count 
void parse(char **input); // trying this parse instead of shifting, I couldnt get & to work properly w/ shift 
void run_external_command(char **input, int redirection); 
int check_for_redirection(char **input); // 0 for < || 1 for > || 2 for ERORR || 3 for >> || 4 for BOTH || 5 for NONE
int check_for_invalid_file(char *file); // ex. output redirection is present, input[i+1] is stored, check that input[i+1] is not another symbol, needs to be a file  
int check_for_pipe(char **input); // check for single pipe
void pipe_parser(char **input); 
int check_for_background(char **input); 
void run_test(int redirection);

int check_for_input_redirection(char **input); 
int check_for_output_redirection(char **input); 


// basic global var 
char *commands[] = {"cd", "clr", "dir", "environ", "echo", "help", "pause", "quit", "path"}; 
int input_argc; 
char error_message[30] = "An error has occured\n";  // **** change all error messages to this (maybe leave some specific ones)
int start_here; // index to start parser from input 

// for redirection  
char *input_file, *output_file; 
int append; // quick fix 

// for pipe 
char *write_side[4]; 
char  *read_side[4]; 
int pipe_index; 

char **parsed; 

int background; 

//int redirection; 


int main(int argc, char *argv[]){

  FILE *fptr = NULL;

  start_here = 0; 
  parsed = NULL;
  
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
    input_argc = 0;  
    input_file = NULL, output_file = NULL; 

    start_here = 0; 

    if(batch_present == 0){ 
      puts(""); // space between outputs 
      input = read_batch_file(fptr); // get a single line 
       //print_this(input); 

      if(input == NULL)
        exit(0); // reached EOF 
    }

    else{
      //interactive mode 

      printf("%s", "myshell> "); 
    
      input = get_user_input(); 
      //print_this(input); 
    }

  
   // SECOND LOOP, used for multiple commands (with use & op, or read input from batch file)
   // ?? ?? 
  // for(int i = 0; i<input_argc; ++i){

    // parse(input); // char *parsed[] will be present with cmd and its args 

      // when input is shifted to remove symbols and files, input_argc is adjusted properly 
      
      int quick_error = quick_error_check(input); // a return of 1 if the first or last input contained an invalid command, error msg already printed 

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

        else if(strcmp(input[0], "quit") == 0)
         successful = quit(input); 

    // path? 

     if(successful == 1) // built-in call not succesful, print standard msg 
       write(STDERR_FILENO, error_message, strlen(error_message)); 

      //shift(input, 0); 

      //free(input); 
      continue; // continue regardless
    }

    // func. does everything, checks for redirection, pipe, &, and will call a shift func to shift appropirately 
    // makes use of global var ref the input file, output file, etc ... 

    //int redirection = check_for_redirection(input);  

    int output_redirection = check_for_output_redirection(input); 

    if(output_redirection == 2)
      continue; 

    int input_redirection = check_for_input_redirection(input); 

    if(input_redirection == 2)
      continue; 

    int redirection; 

    if(output_redirection == 1 && input_redirection == 0)
      redirection = 4; // signal BOTH are present 
    else if(input_redirection == 0)
      redirection = 0; // input redirection
    else if(output_redirection == 1)
      redirection = 1; // output redirection
    else if(output_redirection == 3)
      redirection = 3; // append output redirection
    else if(output_redirection == 5 && input_redirection == 5)
      redirection = 5; // none 

    printf("%d\n", redirection); 

    // if output returns 0, and input returns 1, then both are present, and the output and input file currently store the specified files 

    //print_this(parsed); 

    //run_external_command(input, redirection); 
    //run_test(redirection); //<=- BIG ERROR HERE 

    //print_this(parsed); 
    // background = check_for_background(input); 

    // if(background == 0)
    //   parse(input); 
     
    // ================== ONLY LOOP MUTIPLE PROCESSES YOU IDIOT 
    // while(parsed[0] != NULL){
    //   //second loop, mainly used & / multiple commands args 
        
    //   // //run_external_command(input); 
    //   // if(background == 0){
    //   //   run_test();
    //   //   parse(input); // parse again 
    //   // } 

    //   // else{
    //   //   run_external_command(input); 
    //   // } 
      
    //   run_test(); 
    //  // run_external_command(input); 
    //  // print_this(parsed); 

    //   //run_test(); 
    //   parse(input); // if needed 
    //   //break; 
    // }

    break; 
    

//     start_here = 0; 
//     break; 
     // break; 

//      // parse(input); // parse if needed 

//     //shift(input, 0); // successful, remove input that was executed, there are mutiple commands present, will not overwrite, as null succeeds the command

//  // } // outside for loop 
 
//    // break; // for testing 

  }

  return 0; 
}


char** get_user_input(){

  char *line, *res = NULL;
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

  char *line, *res = NULL;
  size_t len = 0; 
  ssize_t read = -1; 

  char **input = NULL; 
  char del[] = " \n\t";  
  char **tokens = (char **)malloc(sizeof(char*) * 50);

  read = getline(&line, &len, file_ptr);

  if(read == -1){
    // EOF or error, exit loop 
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
  
  int first = 0;
  int last = input_argc; 
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
      //fprintf(stderr, "%s \n", "-myshell: error, could not open output file");
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

  return 0; // success 
}

int environ(char **input){
  // list env strings, supports output redirection 
  int output_redirection = check_for_redirection(input); 
  //printf("%s", outut_redirection); 
  
  if((input_argc > 1) && output_redirection != 1 && output_redirection != 3){
    //fprintf(stderr, "%s \n", "-myshell: error, environ takes no args");
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
      //fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
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
      //fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
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
        //fprintf(stderr, "%s \n", "-myshell: error, cannot open output file");
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

  exit(0); 

  return 0; 
}

int check_for_redirection(char **input){ 
  // wil check for input redirection, both output redirections, and both input and output redirections 
  // if present, the corresponding file / files (both redirections) will be saved, and the redirection symbol along 
  // the file will be "removed" (down shift to replace)
  /*
  return values: 
  0: < present 
  1: > present 
  2: ERROR
  3: >> 
  4: BOTH > and < 
  5: NONE 
  */
  // ex. will_print_names < names.txt > print_names_here
  int out_present, in_present, append_out_present = 0; 
  int i = 1; 
  for(int i=1; i<(input_argc-1); ++i){
   // while(1){
    
    // iterate through inputs/ argc, start at [1] b/c already checked for invalid symybols that lead 

    if(strcmp(input[i], "<") == 0){
      input_file = input[i+1]; 
      //printf("inside redirection, new in: %s\n", input_file); 

      int invalid = check_for_invalid_file(input_file); 
      if(invalid == 1)
        return 2; // erorr warning already printed return to to run_external_command, which will return you back to the main loop to start over

      //shift(input, i); // will shift all input to "remove" the < and input_file

      input[i] = NULL; 
      input[i+1] = NULL; 
      parse(input); 

      // I need reset i, in case there is also ">" present, 
      // set i to 0, will increment to 1, and is "reset"
      // will not interfer if only "<" was present, loop will exit 
      i = 0; // will start at 1 again 

      in_present = 1; 

      if(out_present == 1 || append_out_present == 1)
        return 4; // indicate BOTH redirections are present

       else{
         // iterate, could also be an output redirection 
        ++i; 
        continue; 
      }

    }

    else if(strcmp(input[i], ">") == 0){
      output_file = input[i+1]; 
      //printf("inside redirection, new out: %s\n", output_file); 
      
     int invalid = check_for_invalid_file(output_file); 

      if(invalid == 1)
        return 2;

      //shift(input, i); 
      input[i+1] = NULL;
      input[i] = NULL; 
      
      
      parse(input); 

     // print_this(parsed); 

      return 1; 
      //start_here = 0; 

      i = 0; 

      out_present = 1; 

      if(in_present == 1){
        return 4; // indicate BOTH redirections are present
      }

      else{
        ++i; 
        continue; 
      }
    }


    else if(strcmp(input[i], ">>") == 0){
      output_file = input[i+1]; 
      //printf("inside redirection, new out APPEND: %s\n", output_file); 

      int invalid = check_for_invalid_file(output_file); 
      if(invalid == 1)
        return 2;

      shift(input, i); 
      i = 0; 

      append_out_present = 1; 
      
      if(in_present == 1){

        append = 1; // for use in differentiating between > and >> when both redirections (> <)are implemented 
        return 4; // indicate both redirections are present
      }

      else{
        ++i; 
        continue; 
      }

    //   if(input_argc = 1)
    //     return 3; // only append output redirection present 
    }

    //++i; 
  }

    // At this point, if both redirections were present, the func returned, the remaining value of in_present, out_present, and 
    // append_out_present will return a value idicating only one type of redirection was found. Note: if error, already returned 

    if(in_present == 1){
      // only input redirection 
      return 0; 
    }

    else if(out_present == 1){
      // only output redirection 
      //printf("inside redirection, new out: %s\n", output_file); 
      return 1; 
    }

    else if(append_out_present == 1){
      // 
      return 3; 
    }

    else{
      // no redirection, still just to store into parsed 
      parse(input); 
      return 5;  
    }
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

      //shift(input, i); // will shift all input to "remove" the < and input_file

      input[i] = NULL; 
      input[i+1] = NULL; 
      parse(input); 

      return 0; // input 
    }

  } 
  return 5; // none 
}

int check_for_output_redirection(char **input){

    for(int i = 1; i<(input_argc-1); ++i){

      if(input[i] == NULL)
        continue; // dont want to pass null to strcmp, iterate to next string 

      if(strcmp(input[i], ">") == 0){
      output_file = input[i+1]; 
      //printf("inside redirection, new out: %s\n", output_file); 
      
      int invalid = check_for_invalid_file(output_file); 

      if(invalid == 1)
        return 2;

      //shift(input, i); 
      input[i+1] = NULL;
      input[i] = NULL; 
      
      
      parse(input); 

     // print_this(parsed); 

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

      //shift(input, i); 
      input[i+1] = NULL;
      input[i] = NULL; 
         
      parse(input); 

    return 3; // append output redirection present 
    }
  }

  return 5; // none 
}



int check_for_invalid_file(char *file){
  // input or output file string is ref. in char *file, less redundancy 
  
  if((strcmp(file, ">") == 0 || strcmp(file, "<") == 0) || strcmp(file, ">>") == 0 || 
    strcmp(file, "|") == 0 || strcmp(file, "&") == 0){

      //fprintf(stderr, "%s %s\n", "-myshell: error, unexecpted token: ", file);
      write(STDERR_FILENO, error_message, strlen(error_message)); 
      return 1; 
      }
  
  return 0; 
}

void parse(char **input){
  // parse cmd and args from input (a line of input could have mutiple processes (&), so need to parse)
  // don't know how many args (echo cmd you could have a long comment), use malloc 

  // echo hello & echo world & 
  // parsed = {"ls", "-la", "null"}; 
  // run that 
  // parsed = {"./greet", "Andreas", "null"}; 
  // run that 

  free(parsed); // reset 
  parsed = (char **)malloc(sizeof(char*) * 50);
  
  int i; 
  int j = 0; 

  for(i = start_here; input[i] != NULL; ++i){
    //stop when null is reached in input, for ex. if there is a & symbol it has been replaced with a NULL,
    //and other possible input could be to the right of that. 

    parsed[j] = input[i]; 
    ++j; 
  }

  parsed[j] = NULL; 

  start_here = i+1;  // ingore the null; 

  //print_this(parsed); 
}

void shift(char **input_shift, int start){
  // just simply shift down, which will "remove" the redirection symbol, and the indicated file, which is already saved in a ptr to a string 

  int i = start; 

  int j =0; 

  for(i = start; input_shift[i-1] != NULL; ++i){
  //for(int i = start_here; input_shift[i] != NULL; ++i){

    //input_shift[j] = input_shift[i+2];

    input_shift[i] = input_shift[i+2]; 

    /*  Explanation / diagram 
    before loop: ./greet > names.txt > test.txt null 
    ./greet > names.txt > test.txt
    ./greet > test.txt > test.txt
    ./greet > test.txt null test.txt 
    end loop same as: ./greet > test.txt null null 

    ex 2: ls > output.txt 
    ls null output.txt
    end loop

    shift successful
    */
  }
 
  input_shift[i] = NULL; // already NULl but whatever 

  // decrement the input args appropriately
  int new_count = 0; 
  
  char **print = input_shift; 
  while(*print){
   //printf("%s\n", *(print)); 
    ++new_count; 
    ++print; 
  }

  input_argc = new_count; 
}

void print_this(char **print_this){
  // for testing 

  char **print = print_this; 

  while(*print){
    printf("%s\n", *(print)); 
    ++print; 
  }

  puts("");
}

int check_for_pipe(char **input){
  // myshell will support piping between 2 processes
  // will make use of the global var: pipe_input, pipe_output 

  for(int i=1; i<input_argc; ++i){

    if(strcmp(input[i], "|") == 0){
      // already error checked for incorrect leading and trailing symbols 
     // pipe_input = input[i-1]; // to write from side of pipe 
      pipe_index = i; 

      input[i] = NULL; 

     // pipe_input = input[0]; // cmd name 
     // pipe_output = input[i+1]; // to read from side of the pipe 

     pipe_parser(input); // will store read_side and write_side 

     // TESTING THIS PARSER 
      int j, k; 

      for(j=0; j<=pipe_index; ++j){
        // L side 
        // start at 0, input[0] needs to be a cmd 
        write_side[j] = input[j]; 
        //printf("write: %s\n", write_side[j]); 
      }


      int m = 0; 
      for(k = pipe_index; k<input_argc; ++k){
        //for(k = pipe_index; read_side[k] != NULL; ++k){
        // L side 
        // start at 0, input[0] needs to be a cmd 
        read_side[m] = input[pipe_index+m+1];
       // printf("read: %s\n", read_side[m]); 
        ++m; 
        }

        // write_side = {cmd, optional arg, null}
        // read_side = {cmd, optional arg, null}

        //print_this(write_side); 
       //print_this(read_side); 



      // no longer the | string, dont need to shift 
     // input[i] = NULL; 

      // int in = check_for_invalid_file(pipe_input);
      // int out = check_for_invalid_file(pipe_output); 

      // if(in == 1 || out == 1){
      //   fprintf(stderr, "%s \n", "-myshell: error, | requires an input file and outputfile");
      //   return 2; 
      // }

      return 0; // pipe present, save input and output of pipe files 
    }
  }

  return 1; // no pipe present 
}

void pipe_parser(char **input){
  // use two global array of ptrs to strings to store read_side and write_side of pipe 


}

int check_for_background(char **input){
  // note: can have mutiple & instances in a single line 

  int background = -1; 

    for(int i = 1; i<input_argc; ++i){
      // start at 1, b/c already checked input[0] for invalid input

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

//void run_external_command(char **input, int in_redirection, int out_redirection){
  void run_external_command(char **input, int redirection){
  // user needs to add ./ to specify location of executable ex. ./program 

  int file_des, file_des_two; // for use with both redirections 

  //int redirection = check_for_redirection(input); 
 // int background = check_for_background(input); 

   //int pipe_found = check_for_pipe(input);  
  int pipe_found = 1; 
  
 // print_this(parsed); 

  int pipe_fd[2]; 

  if(pipe_found == 0){
    // pipe symbol found, create a pipe
    
    if(pipe(pipe_fd) == -1){
      fprintf(stderr, "%s \n", "-myshell: error, could not create pipe");
      return; 
    } 
  }

  int pid = fork(); 
  int status, execute; 

  if(pid == -1){
    write(STDERR_FILENO, error_message, strlen(error_message)); 
    return; 
  }

  else if(pid == 0){ 

    // ============================ REDIRECTION ================================================= 
    if(redirection == 0){
      // input redirection < 
      file_des = open(input_file, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO); 
      dup2(file_des, 0); // newfd is stdin, for which dup2 will make a copy of. input is redirected to the "input_file"
    }

    else if(redirection == 1){
      // output redirection > 
      

      file_des = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
      dup2(file_des, 1); // newfd is stdout, for which dup2 will make a copy of. output is redirected to the "output_file"
    }

    else if(redirection == 2)
      return; // error message already printed, when you return already at end of the loop body 

    else if(redirection == 3){
      // output redirection APPEND >> 

      file_des = open(output_file, O_WRONLY | O_CREAT  | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); // dont truncate file 
      dup2(file_des, 1); // newfd is stdout, for which dup2 will make a copy of. output is redirected to the "output_file"
    }

    else if(redirection == 4){
      // both, input_file and output_file have strings relating to the specified file 
      // < and (> or >>) 
      // (> or >>) and <  

      file_des = open(input_file, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO); 
      dup2(file_des, 0); 
      close(file_des); 

      if(append == 1){
        // >>  
        file_des_two = open(output_file, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); 
      }
      else{
        // > 
        file_des_two = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); 
      }

      dup2(file_des_two, 1); 
      close(file_des_two); 
    }
 

  // ============================ PIPING READ SIDE ================================================= 

    if(pipe_found == 0){
      // For child, pipe already created, replace stdout to this end of the pipe (read side) 

      dup2(pipe_fd[0], 0); 
      close(pipe_fd[1]); // close not in use end of pipe (write side)

      execute = execvp(read_side[0], read_side); 
    }


    // ================================= EXECUTE ===========================================================
    //execute = execvp(input[0], input); // execute command and option(if there,), next string is null, fds already modified if needed 
    
    execute = execvp(parsed[0], parsed);


    if(execute == -1){
      fprintf(stderr, "%s \n", "-myshell: error, command not found");
      return;  
    }

    close(file_des); 
  }

  else{
    // if & dont want shell to wait for command to finish, exe in the background 

    if(background == 0){
    // if & dont want shell to wait for command/ program to finish, exe in the background 
      execute = waitpid(pid, &status, 0); 
    }    

    if(pipe_found == 0){
      // parent process, this is the WRITE to side 

     //IGORE // pipe already created with present write side, replace stdout to this end of the pipe (read side) 

      //shift(input, 2); // remove previous command


    //   //dont want to use shift(), slighlty different format here
    //   // testing this out 
    //   int j = 1; 
    //   for(int i = 0; j<(pipe_index+1); ++i){
    //     // right now pipe_index is null, to the right is the second cmd for the read side of the pipe
    //     // shift to start of input
    //     // ex. cat names.txt (null) grep "Andreas" --> grep "Andreas" (null) (doesnt matter whats here) 
    //     input[i] = input[j+pipe_index]; 
    //     ++j; 
    //   }


    //  // print_this(input); 

      dup2(pipe_fd[1], 1); 
      close(pipe_fd[0]); // close not in use end of pipe (read side)

      execute = execvp(write_side[0], write_side); 
   }
   else{

    wait(NULL); // without this, will mess up when myshell> gets printed
   }

  }

}

void run_test(int redirection){

  int pid = fork(); 
  int status, execute; 
  int file_des; 

  if(redirection == 1){
  // output redirection >  
    

    file_des = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    dup2(file_des, 1); // newfd is stdout, for which dup2 will make a copy of. output is redirected to the "output_file"
  }

  else if(redirection == 0){
      file_des = open(input_file, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO); 
      dup2(file_des, 0); // newfd is stdin, for which dup2 will make a copy of. input is redirected to the "input_file"
  }

  if(pid == -1){
    write(STDERR_FILENO, error_message, strlen(error_message)); 
    return; 
  }

   else if(pid == 0){ 

     execvp(parsed[0], parsed); 

   }

   else{
     wait(NULL); 
   }



}









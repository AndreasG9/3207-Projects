/*
Project 0
Andreas Gagas

wgrep.c : search line by line of a file for specific search term (CASE SENSITIVE), if found print entire line (similiar to unix command grep)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h> // used type ssize_t 

int main(int argc, char *argv[]){

   if(argc == 1){
    // no command line arguments passed 

    puts("wgrep: searchterm [file ...]");
    exit(1);
  }

  else if(argc == 2){
    // no file specified, only search term passed 
    // read from stdin, store, and search for term from that location 
 
    char *line, *res = NULL;
    size_t len = 0; 
    ssize_t read = -1; 
  
    //printf("%s", "Type some stuff with or without specified search term: "); 

    while((read = getline(&line, &len, stdin)) != -1){
       res = strstr(line, argv[1]);

      if(res != NULL){
        // term found, print ENTIRE line

        printf("%s", line);
        free(line);
        line = NULL;
        len = 0; 
      }
    }

    // will keep reading from stdin if manually enter input (./wgrep searchterm), ctrl-c to stop 
  }


  for(int i=2; i<argc; ++i){
    // first specified file name located at argv[2]

    FILE *fp = NULL; // file ptr
    fp = fopen(argv[i], "r");

    if(fp == NULL){
      // can't open file, end running of program

      puts("wgrep: cannot open file");
      exit(1);
    }

    // use getline() to read ENTIRE line from specified file 
    char *line, *res = NULL; 
    size_t len = 0; 
    ssize_t read = -1; // will hold length of line (not including '\0'), or getline() returns -1 if EOF or failure to read 

    while((read = getline(&line, &len, fp)) != -1){
      res = strstr(line, argv[1]);

      if(res != NULL){
        // term found, print ENTIRE line

        printf("%s", line);
        free(line); // dellocate 
        line = NULL;
        len = 0; 
      }
    }

    int status = fclose(fp); // close file ptr

    if(status == EOF){
      puts("error: failed to close file");
    }

  }

  return 0; 
}

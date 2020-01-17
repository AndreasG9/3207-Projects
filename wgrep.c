/*
Project 0
Andreas Gagas

wgrep.c : search line by line of a file for specific search term (CASE SENSITIVE), if found print entire line (similiar to unix command grep)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#define BUFFER_SIZE 256 


int main(int argc, char *argv[]){

   if(argc == 1){
    // no command line arguments passed 

    puts("wgrep: searchterm[file...]");
    exit(1);
  }

  else if(argc == 2){
    // no file specified, only search term passed 
    // read from stdin, store, and search for term from that location 
 
    char term[BUFFER_SIZE]; 
  
    printf("%s", "Type some stuff with or without specified search term : "); 
    fgets(term, BUFFER_SIZE, stdin);

    char *res = NULL; 
    res = strstr(term, argv[1]); 

    if(res != NULL){
      // term found, print ENTIRE line

      puts(term); 
    }

    else{
      puts("wgrep: term not found");
    }

  }

  for(int i=2; i<argc; ++i){
    // first specified file name located at argv[2]

    FILE *fp = NULL; // file ptr
    fp = fopen(argv[i], "r");

    if(fp == NULL){
      // can't open file, end running of program

      puts("wrep: cannot open file");
      exit(1);
    }








  }


  return 0; 
}

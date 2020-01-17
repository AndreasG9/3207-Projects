/*
Project 0
Andreas Gagas

wcat.c : display/ print out contents of indicated file / files (similiar to unix command : cat )
*/

#include <stdio.h>
#include <stdlib.h>
#define BUFFER_SIZE 256 

int main(int argc, char *argv[]){
  // include command line argument, allow user to select specified file / files

  if(argc == 1){
    // no file specified on command line 
    
    return 0;
  }

  char line[BUFFER_SIZE]; // temp. string to store line output

  for(int i=1; i<argc; ++i){
    // i init to 1 as argv[0] is refers to ./wcat

    FILE *fp = NULL; // file ptr
    fp = fopen(argv[i], "r"); // open the associated file, in read only mode 

    if(fp == NULL){
      // can't open file, end running of program

      puts("wcat cannot open the file\n");
      exit(1);
    }

    else{

      while(fgets(line, BUFFER_SIZE, fp)){
        // print each line, until EOF

        printf("%s", line);
      }
    }

    int status = fclose(fp); // close file ptr

    if(status == EOF){
      puts("error: failed to close file");
    }

    puts(""); // new line 
 }

  return 0;
}

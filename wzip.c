/*
Project 0
Andreas Gagas

wzip.c: .. ADD 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#define BUFFER_SIZE 256 

int main(int argc, char *argv[]){
  
  // aaaaaaaaaabbbb --> 10a4b 

    if(argc == 1){
    // no file specified on command line 
    
    return 0;
  }

  for(int i=1; i<argc; ++i){

    FILE *fp = NULL; 
    fp = fopen(argv[i], "r");

    if(fp == NULL){
      
      puts("wzip: cannot open file"); 
      exit(1); 
    }

    unsigned int count = 1;
    int lastChar; // can cast to (char) to get its character 
    int currentChar = 0;

    // grab each char, if next char is equal, increment count 
    // otherwise use fwrite() to write the count and character
    // reset and repeat until currentChar returns EOF 

    lastChar = fgetc(fp); 
    
    while(1){ // fix control 

      currentChar = fgetc(fp); 

      if(currentChar == EOF){
        // display lastChar and its counts ADD 
        break; 
      }
      
      if(currentChar == lastChar)
        ++count; 
      
      else{
        // fwrite the count
        // fwrite the character
 
        //currentChar = lastChar;
        count = 0; 
      }

      lastChar = currentChar; 

    }


    // close file 
  }




  return 0; 
}
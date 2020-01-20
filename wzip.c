/*
Project 0
Andreas Gagas

wzip.c: .. "compress" the contents of the specified file using run-length encoding ... aaaaaaaaaabbbb --> 10a4b 
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
  
  if(argc == 1){
    // no file specified on command line 
    
    puts("wzip: file1[file2...]");
    exit(1);
  }

  for(int i=1; i<argc; ++i){

    FILE *fp = NULL; 
    fp = fopen(argv[i], "wb"); // FIX "wb" mode ?? 

    if(fp == NULL){
      
      puts("wzip: cannot open file"); 
      exit(1); 
    }

    unsigned int count = 1;
    int lastChar; // can cast to (char) to get its character 
    int currentChar; 

    // grab each char, if next char is equal, increment count 
    // otherwise use fwrite() to write the count and character to STDOUT
    // reset and repeat until currentChar returns EOF 

    lastChar = fgetc(fp); 
    
    while(1){  

      currentChar = fgetc(fp); 

      if(currentChar == EOF){
        // display count and lastChar for final chracter in the file

        int retvalCount = fwrite(&count, sizeof(int), 1, stdout); 
        int retvalChar = fwrite(&lastChar, sizeof(char), 1, stdout); 

        if(retvalCount != 1 || retvalChar != 1)
          puts("fwrite error");

        //printf("TEST: count: %d\n character: %c\n", count, (char)lastChar); // test in "r" mode 

        break; // exit loop 
      }
      
      if(currentChar == lastChar)
        ++count; 

      else{

        int retvalCount = fwrite(&count, sizeof(int), 1, stdout); // fwrite the count  
        int retvalChar = fwrite(&lastChar, sizeof(char), 1, stdout); // fwrite the character

        if(retvalCount != 1 || retvalChar != 1)
          puts("fwrite error");
 
        //printf("TEST: count: %d\n character: %c\n", count, (char)lastChar); // test in "r" mode

        count = 1; 
      }

      lastChar = currentChar; 

    }

    int status = fclose(fp); // close file ptr

    if(status == EOF){
      puts("error: failed to close file");
    }
  }

  return 0; 
}
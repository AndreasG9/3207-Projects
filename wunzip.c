/*
Project 0
Andreas Gagas

wununzip.c: ... inverse of wzip, use fread to read int and char values from .z file, display in an "uncompressed" format 
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

  if(argc == 1){
    // no file specified on command line 
    
    puts("wunzip: file1[file2...]");
    exit(1);
  }

  for(int i=1; i<argc; ++i){

    FILE *fp = NULL; 
    fp = fopen(argv[i], "rb");

    if(fp == NULL){
      
      puts("wunzip: cannot open file"); 
      exit(1); 
    }

    int count, retvalCount, retvalCharacter;  
    char character; 

    while(1){
      // fread to "read" the 4-byte integer count , followed by the 1-byte char character, 
      // print that character by corresponding count to stdout 
      // loop until fread returns 0 
        
      retvalCount = fread(&count, sizeof(int), 1, fp); 
      retvalCharacter = fread(&character, sizeof(char), 1, fp);  

      if(retvalCharacter == 0){
        // fread should return 0 when LAST char is read from file, exit loop 
        break; 
      }
      
      // // TESTING 
      // printf("retvalCount: %d \t retvalCharacter: %d\n", retvalCount, retvalCharacter);
      // printf("count: %d \t character: %c\n", count, character);
      printf("%d%c", count, character);

      if(retvalCount != 1 || retvalCharacter != 1){
        puts("wunzip: fread error");
        exit(1);
      }


      // loop print character to count 
      // ADD 

    }
    
    int status = fclose(fp); // close file ptr

    if(status == EOF){
      puts("error: failed to close file");
    }

  }

  return 0; 
}
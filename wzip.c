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
    
    puts("wzip: file1 [file2 ...]");
    exit(1);
  }

  int count = 1; 
  int lastChar, currentChar, retvalCount, retvalChar; 

  for(int i=1; i<argc; ++i){

    FILE *fp = NULL; 
    fp = fopen(argv[i], "rb+"); // testing 

    if(fp == NULL){
      
      puts("wzip: cannot open file"); 
      exit(1); 
    }

    // grab each char, if next char is equal, increment count 
    // otherwise use fwrite() to write the count and character to STDOUT
    // reset and repeat until currentChar returns EOF, write the count for last character if last file passed 

    if(i == 1){
      // only for first character of first file passed 
      lastChar = fgetc(fp);
    }

    do{

      currentChar = fgetc(fp);
 
      if(currentChar == lastChar)
         ++count; 

      else{
        if((i+1) < argc){
          // multiple files passed, dont want to print count for final character yet, cont. to next iteration in loop  
          break; 
        }

        retvalCount = fwrite(&count, sizeof(int), 1, stdout); 
        retvalChar = fwrite(&lastChar, sizeof(char), 1, stdout);
        //lastChar = currentChar;
        count = 1;

        if(retvalCount != 1 || retvalChar != 1)
          puts("wzip: fwrite error");  
     }

     lastChar = currentChar; 
      
    }while(currentChar != EOF);

    int status = fclose(fp); // close file ptr

    if(status == EOF){
      puts("error: failed to close file");
    }
  }

  return 0; 
}
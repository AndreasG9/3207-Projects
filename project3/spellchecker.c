/*
Project 3
Andreas Gagas
spellchecker.c - Network Spell Checker 
*/

#include <stdlib.h>
#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 

int check_dictionary(char *word); 

int main (int argc, char *argv[]){

  // testing, move to seperate func 
  char *word = argv[1]; 
  int res = check_dictionary(word); 

  if(res == 0)
    puts("FOUND");
  else
    puts("NO"); 

  return 0; 
}

int check_dictionary(char *word){
  // sorted dictionary in dictionary.txt 
  // implement binary search to find a specific word

  FILE *fptr = fopen("dictionary.txt", "r");

  if(fptr == NULL){
    puts("dictionary.txt not found"); 
    exit(0); 
  }

  // get count for array of strings size 
  int count = 0; 
  char buffer[100]; 

  while(fgets(buffer, 100, fptr) != NULL)
    ++count;


  // create array of ptrs to strings, reset file ptr 

  //char *d_array[count]; // FIX 
  //char *d_array[99171]; 
  char **d_array = malloc(sizeof(char*) * 99171); 
  rewind(fptr);
  int i = 0, j = 0; 

  char *line = NULL;
  size_t len = 0; 
  ssize_t read = -1; 
  char del[] = " \n\t";  
  //char **tokens = NULL; 

  while(read = getline(&line, &len, fptr) != -1){
    // fill the array 

    char *token = strtok(line, del); // solve issue, extra space being added

    d_array[i] = token;
    ++i; 
    line = NULL; // reset 

  }

  d_array[i] = NULL;  

  fclose(fptr); 

  //init indexes 
  int left, middle, right, target; 
  left = 0, 
  right = (count-1); 

  while(left <= right){

    // cont. half the "array", until find term, or -1 
    middle = (left + right) / 2;

    target = strcmp(word, d_array[middle]); 

    //if(target == 0){
    if(strcmp(word, d_array[middle]) == 0){
      // found string, return index or TRUE / 0  
      return 0; // TRUE 
    }

    else if(target < 0){
      // less, check "left side"
      right = middle - 1; 
    }

    else{
      // greater, check "right side"
      left = middle + 1; 
    }

  } //outside while 


  return 1; // FALSE  
}
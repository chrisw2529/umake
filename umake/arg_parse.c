#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>


#include "arg_parse.h"

/*state
 *char* line: line to be put into args
 *char** args: empty char**
 *int wordCounting is a boolean used to determine if the function should be counting the words or adding words to args
 *1 means is counting words, 0 means not counting but adding words to args
 *
 *is a state machine that puts the pointer of the first letter of a word
 *in string (char* line) and puts in into (char** args) using a state machine
 *also adds a null terminating character to the end of args
 *note: if it finds a '#' symbol it will replace the remaining characters with empty spaces
 */
int static state(char* line, char** args, int wordCounting){
  int currentState = 0;
  int argsCount = 0;
  for(int i = 0; line[i] != '\0';i++){
    switch (currentState) {
      case 0:
        if(line[i] == '#'){
          currentState = 2;
          line[i] = ' ';
          break;
        }
        else if(isspace(line[i])) {
          currentState = 0;
          break;
        }
        else{
          currentState = 1;
          if(wordCounting == 0){
            args[argsCount] = &(line[i]);
          }
          argsCount++;
        }
      break;
      case 1:
        if(line[i] == '#'){
          currentState = 2;
          line[i] = ' ';
          break;
        }
        else if(isspace(line[i])) {
          currentState = 0;
          if(wordCounting == 0){
            line[i] = '\0';
          }
          break;
        }
        else{
          currentState = 1;
        }
      break;
      case 2:
        line[i] = ' ';
    }
  }
  if(wordCounting == 0){
    args[argsCount] = NULL;
  }
  return argsCount;
}
/* Arg Parse
 * line The command line to parse.
 *
 */
 char** arg_parse(char* line, int *argcp){
   char* dup = strdup(line);
  char **emp = NULL;
  *argcp = state(dup, emp, 1);
  char** args = malloc((*argcp+1) * sizeof(char*));
  state(dup, args, 0);
  ///free(dup);
  return args;
}

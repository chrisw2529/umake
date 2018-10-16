/* CSCI 347 micro-make
 *
 * 09 AUG 2017, Aran Clauson
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "arg_parse.h"
#include "target.h"
/* CONSTANTS */


/* PROTOTYPES */

/* Process Line
 * line   The command line to execute.
 * This function interprets line as a command line.  It creates a new child
 * process to execute the line and waits for that process to complete.
 */
void processline(char* line);
int expand(char* orig, char* new, int newsize);

/* Arg Parse
 * line for the command line to parse.
 *
 *
 * Main entry point.
 * argc    A count of command-line arguments
 * argv    The command-line argument valus
 *
 * Micro-make (umake) reads from the uMakefile in the current working
 * directory.  The file is read one line at a time.  Lines with a leading tab
 * character ('\t') are interpreted as a command and passed to processline minus
 * the leading tab.
 */
int main(int argc, char* argv[]) {
  FILE* makefile = fopen("./uMakefile", "r");
  if(makefile == NULL){
    fprintf(stderr, "cannot open file, exiting.\n");
    exit(EXIT_FAILURE);
  }
  size_t  bufsize = 0;
  char*   line    = NULL;
  ssize_t linelen = getline(&line, &bufsize, makefile);

  target* tempTarg;
  while(-1 != linelen) {
    if(line[linelen-1]=='\n') {
      linelen -= 1;
      line[linelen] = '\0';
    }
    if(line[0] != '\t'){
      if(strchr(line, ':') != NULL){
        int argCnt;
        char* col = strchr(line, ':');
        *col = ' ';
        char** parLine = arg_parse(line, &argCnt);
        tempTarg = new_target(parLine[0]);
        int test = 0;
        while(parLine[test] != '\0'){
          test++;
        }
        for(int i = 1; i < argCnt; i++){
          add_dependencey_target(tempTarg, parLine[i]);
        }
      }
      if(strchr(line, '=') != NULL){
        int argCnt;
        char* eq = strchr(line, '=');
        *eq = ' ';
        char** parsedLine = arg_parse(strdup(line), &argCnt);
        if(argCnt <= 1){
          setenv(parsedLine[0], "", 1);
        }
        else{
          setenv(parsedLine[0], parsedLine[1], 1);
        }
        free(parsedLine);
      }
    }
    else if(line[0] == '\t'){
      add_rule_target(tempTarg, line);
    }
    linelen = getline(&line, &bufsize, makefile);

  }
  for(int i = 1; i < argc; i++){
    target* tgt = find_target(argv[i]);
    if(tgt != NULL){
      for_each_dependencey(find_target(argv[i]), processline, 0);
    }
  }
  free(line);
  return EXIT_SUCCESS;
}

/* Expand
 * orig    The input string that may contain variables to be expanded
 * new     An output buffer that will contain a copy of orig with all
 *         variables expanded
 * newsize The size of the buffer pointed to by new.
 * returns 1 upon success or 0 upon failure.
 *
 * Example: "Hello, ${PLACE}" will expand to "Hello, World" when the environment
 * variable PLACE="World".
 */
int expand(char* orig, char* new, int newsize){
  int origCount = 0;
  int newCount = 0;
  int saveStart = -1;
  int countForNew = 1; //boolean
  while(orig[origCount] != '\0'){
    if(orig[origCount] == '$' && orig[origCount+1] == '{'){
      origCount = origCount + 2;
      saveStart = origCount;
      countForNew = 0;
    }
    else if(orig[origCount] == '}'){
      int distance = origCount - saveStart;
      char* substr = malloc(sizeof(char) * 1024);
      strncpy(substr, orig+saveStart, distance);
      substr[distance] = '\0';
      char* env = getenv(substr);
      free(substr);
      if(env){
        if(newsize - (int)strlen(env) < 0){
          return 0;
        }
        countForNew = 1;
        int envCount = 0;
        while(env[envCount] != '\0'){
          new[newCount] = env[envCount];
          newsize--;
          newCount++;
          envCount++;
        }
      }
      origCount++;
    }
    else if(countForNew == 1){
      new[newCount] = orig[origCount];
      newsize--;
      if(newsize == 0){
        return 0;
      }
      origCount++;
      newCount++;
    }
    else{
      origCount++;
    }
  }
  new[newCount] = '\0';
  return 1;
}

/* IOChecker
* char** args   input string that is checked for IO flags
*
* checks args for any <, >, or >> flags and opens files with certin flags depending on symbol found.
* returns altered string
*/
char** IOChecker(char** args){
  int file;
  for(int i = 0; args[i] != '\0'; i++){
    if(strchr(args[i], '>') != NULL){
      char* path = args[i+1];
      char * next = strchr(args[i], '>');
      if(next[1] == '>'){
        file = open(path,  O_WRONLY | O_CREAT | O_APPEND, 0666);
      }
      else{
        file = open(path,  O_WRONLY | O_CREAT | O_TRUNC, 0666);
      }
      args[i] = NULL;
      dup2(file, 1);
      close(file);
    }
    else if(strchr(args[i], '<') != NULL){
      char* rediPath = args[i+1];
      file = open(rediPath, O_RDONLY, 0666);
      if (file < 0){
        printf("file %s does not exist, exiting\n", rediPath);
        exit(EXIT_FAILURE);
      }
      args[i] = NULL;

      dup2(file, 0);
      close(file);
    }
  }
  return args;
}
/* Process Line
 *
 */
void processline (char* line) {
  int argc;
  char* dup = strdup(line);
  char new[1024];
  int newsize = 1024;
  int passFail = expand(dup, new, newsize);
  if(passFail == 0){
    printf("error: expand Failed returning\n");
    free(dup);
    return;
  }
  char** arg = arg_parse(new, &argc);
  if(argc > 0){
    const pid_t cpid = fork();
    switch(cpid) {
    //error
    case -1: {
      perror("fork");
      break;
    }
    //child
    case 0: {
      char** execArgs = IOChecker(arg);
      execvp(execArgs[0], execArgs);
      perror("execvp");
      exit(EXIT_FAILURE);
      break;
    }
    //parent
    default: {
      int   status;
      const pid_t pid = wait(&status);
      if(-1 == pid) {
        perror("wait");
      }
      else if (pid != cpid) {
        fprintf(stderr, "wait: expected process %d, but waited for process %d",
                cpid, pid);
      }
      break;
    }
    }
  }
  free(arg);
  free(dup);
}

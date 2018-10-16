/* CSCI 347 umake target.c
* Chris White
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>



#include "arg_parse.h"
#include "target.h"

List globalList;
#define EMPTY NULL


struct StringList_st{
  char* name;
  StringList next;
};

struct target_st{
  char* name;
  List next;
  StringList dependencies;
  StringList rules;
};

typedef struct target_st node;
typedef struct StringList_st *StringList;

/*new_target (working)
*
*char* name: name of new target
*appends a new target to the end of the globalList
*/
target* new_target(char* name){
  List* tempList = &globalList;
  node* n = malloc(sizeof(node));
  if(n != NULL) {
    char* dup = strdup(name);
    n->name = dup;
    n->next = EMPTY;
    n->dependencies = EMPTY;
    n->rules = EMPTY;
    while(*tempList != EMPTY) {
      tempList = &((*tempList)->next);
    }
    *tempList = n;
  }
  else {
    errno  = ENOMEM;
  }
  return n;
}

/*find_target
*
*char* name: name of desired target
*finds desired target within globalList and returns it
*if it is not found it returns NULL
*/
target* find_target(char* name){
  List* tempList = &globalList;
  while(*tempList != EMPTY) {

    if(strcmp((*tempList)->name, name) == 0){
      return *tempList;
    }
    else{
      tempList = &((*tempList)->next);
    }
  }
  return NULL;
}

/* add dependency target
* tgt: target to add rules to
* dep: dependency to be added to the target
*
* appends a passed in dependency to the end to the target dependency linked list
*/
void add_dependencey_target(target* tgt, char* dep){
  StringList n = malloc(sizeof(struct StringList_st));
  char* dup = strdup(dep);
  n->name = dup;
  n->next = EMPTY;
  StringList* tempList = &((tgt)->dependencies);
  while(*tempList != EMPTY) {
    tempList = &((*tempList)->next);
  }
  *tempList = n;
}

/* add rule target
* tgt: target to add rules to
* rule: rule to be added to the target
*
* appends a passed in rule to the end to the target rule linked list
*/
void add_rule_target(target* tgt, char* rule){
  StringList n = malloc(sizeof(struct StringList_st));
  char* dup = strdup(rule);
  n->name = dup;
  n->next = EMPTY;
  StringList* tempList = &((tgt)->rules);
  while(*tempList != EMPTY) {
    tempList = &((*tempList)->next);
  }
  *tempList = n;
}
/* for each rule
* tgt: the target your executing the rules of
* (*action)(char*): the function to be executed
*
* for each rule in the passed in target executes a passed in function on it
*/
void for_each_rule(target* tgt, void (*action)(char*)){
  StringList tempRules = tgt->rules;
  while(tempRules != EMPTY){
    action(tempRules->name);
    tempRules = tempRules->next;
  }
}

/* for each dependency
* tgt: the target your executing the dependencies of
* (*action)(char*): the function to be executed
*
* for each dependency in the passed in target executes a passed in function on it
*/
void for_each_dependencey(target* tgt, void (*action)(char*), time_t newest){
  StringList tempDepen = tgt->dependencies;
  struct stat tgtFileTime;
  struct stat depFileTime;
  stat(tgt->name, &tgtFileTime);
  newest = tgtFileTime.st_mtime;
  while(tempDepen != EMPTY){
    //if there is a target in dep list
    if(find_target(tempDepen->name) != NULL){
      for_each_dependencey(find_target(tempDepen->name), *action, newest);
      stat((tempDepen->name), &depFileTime);
      if(depFileTime.st_mtime > newest){
        newest = depFileTime.st_mtime;
      }
      tempDepen = tempDepen->next;
    }
    //if no target in dep list
    else{
      stat((tempDepen->name), &depFileTime);
      if(depFileTime.st_mtime > newest){
        newest = depFileTime.st_mtime;
      }
      tempDepen = tempDepen->next;
    }
  }

  if(tgtFileTime.st_mtim.tv_sec < newest || tgtFileTime.st_mtim.tv_sec == 0){
    for_each_rule(tgt, *action);
    stat(tgt->name, &tgtFileTime);
    newest = tgtFileTime.st_mtime;
  }
  return;
}

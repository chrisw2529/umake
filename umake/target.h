#ifndef _TARGET_H_
#define _TARGET_H_

struct target_st;
typedef struct target_st target;

typedef struct target_st* List;

typedef void(*list_action)(char*);


struct StringList_st;
typedef struct StringList_st* StringList;


target* new_target(char* name);
target* find_target(char* name);
void add_dependencey_target(target* tgt, char* dep);
void add_rule_target(target* tgt, char* rule);
void for_each_rule(target* tgt, list_action action);
void for_each_dependencey(target* tgt, list_action action, time_t newest);
//void recursive_depend(target* tgt, void (*action)(char*));
void free_targets();//optional

#endif

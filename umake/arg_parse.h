#ifndef _arg_parse
#define _arg_parse


/*Arg Parse
 *line The line to be parsed
 *argcp (out) the number of arguments found
 *returns a null-terminated array of pointers into line.
 *
 *parses through line and returns a char** of the line seperated by spaces and/or tabs
 */
char** arg_parse(char* line, int *argc);

#endif

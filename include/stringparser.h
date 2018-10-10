#ifndef STRINGPARSER_H
#define STRINPARSER_H

#include <string.h>
int pipestring(char* line, char** argv);
void parsestring(char* myComm, char** myComm_split);
void redirstring(char* line);
void filestring(char *line, char *newline, char* infile, char* outfile, char* appfile);
#endif

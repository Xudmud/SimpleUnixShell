//functions for the shell project. Eventually should separate them into appropriate headers.
#include "myfuncs.h"
#include <stdio.h>
#include <stdlib.h>
char* getcomm()
{
	char* thecomm = malloc(1024);
	size_t maxlen = 1024;
	printf("gregshell>");
	getline(&thecomm, &maxlen, stdin);
	return(thecomm);
}
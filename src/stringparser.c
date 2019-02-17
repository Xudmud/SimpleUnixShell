#include <string.h>
#include <stdlib.h>
//#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include "stringparser.h"


//This version breaks the string into pipes.
int pipestring(char* line, char** argv)
{
    int num_comms = 0;
    for(unsigned int i = 0; line[i] != '\0'; ++i)
    {
        if(line[i] == ' ' && line[i+1] == '|')
        {
            //Condense the extra space.
            for(unsigned int j = i; line[j] != '\0'; ++j)
            {
                line[j] = line[j+1];
            }
            //line[i] should now be the pipe.
        }
        if(line[i] == '|')
        {
            //Remove whitespace after the pipe.
            ++i;
            for(unsigned int j = i; line[j] != '\0'; ++j)
            {
                line[j] = line[j+1];
            }
        }
    }
    while(*line != '\0')
    {
        if(*line == '|')
        {
            *line++ = '\0';
            ++num_comms;
        }
        *argv++ = line;
        while(*line != '|' && *line != '\0' && *line != '\n')
            line++;
    }
    ++num_comms;
    *argv = '\0';
    return(num_comms);
}

//This is a *basic* interpretation, but not the best method.
void parsestring(char *line, char **argv)
{
    //Get to the next non-whitespace...

    while (*line != '\0')
    {       /* if not the end of line ....... */
            while (*line == ' ' || *line == '\t' || *line == '\n')
                *line++ = '\0';     /* replace white spaces with 0    */
            *argv++ = line;          /* save the argument position     */
            while (*line != '\0' && *line != ' ' &&
                *line != '\t' && *line != '\n' && *line != '\\')
            line++;             /* skip the argument until ...    */
    }
    *argv = '\0';                 /* mark the end of argument list  */
}

//Removes whitespace around the <, >, and >> redirects, to give the parser an
//easier time and avoid "" strings.
void redirstring(char* line)
{
    int num_comms = 0;
    for(unsigned int i = 0; line[i] != '\0'; ++i)
    {
        if(line[i] == ' ' && (line[i+1] == '>' || line[i+1] == '<'))
        {
            //Condense the extra space.
            for(unsigned int j = i; line[j] != '\0'; ++j)
            {
                line[j] = line[j+1];
            }
            //line[i] should now be the redirect.
        }
        if(line[i] == '>')
        {
            //Remove whitespace after the redirect.
            ++i;
            if(line[i] == '>') //Keep the second > for append.
                ++i;
            for(unsigned int j = i; line[j] != '\0'; ++j)
            {
                line[j] = line[j+1];
            }
        }
        else if(line[i] == '<')
        {
            ++i;
            for(unsigned int j = i; line[j] != '\0'; ++j)
            {
                line[j] = line[j+1];
            }
        }
    }
}

void filestring(char *line, char *newline, char* infile, char* outfile, char* appfile)
{
    //
    unsigned int line_pos = 0;
    unsigned int newline_pos = 0;
    unsigned int infile_pos = 0;
    unsigned int outfile_pos = 0;
    unsigned int appfile_pos = 0;

    while(line[line_pos] != '\0')
    {
        //Read each character one at a time, assign to appropriate array.
        //First, check input character.
        if(line[line_pos] == '<')
        {
            ++line_pos;
            while(line[line_pos] != ' ' && line[line_pos] != '\0' && line[line_pos] != '>')
            {
                infile[infile_pos] = line[line_pos];
                ++infile_pos;
                ++line_pos;
            }
        }
        else if(line[line_pos] == '>')
        {
            ++line_pos;
            if(line[line_pos] == '>')
            {
                //Dealing with appended file.
                ++line_pos;
                while(line[line_pos] != ' ' && line[line_pos] != '\0' && line[line_pos] != '<')
                {
                    appfile[appfile_pos] = line[line_pos];
                    ++appfile_pos;
                    ++line_pos;
                }
            }
            else
            {
                //Dealing with overwrite file.
                while(line[line_pos] != ' ' && line[line_pos] != '\0' && line[line_pos] != '<')
                {
                    outfile[outfile_pos] = line[line_pos];
                    ++outfile_pos;
                    ++line_pos;
                }
            }
        }
        else
        {
            //Dealing with normal input.
            newline[newline_pos] = line[line_pos];
            ++newline_pos;
            ++line_pos;
        }

    }

}

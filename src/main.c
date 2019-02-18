#include "myfuncs.h"
#include "stringparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int pipeComm(char** args, int comm_pos, int num_comms, int filein);

int main(int argc, char* argv[])
{


    //If called with arguments, just fork the process, call execvp and exit.
    //Parent process or shell handles pipes/redirects.
    if(argc > 1)
	{
        pid_t pid;
        pid = fork();
        if(pid < 0)
        {
            fprintf(stderr,"Fork failed.");
            exit(127);
        }
        else if(pid == 0)
        {
            execvp(*++argv,argv);
            fprintf(stderr,"%s: Process not found.\n",*argv);
            exit(-1);
        }
        else
        {
            wait(NULL);
        }
        exit(0);
    }
    //Otherwise, drop into a normal shell.
    //Continue looping until "exit" or "logout" is given.
	else
	{
    	while(1)
    	{
            //Build arrays to handle the input, pipes, etc.
            //Ideally, these ought to be char* that gets freed at the end.
            int comm_pos = 0;
        	char comm[1024];
            char* comm_pipe[16];
            int num_comms = 0;
			//Print a command prompt
			printf("gregshell>");

        	//Call function for reading in line.
        	fgets(comm, 1024, stdin);

            if(*comm != '\n') //a '\n' means a null command.
            {
                //strip the last character
                comm[strlen(comm)-1] = '\0';
                num_comms = pipestring(comm, comm_pipe);
                //Exit needs to be a builtin.
                if(!strcmp(comm_pipe[0],"exit") || !strcmp(comm_pipe[0],"logout"))
                {
                    exit(0);
                }
                //Call the execute function, pass fds...?
                int commRes = pipeComm(comm_pipe, 0, num_comms, STDIN_FILENO);
                printf("%d|",commRes);
            }
        }
    }
}

//Source for recursive pipe implementation: https://gist.github.com/zed/7835043
int pipeComm(char** comm, int comm_pos, int num_comms, int filein)
{
    //Parse the command.
    char* comm_split[64];
    char* comm_redir = calloc(1024,sizeof(char));
    char* comm_tmp = calloc(1024,sizeof(char));
    char* comm_main = calloc(1024,sizeof(char));
    char* comm_infile = calloc(64,sizeof(char));
    char* comm_outfile = calloc(64,sizeof(char));
    char* comm_appfile = calloc(64,sizeof(char));

    strcpy(comm_redir,comm[comm_pos]);
    redirstring(comm_redir);
    filestring(comm_redir, comm_tmp, comm_infile, comm_outfile, comm_appfile);
    //buildcomms(comm[comm_pos],comm_main,comm_infile,comm_outfile);
    parsestring(comm_tmp,comm_split);

    //Check if the command is cd, and if it's the first command.
    //No sense in changing the active directory on a piped command.
    //TODO: Look into adding support for ~ as home.
    if(!strcmp(comm_split[0],"cd") && comm_pos == 0)
    {
        if(chdir(comm_split[1]) < 0)
        {
            fprintf(stderr,"%s: File or directory not found.\n",comm_split[1]);
            return(1);
        }
        else
        {
            return(0);
        }
    }
    else
    {
        //Get a pid
        pid_t pid;
        //Base case: Last command.
        if(comm[comm_pos+1] == NULL)
        {
            pid = fork();
            if(pid < 0)
        {
            fprintf(stderr,"Fork failed on process %s.\n",comm_split[0]);
        }
        else if(pid == 0)
        {
            //Use strlen to determine where stdin is coming from.
            if(strlen(comm_infile) != 0)
            {
                //Input file is providing stdin
                int infd = open(comm_infile,O_RDONLY);
                dup2(infd,0);
                close(infd);
            }

            else
            {
                //stdin or a pipe is providing stdin.
                dup2(filein,0);
                close(filein);
            }

            //Check if an output file will be created or overwritten.
            if(strlen(comm_outfile) != 0)
            {
                //Output file was passed. Open the file, create with standard
                //rw-rw-r-- permissions if it doesn't exist.
                //If it does exist, existing permissions are used.
                int outfd = open(comm_outfile,O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                dup2(outfd,1);
                close(outfd);
            }
            else if(strlen(comm_appfile) != 0)
            {
                //Append file was passed. Open the file, set append mode. create
                //with standard rw-rw-r-- permissions if it doesn't exist.
                //If the file does exist, existing permissions are used.
                int appfd = open(comm_appfile,O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                dup2(appfd,1);
                close(appfd);
            }

            //Otherwise, stdout goes to stdout.
            execvp(*comm_split,comm_split);
            fprintf(stderr,"%s: Command not found.\n",comm_split[0]);
            exit(-1);
        }
        else
        {
            int status;
            for(unsigned int i = 0; i < num_comms; ++i)
                waitpid(pid,&status,0);
            return(status);
        }
    }
    else //Not the last command, so work with it.
    {
        //Make a new pipeline.
        int fd[2];
        if(pipe(fd) == -1)
        {
            fprintf(stderr,"Pipe failed on process %s.\n",comm[comm_pos]);
            exit(127);
        }
        //Fork the process.
        pid = fork();
        {
            if(pid < 0)
            {
                fprintf(stderr,"Fork failed on process %s.\n",comm[comm_pos]);
                exit(comm_pos+1);
            }
            else if(pid == 0)
            {
                //Child process.

                if(strlen(comm_infile) != 0)
                {
                    close(fd[0]); //Unused, stdin is passed.
                    int infd = open(comm_infile,O_RDONLY);
                    dup2(infd,0);
                    close(infd);
                }

                else
                {
                    dup2(filein,0);
                    close(filein);
                }
                if(strlen(comm_outfile) != 0)
                {
                    //Close fd[1]?
                    close(fd[1]);
                    int outfd = open(comm_outfile,O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                    dup2(outfd,1);
                    close(outfd);
                }
                else if(strlen(comm_appfile) != 0)
                {
                    close(fd[1]);
                    int appfd = open(comm_appfile,O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
                    dup2(appfd,1);
                    close(appfd);
                }
                else
                {
                    dup2(fd[1],1);
                    close(fd[1]);
                }

                //Assuming output on all but the last process goes to the pipe.

                execvp(*comm_split,comm_split);
                fprintf(stderr,"%s: Command not found.\n",comm_split[0]);
            }
            else
            {
                int status;
                //DON'T WAIT HERE, if you do the program will deadlock on any
                //input larger than the pipe buffer.
                //waitpid(pid,&status,0);
                close(fd[1]);
                pipeComm(comm,comm_pos+1,num_comms,fd[0]);
            }
        }

    }
}
    //Free the memory allocated for strings.
    free(comm_redir);
    free(comm_tmp);
    free(comm_main);
    free(comm_infile);
    free(comm_outfile);
    free(comm_appfile);
}


/*TODO:
Look into catching signals. I want to be able to drop back into my shell instead
of the shell that I ran it from.

More error checking on pipes/forks/files. Right now the program relies on good
faith from the user and system that things will work properly.

Utilize buffers for rather large inputs, since the pipe only has so much space
it can work with.
*/

{\rtf1\ansi\ansicpg1252\cocoartf2820
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 /*--------------------------------------------------------------------------\
File: mon2.c\
\
Description: This program creates a process to run the program identified\
             on the command line. It will then start procmon in another\
             process to monitor the change in the state of the first process.\
             After 20 seconds, signals are sent to the processes to terminate\
             them.\
\
             Also a third process is created to run the program filter.  \
             A pipe is created between the procmon process and the filter\
             process so that the standard output from procmon is sent to\
             the standard input of the filter process.\
--------------------------------------------------------------------------*/\
#include <stdio.h>\
#include <stdlib.h>\
#include <sys/types.h>\
#include <signal.h>\
#include <unistd.h>\
#include <string.h>\
\
\
int main(int argc, char **argv)\
\{\
    char *program;\
    pid_t pid_prog, pid_procmon, pid_filter;\
    int fd[2]; // pipe\
    char pid_str[20];\
\
    if (argc != 2) \{\
        printf("Usage: mon2 fileName\\n where fileName is an executable file.\\n");\
        exit(-1);\
    \} else\
        program = argv[1];  /* This is the program to run and monitor */\
\
    /* First Step: Create the first process to run the program from the command line */\
    pid_prog = fork();\
    if (pid_prog == 0) \{\
        execl(program, program, NULL);\
        perror("execl for program");\
        exit(1);\
    \}\
\
    /* Second step: Create the pipe to be used for connecting procmon to filter */\
    pipe(fd);\
\
    /* Third step: Create the filter process - connect its stdin to pipe read end */\
    pid_filter = fork();\
    if (pid_filter == 0) \{\
        close(fd[1]); // close write end\
        dup2(fd[0], STDIN_FILENO); // connect pipe read end to stdin\
        close(fd[0]);\
\
        execl("filter", "filter", NULL);\
        perror("execl for filter");\
        exit(1);\
    \}\
\
    /* Fourth step: Create the procmon process - connect its stdout to pipe write end */\
    pid_procmon = fork();\
    if (pid_procmon == 0) \{\
        close(fd[0]); // close read end\
        dup2(fd[1], STDOUT_FILENO); // connect pipe write end to stdout\
        close(fd[1]);\
\
        sprintf(pid_str, "%d", pid_prog); // convert pid to string\
        execl("procmon", "procmon", pid_str, NULL);\
        perror("execl for procmon");\
        exit(1);\
    \}\
\
    // Parent: close both ends of the pipe\
    close(fd[0]);\
    close(fd[1]);\
\
    /* Fifth step: Let things run for 20 seconds */\
    sleep(20);\
\
    /* Last step: \
       1. Kill the process running the program\
       2. Sleep for 2 seconds \
       3. Kill the procmon and filter processes\
    */\
    kill(pid_prog, SIGKILL);\
    sleep(2);\
    kill(pid_procmon, SIGKILL);\
    kill(pid_filter, SIGKILL);\
\
    return 0;  /* All done */\
\}\
}
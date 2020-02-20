#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <linux/limits.h>

int main(int argc, char** argv){
    int firstPip[2];
    int firstChildStatus;
    int secondChildStatus;
    char* ls[3]={"ls","-l",NULL};
    char *tail[4]={"tail","-n","2",NULL};

    pipe(firstPip);
    printf("%s\n","(parent_process>forking...)");
    __pid_t firstChild_pid=fork();
    printf("(parent_process>created process with id: %d)\n",firstChild_pid);

    if(firstChild_pid==0){/*firstchild process*/
        printf("%s\n","(child1>redirecting stdout to the write end of the pipe...)");
        close(STDOUT_FILENO);
        dup2(firstPip[1],STDOUT_FILENO);
        close(firstPip[1]);
        printf("%s\n","(child1>going to execute cmd: ...)");
        execvp(ls[0],ls);
        _exit(0);
    }
    else{
        printf("%s\n","(parent_process>closing the write end of the pipe...)");
        close(firstPip[1]);
    }

    __pid_t secondChild_pid=fork();

    if(secondChild_pid==0){/*secondchild process*/
        printf("%s\n","(child2>redirecting stdin to the read end of the pipe...)");
        close(STDIN_FILENO);
        dup2(firstPip[0],STDIN_FILENO);
        close(firstPip[0]);
        printf("%s\n","(child2>going to execute the cmd: ...)");
        execvp(tail[0],tail);
        _exit(0);
    }
    else{
        printf("%s\n","(parent_process>closing the read end of the pipe...)");
        close(firstPip[0]);
    }
    printf("%s\n","(parent_process>waiting for child processes to terminate...)");
    waitpid(firstChild_pid,&firstChildStatus,0);
    waitpid(secondChild_pid,&secondChildStatus,0);

    printf("%s\n","(parent_process>exiting...)");
    return 0;
}


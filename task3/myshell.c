#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "LineParser.h"



typedef struct cmdNode{
    struct cmdLine *cmdata;
    struct cmdNode *prev;
    struct cmdNote *next;
}cmdNode;


typedef struct pair{
    char* name;
    char* value;
    struct pair* next;
    struct pair* prev;
}pair;

int listSize;
int pairsSize;

/*DECLARATIONS*/
void tildaCase(cmdLine* line);
void historyCase(cmdNode* head,cmdLine* cmd);
char replacing(char *replaceIn, char *replaceWhat, char *replaceWith);
cmdNode* newNode(cmdLine* myCmd); /*creates a new node*/
cmdNode* add_cmd(cmdNode *originalHead, cmdLine* newCmd);
cmdLine *copy_cmd_line(cmdLine *command);
void printHistory(cmdNode* head);
void makeHistory(char buffer[],cmdNode *node);
void free_linked_list(cmdNode* start);
cmdNode* HistoryDeleteCase(cmdNode *head, cmdLine *cmd);
void exclamationMarkCase(cmdNode* head, cmdLine** cmd);
pair* set_pair(char* name, char* value, pair* head);
pair* newPair(char* value, char* name,pair* prev);
void freePair(pair* toFree);
void free_pair_list(pair* start);
void freeCertainPair(pair* toFree);
pair* setCase(cmdLine* cmd,pair* head);
void printPair(pair* head, char* value);
void dollar$ignCase(cmdLine** cmd, pair* head);
void deleteCase(cmdLine* toDelete,pair* head);

int execute(cmdLine *pCmdline){
    if(pCmdline->next!=NULL){
        int firstPip[2];
        int firstChildStatus;
        int secondChildStatus;

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
            execvp(pCmdline->arguments[0],pCmdline->arguments);
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
            execvp(pCmdline->next->arguments[0],pCmdline->next->arguments);
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
    }
    else {
        pid_t pid_child;
        int status;
        pid_child = fork(); /*this is information to the parent- it's child id*/

        if (pid_child == 0) { /*meaning it is a child*/
            execvp(pCmdline->arguments[0], pCmdline->arguments);
            perror("execvp");/*if the code reached here, it means the execution did not work*/
            _exit(EXIT_FAILURE);
        } else { /*parent proccess*/
            if (pCmdline->blocking == 1)
                waitpid(-1, &status, 0);
            return status;
        }
    }
}

int main(int argc,char **argv){
    int shouldquit=-1;
    int firstIteration=1;
    listSize=1;
    cmdNode* head=NULL;
    pair* firstPair=NULL;
    cmdLine* command;

    while(shouldquit!=0){/*the main endless loop*/
        char mypath[PATH_MAX];
        getcwd(mypath,sizeof(mypath));
        printf("%s>",mypath); /*prints the pathin each loop , just like in a real shell*/
        char str[2048];
        fgets(str,2048,stdin);/*fill str from whats in stdin*/
        command=parseCmdLines(str); /*creating a new command using what was written on stdin*/

        if(firstIteration==1){
            head=newNode(command);
            tildaCase(command);
            exclamationMarkCase(head,&command);
            firstIteration++;

            if(strcmp(command->arguments[0],"set")==0) {
                firstPair = setCase(command, NULL);
            }
            else if(strcmp(command->arguments[0],"cd")==0){
                if(chdir(command->arguments[1])==-1){/* if directory transfer failed*/
                    _exit(EXIT_FAILURE);
                }
            }
            else if(strcmp(command->arguments[0],"history")==0){
                historyCase(head,command);
            }
            else if(shouldquit!=0){
                execute(command);
            }

        }
        else{
            head=add_cmd(head,command);
            exclamationMarkCase(head,&command);
            dollar$ignCase(&command,firstPair);
            shouldquit=(strcmp(command->arguments[0],"quit"));
            if(strcmp(command->arguments[0],"set")==0) {
                if (firstPair == NULL) {
                    firstPair = setCase(command,NULL);
                }
                else{
                    setCase(command,firstPair);
                }
            }
            else if(strcmp(command->arguments[0],"delete")==0){
                deleteCase(command,firstPair);
            }
            else if(strcmp(command->arguments[0],"cd")==0){
                head=add_cmd(head,command);
                if(chdir(command->arguments[1])==-1){
                    _exit(EXIT_FAILURE);
                }
            }
            else if(strcmp(command->arguments[0],"history")==0){
                historyCase(head,command);
            }
            else if(strcmp(command->arguments[0],"env")==0){
                printPair(firstPair,command->arguments[1]);
            }
            else if(shouldquit!=0){
                execute(command);
            }
        }
    }
    free_linked_list(head);
    free_pair_list(firstPair);
    return 0;
}

void tildaCase(cmdLine *line){
    char temp[1024];
    char *myhome = getenv("HOME"); /*home directory*/
    int i;

    for (i = 0; i < line->argCount; i++)
    {
        memset(temp, '\0', strlen(temp)); /*fills up temp with the '\0'. You'll see why in 'replacing' */
        /*char *current_argument = line->arguments[i];*/
        strcpy(temp, line->arguments[i]);/*copies the argument to temp*/

        if (replacing(temp, "~", myhome))
            replaceCmdArg(line, i, temp);
    }
}

char replacing(char *replaceIn, char *replaceWhat, char *replaceWith)
{
    char *accurrence = strstr(replaceIn, replaceWhat); /*to find the first accurrence on '~' in temp (stops when it reaches '\0'!)*/
    if (!accurrence)/*no accurrence*/
        return 0;

    char buffer[1024];
    memset(buffer, '\0', strlen(buffer));/*same reason as in 'tildaCase'*/

    if (replaceIn == accurrence) /* if 'replaceWhat' was found at the beggining of 'replaceIn' */
    {
        strcpy(buffer, replaceWith);
        strcat(buffer, accurrence + strlen(replaceWhat));/*adds 'accurrence' except for the 'replaceWhat' to the end of buffer*/
    }
    else /*there is an accurrence further in 'replaceIn'*/
    {
        strncpy(buffer, replaceIn, strlen(replaceIn) - strlen(accurrence));/*copies the string until the first accurrence*/
        strcat(buffer, replaceWith); /*adds 'replaceWith*/
        strcat(buffer, accurrence + strlen(replaceWhat));/*adds the rest of 'replaceIn' after 'replaceWhat'*/
    }

    memset(replaceIn, '\0', strlen(replaceIn));/*'empties' 'replaceIn'*/
    strcpy(replaceIn, buffer); /*copies the updated string to 'replaceIn'*/
    return 1;
}

cmdNode* newNode(cmdLine* myCmd){ /*creates a new node*/
    cmdNode *node= (cmdNode*) malloc(sizeof(cmdNode));
    node->cmdata=copy_cmd_line(myCmd);
    node->prev=NULL;
    node->next=NULL;
    return node;
}
pair* newPair(char* value, char* name,pair* prev){
    pair* node=(pair*) malloc(sizeof(pair));
    node->name=name;
    node->value=value;
    node->next=NULL;
    node->prev=prev;
    if(prev!=NULL)
        prev->next=node;
    return node;
}

cmdNode* add_cmd(cmdNode *originalHead, cmdLine* newCmd){
    cmdNode *newHead=newNode(newCmd);
    /*connecting the newHead:*/
    originalHead->prev=newHead;
    newHead->next=originalHead;
    listSize++;/*updating the global variable representing the length of the linkedlist*/
    return newHead;
}

pair* set_pair(char* name, char* value, pair* head){
    pair* curr=head;
    while(curr->next!=NULL){
        curr=curr->next;
    }
    curr->next=newPair(value,name,curr);
    pairsSize++;
    return head;
}

void freeNode(cmdNode* toFree){ /*frees a node*/
    cmdNode* next = toFree->next;
    freeCmdLines(toFree->cmdata);
    free(toFree);
}
void deleteCase(cmdLine* toDelete, pair* head){
    pair* curr=head;
    while(curr!=NULL&&strcmp(curr->name,toDelete->arguments[1])!=0){
        curr=curr->next;
    }
    if(curr==NULL){
        printf("%s\n","NO SUCH PAIR!");
    }
    else{
        freeCertainPair(curr);
    }
}
void freeCertainPair(pair* toFree){
    pair* tempNext;
    pair* tempPrev;
    if(toFree->next==NULL)
        tempNext=NULL;
    else
        tempNext=toFree->next;

    if(toFree->prev==NULL)
        tempPrev=NULL;
    else
        tempPrev=toFree->prev;

    if(tempNext==NULL&&tempPrev!=NULL)
        tempPrev->next=NULL;

    if(tempPrev==NULL&&tempNext!=NULL)
        tempNext->prev=NULL;

    if(tempPrev!=NULL&&tempPrev->next!=NULL&&tempNext!=NULL)
        tempPrev->next=tempNext;
    if(tempNext!=NULL&&tempNext->prev!=NULL&&tempPrev!=NULL)
        tempNext->prev=tempPrev;
    free(toFree);
}

void free_linked_list(cmdNode* start){
    cmdNode *curr=start;
    cmdNode* temp; /*the one which is going to be freed*/
    while(curr!=NULL){
        temp=curr;
        curr=curr->next;
        freeNode(temp);
    }
}
void free_pair_list(pair* start){
    pair* temp;
    pair* curr=start;
    while(curr!=NULL){
        temp=curr;
        curr=curr->next;
        free(temp);
    }
}


void historyCase(cmdNode* head,cmdLine* cmd){
    if ((cmd->argCount > 2) && (strcmp(cmd->arguments[0],"history") == 0) &&  (strcmp(cmd->arguments[1],"-d") == 0)){ /*meaning we are in the deleting case*/
        head= HistoryDeleteCase(head,cmd);
    }
    else if(strcmp(cmd->arguments[0],"history")==0)/*meaning it is indeed the history case*/
        printHistory(head);
}

cmdNode* HistoryDeleteCase(cmdNode *head, cmdLine *cmd) {
    long int whereToDelete=atoi(cmd->arguments[2]);
    cmdNode *tempPrev;
    cmdNode *tempNext;
    cmdNode *curr=head;
    int i=0;
    while(curr->next!=NULL){/*going all the way to the end of the list (which is actually where the first command is)*/
        curr=curr->next;
    }
    while(i<whereToDelete){/*going to the node we want to delete*/
        curr=curr->prev;
        i++;
    }
    tempNext=curr->next;/*connecting his right and left 'hands'*/
    tempPrev=curr->prev;
    tempPrev->next=tempNext;
    tempNext->prev=tempPrev;
    freeCmdLines(curr->cmdata);
    free(curr);
    listSize--;
    return tempPrev;
}

void printHistory(cmdNode* head){/*where we actually print in one by one*/
    char buffer[1024];
    int i;
    cmdNode*  curr=head;
    i=listSize-1;
    while(curr!=NULL&&i<10){ /*while there are still nodes to print+ we know there are at most 10*/
        makeHistory(buffer,curr);
        printf("%d. %s\n",i--,buffer);
        curr=curr->next;
    }
}
void printPair(pair* head, char* value){
    pair *curr=head;
    int i=0;
    int howMany=0;
    while(curr!=NULL){
        if(strcmp(curr->value, value)==0){
            printf("-%d. %s.\n",i,curr->name);
            howMany++;
        }
        curr=curr->next;
        i++;
    }
    if(howMany==0)
        printf("%s\n","VALUE NOT FOUND!");
}
void makeHistory(char buffer[],cmdNode *node){ /*fill the buffer*/
    memset(buffer, '\0',1024);/*filling it with \0*/

    int i;
    for(i=0;i<node->cmdata->argCount;i++){/*copies all of the command to the buffer*/
        strcat(buffer,node->cmdata->arguments[i]);
        strcat(buffer, " ");
    }
}
void exclamationMarkCase(cmdNode* head,cmdLine** cmd){
    if(strncmp((*cmd)->arguments[0],"!",1)==0){/*exclamation mark case*/
        int whereToRepeat;
        char buffer[10];
        strcpy(buffer,(*cmd)->arguments[0]+1);/*we dont know how long is the wanted number, so we copy everything to a buffer and then convert it i=to int */
        whereToRepeat=atoi(buffer);
        int i=0;
        cmdNode* curr=head;
        while(curr->next!=NULL){
            curr=curr->next;
        }
        while(i<whereToRepeat){
            curr=curr->prev;
            i++;
        }
        *cmd=copy_cmd_line(curr->cmdata);
    }
}

pair* setCase(cmdLine* cmd,pair* head){
    if(strcmp(cmd->arguments[0],"set")==0){
        char* name=cmd->arguments[1];
        char* value=cmd->arguments[2];
        pair *curr=head;
        if(curr==NULL){/*first iteration case*/
            curr=newPair(value,name,NULL);
            return curr;
        }
        else{/*there has been at least one 'set'*/
            while((strcmp(curr->name,name)!=0)&&curr->next!=NULL){/*while we didnt find an existing pair+havent reached the end of the list*/
                curr=curr->next;
            }
            if(strcmp(curr->name,name)==0){/* we reached an existing pair*/
                curr->value=value;
            }
            else{
                newPair(value,name,curr);
            }
            return head;
        }
    }
}

void dollar$ignCase(cmdLine** cmd, pair* head){
    char changeTo[1024];
    char currName[1024];
    memset(currName,'\0',1024);
    memset(changeTo,'\0',1024);
    int i;
    pair* curr=head;
    for(i=0;i<(*cmd)->argCount;i++) {
        if(strncmp((*cmd)->arguments[i],"$",1)==0) {/*meaning this one needs to be changed*/
            strcpy(currName,(*cmd)->arguments[i]+1);
            while (curr!= NULL&&(strcmp(curr->name,currName)!=0)){/*going forward until we meet a match or reach the end of the list*/
                curr=curr->next;
            }
            if(curr==NULL){/*invalid name*/
                printf("%s\n","INVALID NAME!!");
                return;
            }
            else{/*we found a matching pair!*/
                strcpy(changeTo,curr->value);/*copies the name we want to change to*/
                replaceCmdArg(*cmd, i, changeTo);
            }
        }
    }
}



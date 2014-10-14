#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>

struct node{
	char string[50];
	struct node *next;
};

void list_clear(struct node *list) {
    while (list != NULL) {
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

void list_append(const char *string, struct node **head) {
	//initialize the node to append
    struct node *new = malloc(sizeof(struct node));
	strcpy(new->string, string);
	new->next = NULL;
	
	//if the list is empty, replace its head with the new node
	if(*head==NULL){
		*head = new;
		return;
	}	
    struct node *temp = *head;
    //advance the pointer till we reach the end
    while(temp->next!=NULL){
    	temp = temp->next;
    }        
    temp->next = new; //append the name at the end
}

/*
 * 1. Separate input string s into tokens by delimeter
 * 2. Add the tokens to linked list
 * 3. Return the number of tokens added to the list
 *
 * Some parts adapted from lab02 tokenify.c
 */
int tokenify(const char *s, struct node** head, char* delim) {
	char *copy_s = strdup(s);
    char *reserve;
    char *token = strtok_r(copy_s,delim, &reserve);
    int num = 0;
	
    while(token!=NULL){
		char* temp = strdup(token);
		list_append(temp,head);	
		token = strtok_r(reserve,delim, &reserve);
		num++;
		free(temp);
    }
    free(copy_s);
    return num;
}

/*
 * Execute the input command in parallel mode
 * Input must be an array of pointers to char (or an array of strings)
 * The last element of the input array must be NULL
 *
 * Return -1 if the user wants to exit
 * Return 1 if the suer wants to switch to sequential mode
 * Otherwise return 0
 */
int par_execute(char *arg[]){
	if((strncmp(arg[0],"exit",4))==0){
		return -1;
	}	
	if((strncmp(arg[0],"mode",4))==0){
		if(arg[1]==NULL){
			printf("Current mode is: parallel\n");
			return 0;
		}else if((strncmp(arg[1],"sequential",8))==0){
			printf("Switch to sequential!\n");
			return 1;
		}else if((strncmp(arg[1],"s",1))==0 && strlen(arg[1])==1){
			printf("Switch to sequential!\n");
			return 1;
		}else if((strncmp(arg[1],"parallel",8))==0){
			printf("We are already in parallel mode!\n");
			return 0;
		}else if((strncmp(arg[1],"p",1))==0 && strlen(arg[1])==1){
			printf("We are already in parallel mode!\n");
			return 0;
		}				
	}
			
	pid_t child_pid = fork();
	if(child_pid == 0){
		if(execv(arg[0],arg)<0){
			fprintf(stderr, "execv failed\n");
			return 0;
		}
	}
	return 0;	
}

/*
 * Execute the input command in sequential mode
 * Input must be an array of pointers to char (or an array of strings)
 * The last element of the input array must be NULL
 *
 * Return -1 if the user wants to exit
 * Return 1 if the suer wants to switch to parallel mode
 * Otherwise return 0
 */
int seq_execute(char *arg[]){	
	if((strncmp(arg[0],"exit",4))==0){
		return -1;
	}
	
	if((strncmp(arg[0],"mode",4))==0){
		if(arg[1]==NULL){
			printf("Current mode is: sequential\n");
			return 0;
		}else if((strncmp(arg[1],"sequential",8))==0){
			printf("We are already in sequential mode!\n");
			return 0;
		}else if((strncmp(arg[1],"s",1))==0 && strlen(arg[1])==1){
			printf("We are already in sequential mode!\n");
			return 0;
		}else if((strncmp(arg[1],"parallel",8))==0){
			printf("Switch to parallel!\n");
			return 1;
		}else if((strncmp(arg[1],"p",1))==0 && strlen(arg[1])==1){
			printf("Switch to parallel!\n");
			return 1;
		}		
	}
		
	pid_t child_pid = fork();
	if(child_pid == 0){
		if(execv(arg[0],arg)<0){
			fprintf(stderr, "execv failed\n");
			return 0;
		}
	}else{
		int status;
		waitpid(child_pid, &status, 0);
	}	
	return 0;	
	
}

int main(int argc, char **argv) {
	bool mode = true; //true when sequential, false when parallel

	printf("prompt> ");
	fflush(stdout);		
	char buffer[1024];		
	while(fgets(buffer,1024,stdin)!=NULL){
		buffer[strlen(buffer)-1] = '\0';		
		
		//ignore everything after #		
		for(int i=0;i<strlen(buffer);i++){
			if(buffer[i]=='#'){
				buffer[i]='\0';
				break;
			}
		}
		
		//linked list of commands separated by semicolons
		struct node *commands = NULL;	
		int command_num = tokenify(buffer,&commands,";");
		
		//go through all commands
		struct node *temp = commands;
		while(temp!=NULL){		
		
			//linked list of tokens of a command separated by space
			struct node *command = NULL;
			int token_num = tokenify(temp->string,&command," \n\t");
			
			//array of pointers to strings			
			char **arg = malloc((token_num+1)*sizeof(char*));
			struct node *temp2 = command;
			int index = 0;
			
			//go through all tokens and store them in the array
			while(temp2!=NULL){
				arg[index++] = temp2->string;
				temp2 = temp2->next;
			}
			//set the last element of the array to null	
			arg[index] = NULL;		
			
			int result = 0;
			if(mode==true){
				result = seq_execute(arg);
			}else{
				result = par_execute(arg);
			}
			
			//user wants to exit
			if(result==-1){
				list_clear(command);
				list_clear(commands);
				free(arg);
				exit(0);
			}
			//user wants to switch mode
			else if(result==1){
				mode ^= true;
			}
			temp = temp->next;				
			list_clear(command);
			free(arg);
		}
		list_clear(commands);				
	}
    return 0;
}

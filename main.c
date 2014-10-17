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

/*
 * Linked list operations adapted from sample solution
 */
struct node{
	char string[50];
	struct node *next;
};

struct p_node{
    pid_t pid;
    struct p_node *next;
};

void list_clear(struct node *list) {
    while (list != NULL) {
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

void p_list_clear(struct p_node *list) {
    while (list != NULL) {
        struct p_node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

int p_list_delete(const pid_t pid, struct p_node **head) {
    if((*head)->pid == pid) {
        struct p_node *dead = *head;
        *head = (*head)->next;
        free(dead);
        return 1;
    }
    struct p_node *tmp = *head;
    while (tmp->next != NULL) {
        if (tmp->next->pid == pid) {
            struct p_node *dead = tmp->next;
            tmp->next = dead->next;
            free(dead);
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}

void list_append(const char *string, struct node **head) {
    struct node *new = malloc(sizeof(struct node));
	strcpy(new->string, string);
	new->next = NULL;

	if(*head==NULL){
		*head = new;
		return;
	}	
    struct node *temp = *head;
    while(temp->next!=NULL){
    	temp = temp->next;
    }        
    temp->next = new;
}

void p_list_append(const pid_t p, struct p_node **head) {
    struct p_node *new = malloc(sizeof(struct p_node));
    new->pid = p;
    new->next = NULL;
    
    if (*head == NULL) {
        *head = new;
        return;
    }
    struct p_node *tmp = *head;
    while (tmp->next != NULL) {
        tmp = tmp->next;
    }
    tmp->next = new;
}

void p_list_print(const struct p_node *list) {
    while (list != NULL) {
        printf("%d\n", list->pid);
        list = list->next;
    }
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
 * Return -1    if the user wants to exit
 * Return 1     if the user wants to switch to sequential mode
 * Return 2     if the user wants to see all the currently running processes printed out
 * Return 3     if the user wants to pause a process
 * Return 4     if the user wants to resume a process
 * Return 0     otherwise
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
			printf("Switch to sequential\n");
			return 1;
		}else if((strncmp(arg[1],"s",1))==0 && strlen(arg[1])==1){
			printf("Switch to sequential\n");
			return 1;
		}else if((strncmp(arg[1],"parallel",8))==0){
			printf("We are already in parallel mode\n");
			return 0;
		}else if((strncmp(arg[1],"p",1))==0 && strlen(arg[1])==1){
			printf("We are already in parallel mode\n");
			return 0;
		}				
	}
	if(strncmp(arg[0],"jobs",4)==0 && arg[1]==NULL){
	    return 2;
	}
	if(strncmp(arg[0],"pause",5)==0 && arg[1]!=NULL && arg[2]==NULL){
	    return 3;
	}		
	if(strncmp(arg[0],"resume",6)==0 && arg[1]!=NULL && arg[2]==NULL){
	    return 4;
	}		
	pid_t child_pid = fork();
	if(child_pid == 0){
	    int r = execv(arg[0],arg);
		if(r<0){
			fprintf(stderr, "Error: execv failed in parallel mode\n");
			return -2;
	    } 	
	}
	return child_pid;
}

/*
 * Execute the input command in sequential mode
 * Input must be an array of pointers to char (or an array of strings)
 * The last element of the input array must be NULL
 *
 * Return -1    if the user wants to exit
 * Return 1     if the user wants to switch to parallel mode
 * Return 0     otherwise
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
			printf("We are already in sequential mode\n");
			return 0;
		}else if((strncmp(arg[1],"s",1))==0 && strlen(arg[1])==1){
			printf("We are already in sequential mode\n");
			return 0;
		}else if((strncmp(arg[1],"parallel",8))==0){
			printf("Switch to parallel\n");
			return 1;
		}else if((strncmp(arg[1],"p",1))==0 && strlen(arg[1])==1){
			printf("Switch to parallel\n");
			return 1;
		}		
	}
		
	pid_t child_pid = fork();
	if(child_pid == 0){
		if(execv(arg[0],arg)<0){		
			fprintf(stderr, "Error: execv failed in sequential mode\n");
			return 0;
		}
	}else{
		int status;
		waitpid(child_pid, &status, 0);
	}	
	return 0;	
	
}

/*
 * PATH varaible capability
 * Check if the command (arg) refers to an actual file
 * If it does, return the command as it is
 * If not, prepend path variable element given in linked list paths
 * and check if it exists
 * If the prepended path exists, return the updated command
 * If not, return NULL
 */
char* path(char *arg, struct node *paths){
    struct stat statresult;
    int rv = stat(arg,&statresult);
    char *result = malloc(50*sizeof(char));
    if(rv<0){
        struct node *temp = paths;
        while(temp!=NULL){ 
            struct stat statresult2;           
            char str[50] = "";
            sprintf(str, "%s/%s", temp->string,arg);
            int rv2 = stat(str, &statresult2);
            if(rv2==0){              
                strcpy(result,str);
                return result;
            }
            temp = temp->next;
        }
        fprintf(stderr, "Error: path not found\n");
        free(result);
        return NULL;
    }else{
        strcpy(result,arg);
        return result;
    }
}

/*
 * Check if the processes are finished and kill them
 */ 
void check_child(struct p_node **processes){
    struct pollfd pfd[1];
    pfd[0].fd = 0; // stdin is file descriptor 0
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
 
    // wait for 100 milliseconds
    int rv = poll(&pfd[0], 1, 100);
    
    pid_t temp[50];
    int index = 0;
 
    //collect all the finished child processes
    struct p_node *t = *processes;
    while(t!=NULL){
        int status;
        if(waitpid(t->pid,&status,WNOHANG)!=0){
            printf("Process %d completed\n",t->pid);
            temp[index++] = t->pid;
        }
        t = t->next;  
    }
    //kill all the finished child processes
    for(int i=0; i<index; i++){
        p_list_delete(temp[i],processes);
        kill(temp[i],0);
    }
}

int main(int argc, char **argv) {
	bool mode = true; //true when sequential, false when parallel
	bool directory = true;
	
	struct p_node *processes = NULL;
	int p_index = 0;
    
    //read shell-config
    FILE *f = fopen("shell-config","r");
    struct node *paths = NULL;
    if(!f){
        fprintf(stderr, "Error: shell-config not found\n");
        directory = false;
    }else{
        char line[50];
        //create a linked list of directories
        while(fgets(line,50,f)!=NULL){
            tokenify(line,&paths,"\n");
        }  
        fclose(f);
    } 
    
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
		
		check_child(&processes);
		
		//linked list of commands separated by semicolons
		struct node *commands = NULL;	
		int command_num = tokenify(buffer,&commands,";");
		
		//boolean used in parallel mode in case one of the commands calls exit
		bool quit_later = false;
		
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
			    //if this is arg[0] AND the command is not a built-in command
			    if(directory && (index==0) && strncmp(temp2->string,"mode",4)!=0 && strncmp(temp2->string,"exit",4)!=0 && strncmp(temp2->string,"jobs",4)!=0 && strncmp(temp2->string,"pause",5)!=0&& strncmp(temp2->string,"resume",6)!=0){
			        //update the command if needed
			        arg[0] = path(temp2->string,paths); 		        
			    }else{
			        arg[index] = temp2->string;
			    }
			    index++;
				temp2 = temp2->next;
			}
			//set the last element of the array to null	
			arg[index] = NULL;			
			
			//if the argument is valid, execute
	        if(arg[0]!=NULL){		
			    int result = 0;
			    if(mode==true){
				    result = seq_execute(arg);
			    }else{
				    result = par_execute(arg);
			    }			    
			    			    
			    //exev failed
			    if(result==-2){
			        //do nothing
			    }
			    //user wants to exit in sequential mode
			    else if(mode==true && result==-1){
			        //if there aren't any processes running, exit
			        if(processes==NULL){
				        list_clear(command);
				        list_clear(commands);
				        list_clear(paths);
				        p_list_clear(processes);
				        free(arg);
				        exit(0);
				    }
				    //if not, cannot exit
				    else{
				        fprintf(stderr,"Error: cannot exit because processes are still running\n");
				    }
				    
			    }
			    //user wants to exit in parllel mode
			    else if(mode==false && result==-1){
			        //will exit once all commands are executed
			        quit_later = true;
			    }
			    //user wants to switch mode
			    else if(result==1){
				    mode ^= true;
			    }
			    //user wants to see all the running processes
			    else if(result==2){
			        struct p_node *t = processes;
			        if(processes!=NULL){
			            printf("Still running: \n");
			            p_list_print(t);
			        }
			    }
			    //user wants to pause a process
			    else if(result==3){
			        kill(atoi(arg[1]),SIGSTOP);
			    }
			    //user wants to resume a process
			    else if(result==4){
			        kill(atoi(arg[1]),SIGCONT);
			    }
			    //command was normally executed
			    else{			        
			        if(result!=0 && mode==false){			            	
			            p_list_append(result,&processes);
			            check_child(&processes);
			            p_list_print(processes);
			        }
			    }
			    check_child(&processes);
			}
			temp = temp->next;				
			list_clear(command);
			free(arg);		
			
		}
		list_clear(commands);
		//if there was an exit command in parallel mode we can now exit
		if(quit_later==true){
		    if(processes==NULL){
		        list_clear(paths);
		        p_list_clear(processes);		        
		        exit(0);
		    }else{
		        fprintf(stderr,"Error: cannot exit because processes are still running\n");
		    }		    
		}	
		printf("\nprompt> ");		
	}
	list_clear(paths);
	p_list_clear(processes);
	fflush(stdout);
    return 0;
}

//308541739
//Elor lichtziger

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <malloc.h>
#include <stdlib.h>

struct Job {
    pid_t pid;
    char command[1000];
    struct Job *next;
    struct Job *prev;
};
enum Bool {True = 1, False = 0};
enum Bool checkAmpersand(char *args[]);
void cdFunc(char *arg, char *oldDir);
void updateJobs(struct Job *head);
void printJobs(struct Job *head);
void exitTerminal(struct Job *head, char *oldDir, char *input2);
enum Bool checkQuotes(char *input2);

int main() {
    enum Bool flag = True, flagPrintPid = True, ampersand ;
    char input[1000], command[1000];
    char *args[100], *token, *dir, *oldDir, *input2;
    char space[2] = " ", qu[2] = "\"";
    int i, status;
    pid_t val;
    struct Job *head, *current;
    input2 = malloc(1000* sizeof(char));
    oldDir = malloc(sizeof(char) * 1000);
    head = malloc(sizeof(struct Job));
    head->next = NULL;
    head->prev = NULL;
    current = head;
    while(flag) {
        i = 0;
        //handle with case which the child print the PID after the father print the next "prompt>".
        sleep(1);
        printf("prompt> ");
        fgets (input, 100, stdin);
        //divide the input command into arguments.
        strcpy(command, input);
        strcpy(input2, input);
        //check if there is "" signs in the input.
        if(checkQuotes(input2)) {
            token = strtok(input, qu);
            args[i] = token;
            while( token != NULL ) {
                token = strtok(NULL, qu);
                if((token == NULL) || (strcmp(token, "\n") == 0) || (strcmp(token, " ") == 0)) {
                    continue;
                }
                i++;
                args[i] = token;
            }
            args[i + 1] = NULL;
            strcpy(input2, args[0]);
            args[0] = strtok(input2, " ");

        } else {
            token = strtok(input, space);
            args[0] = token;
            if(strcmp(args[0], "\n") == 0) {
                continue;
            }
            do {
                ++i;
                token = strtok(NULL, space);
                args[i] = token;
            } while( token != NULL );
            args[i-1] = strtok(args[i-1], "\n");
        }
        //check if the command is cd, jobs or something else.
        if(strcmp(args[0], "jobs") == 0) {
            updateJobs(head);
            printJobs(head);
        } else if(strcmp(args[0], "cd") == 0) {
            printf("%d\n", getpid());
            cdFunc(args[1], oldDir);
        } else if(strcmp(args[0], "exit") == 0) {
            printf("%d\n", getpid());
            flag = False;
        } else {
            //current = findLastJob(head);
            current->next = malloc(sizeof(struct Job));
            current->next->prev = current;
            current = current->next;
            //check if there is an '&' sign in the command.
            ampersand = checkAmpersand(args);
            if((val = fork()) < 0) {
                printf("\nThere was an error while creating another process.\n");
            } else {
                //parent process
                if(val > 0) {
                    //update the job in the linked-list of Jobs.
                    current->pid = val;
                    current->next = NULL;
                    strcpy(current->command,command);
                    //this is command without '&'.
                    if(!ampersand) {
                        //wait for the child process.
                        if(waitpid(val, &status, 0) != val) {
                            printf("the process have been failed");
                        }
                        //the system call have been failed.
                        if(!WIFEXITED(status)) {
                            fprintf(stderr, "Error in system call\n");
                        }
                    }
                    //child process.
                } else if(val == 0) {
                    printf("%d\n", getpid());
                    execvp(args[0], args);
                }
            }
        }
    }
    exitTerminal(head, oldDir, input2);
}

enum Bool checkAmpersand(char *args[]) {
    int i = 0;
    while(args[i] != NULL) {
        if(strcmp(args[i], "&") == 0) {
            args[i] = NULL;
            return True;
        }
        ++i;
    }
    return False;
}
void cdFunc(char *arg, char *oldDir) {
    char buf[1000];
    char *dir;
    if(strcmp(arg, "-") == 0) {
        if(strcmp(oldDir, "") == 0) {
            printf("\ncd: OLDPWD not set");
        } else {
            chdir(oldDir);
            return;
        }
    }
     strcpy(oldDir,getcwd(buf, sizeof(buf)));
    if((arg == NULL) || (strcmp(arg, "~") == 0)) {
        chdir(getenv("HOME"));
    } else {
        dir = getcwd(buf, sizeof(buf));
        dir = strcat(dir, "/");
        dir = strcat(dir, arg);
        chdir(dir);
    }
}
void updateJobs(struct Job *head) {
    int status;
    pid_t pid;
    struct Job *delCurrent, *current;
    enum Bool flag = True;
    if(head->next != NULL) {
        current = head->next;
        while(flag) {
            pid = current->pid;
            pid_t retPid = waitpid(pid, &status, WNOHANG);
            if(retPid != 0) {
                //get out the job from the linked-list.
                current->prev->next = current->next;
                if (current->next != NULL) {
                    current->next->prev = current->prev;
                    delCurrent = current;
                    current = current->next;
                    free(delCurrent);
                } else {
                    current->prev->next = NULL;
                    free(current);
                    flag = False;
                }
            } else {
                if(current->next == NULL) {
                    flag = False;
                } else {
                    current = current->next;
                }
            }
        }
    }
}
void printJobs(struct Job *head) {
    struct Job *current;
    enum Bool flag = True;
    if(head->next != NULL) {
        current = head->next;
        while (flag) {
            printf("%d ",current->pid);
            printf("%s\n", strtok(current->command, "&"));
            if(current->next == NULL) {
                flag = False;
            } else {
                current = current->next;
            }
        }
    }
}
void exitTerminal(struct Job *head, char *oldDir, char *input2) {
    struct Job *delJob;
    do {
        delJob = head;
        if(head->next != NULL) {
            head = head->next;
        }
        free(delJob);
    }while(head->next != NULL);
    free(oldDir);
    free(input2);
    exit(0);
}
enum Bool checkQuotes(char *input2) {
    int i = 0;
    while (*input2)
    {
        if (strchr("\"", *input2))
        {
            i++;
        }

        input2++;
    }
    if((i > 0) && (i % 2 == 0)) {
        return True;
    }
    return False;
}

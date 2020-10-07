#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char** args;
char *getdir;
char *dir;
char *to;
bool background = false;

char *read_line() {
  int bufsize = 1024;
  int i = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;
	while(true) {

		c = getchar();
		
		if(c == EOF) {
			system("echo");
			exit(0);
		} else if(c == '\n') {
			buffer[i] = '\0';
      		return buffer;
		} else {
			buffer[i] = c;
		}
		i++;
	}
}

void exit_command(char* line) {
	printf("%s\n", line);
	if (!strcmp(line, "exit")) {
		exit(0);
	} 
	else {
		printf("lhs: unknown command \"%s\"\n", line);
	}
}

char **split_line(char *line) {
	int i = 0;
	char **words = malloc(64 * sizeof(char*));
	char *word;
	word = strtok(line, " \t\r\n\a");
	while (word != NULL) {
		words[i] = word;
		word = strtok(NULL, " ");
		i++;
	}
	words[i] = NULL;

	return words;
}

void cd_command() {
  if (args[1] == NULL) {
    printf("Expected argument to \"cd\"\n");
    return;
  } 
	dir = strcat(getdir, "/");
	to = strcat(dir, args[1]);
	chdir(to);
}

int findPipe() {
	for (int i = 0; i <= sizeof(args); i++) {
		if (args[i] != NULL) {
			if (!(strcmp(args[i], "|"))) {
				return i;
			}
		}
		else {
			return 0;
		}
	}
	return 0;
}

void pipe_command(int place, int argumentsSize){

	int pipefd[2];
	char* argsBefore[place+1];		
	char* argsAfter[(argumentsSize+1)-place];
	pid_t pid;
	
	if(pipe(pipefd) < 0){
		printf("Error during creating pipe.\n");
		return;
	}

	for(int i=0; i < place; i++){
		argsBefore[i] = args[i];
	}

	argsBefore[place] = NULL;

	int j=0;

	for(int i=place+1; i < argumentsSize; i++){	
		argsAfter[j] = args[i];
		j++ ;
	} 

	pid = fork();

	if(pid < 0){
		printf("pid err\n");
		return;
	}

	if(pid==0){
		
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);

		if(execvp(argsBefore[0],argsBefore) < 0){
			printf("execvp1 err\n");
			return;
		}
	} else {

		pid=fork();

		if(pid < 0){
			printf("pid err\n");
			return;
		}

		if(pid==0){
			dup2(pipefd[0],STDIN_FILENO);
			close(pipefd[1]);
			close(pipefd[0]);

			if(execvp(argsAfter[0],argsAfter) < 0){
				printf("execvp2 err\n");
				return;
			}
		} 
		else {
			int status;
			close(pipefd[0]);
			close(pipefd[1]);
			waitpid(pid,&status,0);
			wait(NULL);
		}
	}
}

int getArgsSize() { 
	int i;
	for (i = 0; i < sizeof(args); i++) {
		if(args[i] == NULL) {
			break;
		}
	}
	return i + 1;
}

int findErr() {
	for (int i = 0; i <= sizeof(args); i++) {
		if (args[i] != NULL) {
			if (!(strcmp(args[i], "2>"))) {
				return i;
			}
		}
		else {
			return 0;
		}
	}
	return 0;
}

int findIn() {
	for (int i = 0; i <= sizeof(args); i++) {
		if (args[i] != NULL) {
			if (!(strcmp(args[i], "<"))) {
				return i;
			}
		}
		else {
			return 0;
		}
	}
	return 0;
}

int findOut() {
	for (int i = 0; i <= sizeof(args); i++) {
		if (args[i] != NULL) {
			if (!(strcmp(args[i], ">"))) {
				return i;
			}
		}
		else {
			return 0;
		}
	}
	return 0;
}

void err_command(int place, int argumentsSize) {
	char *argsBefore[place+1];
	char* argsAfter[(argumentsSize+1)-place]; 
	pid_t pid;
	int fd;

	for(int i=0; i < place; i++) {
		argsBefore[i] = args[i];
	}

	argsBefore[place] = NULL;

	int j=0;

	for(int i=place+1; i < argumentsSize; i++) {
		argsAfter[j] = args[i];
		j++ ;
	} 

	pid=fork();

	if(pid==0){

		close(0);
		fd = open(argsAfter[0], O_RDWR);
		dup2(fd, 2);
		execvp(argsBefore[0],argsBefore);
		printf("execvp error\n");
		return;
	}
	else{
		wait(NULL);

	}
}

void in_command(int place, int argumentsSize) {
	char *argsBefore[place+1];
	char* argsAfter[(argumentsSize+1)-place]; 
	pid_t pid;
	int fd;

	for(int i=0; i < place; i++){
		argsBefore[i] = args[i];
	}

	argsBefore[place] = NULL;

	int j=0;

	for(int i=place+1; i < argumentsSize; i++){
		argsAfter[j] = args[i];
		j++ ;
	} 

	pid=fork();

	if(pid==0){

		close(0);
		fd = open(argsAfter[0], O_RDWR);
		dup2(fd, 0);
		execvp(argsBefore[0],argsBefore);
		printf("execvp error\n");
		return;
	}
	else{
		wait(NULL);

	}
}

void out_command(int place, int argumentsSize) {
	char* argsBefore[place+1];				
	char* argsAfter[(argumentsSize+1)-place]; 	
	pid_t pid;
	int fd;

	for(int i=0; i < place; i++){
		argsBefore[i] = args[i];
	}

	argsBefore[place] = NULL;

	int j=0;

	for(int i=place+1; i < argumentsSize; i++){	
		argsAfter[j] = args[i];	
		j++ ;
	} 

	pid=fork();

	if(pid==0){
		close(1);
		fd = creat(argsAfter[0], 0644);
		dup2(fd, 1);
		execvp(argsBefore[0],argsBefore);
		printf("execvp error\n");
		return;
	}
	else{
		wait(NULL);
	}
}

bool findBackground(int argsSize) {
	if (!strcmp(args[argsSize-2], "&")) {
		args[argsSize-2] = NULL;
		return true;
	}
	return false;
}

void signal_handler() {
	printf("\n");
	return;
}

int main(int argc, char **argv, char **envp){

	char *line;
	char path[100];
	pid_t pid;

	signal(SIGINT, signal_handler);
	signal(SIGCHLD, SIG_IGN);

	do{
		getdir= getcwd(path, sizeof(path));
		printf("\e[1;31mlsh:~\e[0m%s", path);
		printf("\e[1;32m:-> \e[0m");		
		line = read_line();		
		args = split_line(line);

		int p = findPipe();
		int e = findErr();
		int i = findIn();
		int o = findOut();

		if (*args == NULL) {
			continue;
		}

		if (e > 0) {
			err_command(e, getArgsSize());
			free(line);
			free(args);
			continue;
		}

		if (p > 0) {
			pipe_command(p, getArgsSize());
			free(line);
			free(args);
			continue;
		}

		if (i > 0) {
			in_command(i, getArgsSize());
			free(line);
			free(args);
			continue;
		}

		if (o > 0) {
			out_command(o, getArgsSize());
			free(line);
			free(args);
			continue;
		}

		if (!(strcmp(*args, "cd"))) {
			cd_command();
			free(line);
			free(args);
			continue;
		}

		background = findBackground(getArgsSize());

		pid = fork();
		int status;

		if(!strcmp(*args, "exit")) {
			exit(0);
		}

		if(pid == 0) {
			if (execvp(*args, args) < 0) {
				printf("Unknown command \"%s\"\n", *args);
				continue;
			} 
		}

		if (!background) {
			waitpid(pid, &status, WUNTRACED);
		}

		background = false;

		free(line);
		free(args);

	}while(true);

	return 0;
}
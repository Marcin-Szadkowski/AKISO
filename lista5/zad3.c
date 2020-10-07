#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<signal.h>
#include <fcntl.h>

//Deklaracje wbudowanych funkcji
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

//Lista wbudowanych polecen, ktorym odpowiadaja wbudowane funkcje
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//Implementacje funkcji wbudowanych

//Change Directory
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

//Help
int lsh_help(char **args)
{
  int i;
  printf("********************************\n");
  printf("Simple shell LSH\n");
  printf("Working: pipes, redirection of stdout, stdin and stderr\n");
  printf("Control+C terminates working program\n");
  printf("To terminate shell enter \"exit\"\n");
  printf("Here are built in functions:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }
  printf("Man command provides further informations.\n");
  printf("********************************\n");
  return 1;
}

//Wbudowane polecenie exit
int lsh_exit(char **args)
{
  return 0;
}
//Zwraca liczbe elementow dynamicznej tablicy
int getSize(char **args){
  int size=0;

  while(args[size] != NULL){
    size++;
  }
  return size;
}
//Funkcja usuwajaca ostatni element tablicy
char** deleteLastEl(char **args, int size){
    args[size-1]= NULL;
    return args;
}
//Funkcja wykonujaca potok polecen
//Pierwszy argument to pierwsze polecenie
//Drugi argument to drugie polecenie
int execArgsPiped(char** args) 
{ 
  int pipefd[2];
	pid_t pid;

  int size = getSize(args);
  char **argsBefore = malloc( size* sizeof(char*));
  char **argsAfter = malloc( size * sizeof(char*));
  //Dziele argumenty na dwie czesci
  int i=0;
  while( args[i] != NULL ){
    if( strcmp( args[i], "|") != 0){
      argsBefore[i] = args[i];
      i++;
    }
    else if( strcmp(args[i], "|") == 0)
      break;   
  }
  argsBefore[i] = NULL;
  i++;
  int j=0;
  while( args[i] != NULL ){
    argsAfter[j]= args[i];
    i++;
    j++;  
  } 
	
	if(pipe(pipefd) < 0){
		printf("lsh:pipe error:execArgsPiped()\n");
		return 1;
	}
	pid = fork();

	if(pid < 0){
		printf("lsh:fork error:execArgsPiped()\n");
		return 1;
	}

	if(pid==0){	
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);
		close(pipefd[1]);

		if(execvp(argsBefore[0],argsBefore) < 0){
			printf("lsh:execvp1 err:execArgsPiped()\n");
			return 1;
		}
	} else {

		pid=fork();

		if(pid < 0){
			printf("lsh:fork error:execArgsPiped()\n");
			return 1;
		}

		if(pid==0){
			dup2(pipefd[0],STDIN_FILENO);
			close(pipefd[1]);
			close(pipefd[0]);

			if(execvp(argsAfter[0], argsAfter) < 0){
				printf("lsh:execvp2 err:execArgsPiped()\n");
				return 1;
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
    free(argsBefore);
    free(argsAfter);
    return 1; 
} 

//Funkcja wykonujaca zwykle polecenie
//Tzn. bez potokow, bez przekierowania stdout, stdin, stderr
int lsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;
  int size= getSize(args);
  char c = args[size -1 ][0];
  //Zmienna przechowujaca informacje o tym czy proces ma czekac na proces potomny
  int wait = 1;

  //Jesli ostatni znak to & to go usun i zmien wartosc wait na 0 
  if( c == '&'){ 
    wait =0;  //Teraz proces nie bedzie czekac az proces potomny zakonczy dzialanie
    args = deleteLastEl(args, size);
  }
  pid = fork();
  if (pid == 0) {
    //Proces potomny wykonuje polecenie
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    //Blad fork
    perror("lsh");
  } else if( wait == 1){ //Jesli wait == 1 to poczekaj az child skonczy dzialanie
    //Rodzic czeka na proces potomny
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }else
  {
    //Ignoruj sygnal SIGCHLD
    //Dzieki temu wejsciu procesu potomnego jest kasowane i nie ma problemu procesu zombie
    signal(SIGCHLD, SIG_IGN);
  }  
  return 1;
}
//Funkcja wykonujaca polecenie z przekierwoaniem standardowego wyjscia
int lsh_execute_in(char **args){
  int size = getSize(args);
  char **argsBefore = malloc( size* sizeof(char*));
  char **argsAfter = malloc( size * sizeof(char*));
  pid_t pid;
  int fd;

  //Dziele argumenty na dwie czesci
  int i=0;
  while( args[i] != NULL ){
    if( strcmp( args[i], "<") != 0){
      argsBefore[i] = args[i];
      i++;
    }
    else if( strcmp( args[i], "<") == 0)
      break;   
  }
  argsBefore[i] = NULL;
  i++;
  int j=0;
  while( args[i] != NULL ){
    argsAfter[j]= args[i];
    i++;
    j++;  
  }
  pid=fork();

	if(pid==0){

		close(0);
		fd = open(argsAfter[0], O_RDWR);
		dup2(fd, 0);
		execvp(argsBefore[0],argsBefore);
		printf("lsh:lsh_execute_in:execvp err\n");
		return 1;
	}
	else{
		wait(NULL);

	}
  //Zwolnienie alokowanej pamieci
  free(argsBefore);
  free(argsAfter);
  return 1;
}
//Funkcja wykonujaca polecenie z przekierowaniem standardowego wyjscia
int lsh_execute_out(char** args){
  int size = getSize(args);
  char **argsBefore = malloc( size* sizeof(char*));
  char **argsAfter = malloc( size * sizeof(char*));
  pid_t pid;
  int fd;

  //Dziele argumenty na dwie czesci
  int i=0;
  while( args[i] != NULL ){
    if( strcmp( args[i], ">") != 0){
      argsBefore[i] = args[i];
      i++;
    }
    else if( strcmp( args[i], ">") == 0)
      break;   
  }
  argsBefore[i] = NULL;
  i++;
  int j=0;
  while( args[i] != NULL ){
    argsAfter[j]= args[i];
    i++;
    j++;  
  } 
  pid=fork();

	if(pid==0){
		close(1);
		fd = creat(argsAfter[0], 0644);
		dup2(fd, 1);
		execvp(argsBefore[0],argsBefore);
		printf("lsh:lsh_execute_out():execvp err\n");
		return 1;
	}
	else{
		wait(NULL);
	}
  free(argsAfter);
  free(argsBefore);
  return 1;
}
//Funkcja wykonujaca polecenie zawierajace przekirowanie wyjscia bledu
int lsh_execute_error(char **args){
  int size = getSize(args);
  char **argsBefore = malloc( size* sizeof(char*));
  char **argsAfter = malloc( size * sizeof(char*));
  pid_t pid;
  int fd;

  //Dziele argumenty na dwie czesci
  int i=0;
  while( args[i] != NULL ){
    if( strcmp( args[i], "2>") != 0){
      argsBefore[i] = args[i];
      i++;
    }
    else if( strcmp( args[i], "2>") == 0)
      break;   
  }
  argsBefore[i] = NULL;
  i++;
  int j=0;
  while( args[i] != NULL ){
    argsAfter[j]= args[i];
    i++;
    j++;  
  } 
  pid=fork();

	if(pid==0){

		close(0);
		fd = open(argsAfter[0], O_RDWR);
		dup2(fd, 2);
		execvp(argsBefore[0],argsBefore);
		printf("lsh:lsh_execute_error:execvp err\n");
		return 1;
	}
	else{
		wait(NULL);
	}
  free(argsBefore);
  free(argsAfter);
  return 1;
}

//Funkcja wykonujaca wbudowane polecenie lub polecenie jesli takie istnieje
//Lub zwykle polecenie w powloce
//Zwraca 0 jesli program ma zakonczyc dzialanie
//Czyli wtedy kiedy uzytkownik wpisal  "exit"
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    //Nic nie wpisano
    return 1;
  }
  //Sprawdzam czy podane polecenie jest wbudowane
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  //Jesli to nie wbudowane polecenie to args przekazuje do innej funkcji
  return lsh_launch(args);
}

#define LSH_RL_BUFSIZE 1024
//Funkcja czytajaca wiersz polecen
char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    //Czytaj znak
    c = getchar();

    //Jezeli napotkamy EOF to wpisz NULL
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    //Zarezerwuj wiecej pamieci jest rozmiar buffsize zostal przekroczony
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}
#define LSH_TOK_BUFSIZE 64
#define LSH_PIPE_DELIM "|"

//Funkcja sprawdzajaca czy podany wiersz jest potokiem polecen
int isPipe(char** args){
  int i =0;
  while( args[i] != NULL){
    if(strcmp(args[i], "|") == 0)
      return 1; //Tzn, ze polecenie zawiera potok polecen
    i++;
  }
  return 0;
}
//Funkcja sprawdza czy w podanej linii polecenei znajduje sie przekierowanie standardowego wejscia
//Zwraca 1 jesli linia zawiera przekierowanies
int isIn(char** args){
  int i =0;
  while( args[i] != NULL){
    if(strcmp(args[i], "<") == 0)
      return 1; //Tzn, ze polecenie zawiera przekierowanie wejscia
    i++;
  }
  return 0;
}
//Funkcja sprawdza czy w podanej linii polecenei znajduje sie przekierowanie standardowego wyjscia
//Zwraca 1 jesli linia zawiera przekierowanie
int isOut(char** args){
  int i =0;
  while( args[i] != NULL){
    if(strcmp(args[i], ">") == 0)
      return 1; //Tzn, ze polecenie zawiera przekierowanie wyjscia
    i++;
  }
  return 0;
}
int isError(char** args){
  int i =0;
  while( args[i] != NULL){
    if(strcmp(args[i], "2>") == 0)
      return 1; //Tzn, ze polecenie zawiera przekierowanie wyjscia bledu
    i++;
  }
  return 0;
}


#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
//Funkcja dzielaca wiersz polocen na poszczegolne wyrazy
//Wedlug podanego powyzej wzorca
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

//Glowna petla
void lsh_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
   // printf("> ");
    printf("\e[1;31m> \e[0m");	  
    //Pobierz linie polecen  
    line = lsh_read_line();
    //Rozdziel linie na poszczegolne argumenty
    args = lsh_split_line(line);
    
    if(isIn(args)){
      //Wykonaj polecenie z przekierowaniem wejscia   
      status = lsh_execute_in(args);
    }
    else if(isOut(args)){
      //Wykonaj polecenie z przekierowaniem wyjscia
      status = lsh_execute_out(args);
    }
    else if(isError(args)){
      //Wykonaj polecenie z przekierowaniem wyjscia bledu
      status = lsh_execute_error(args);
    }
    //Jesli to potok
    else if(isPipe(args)){  
      status = execArgsPiped(args);      
    }
    else { 
         status = lsh_execute(args);       
    }
      
    free(line); //zwolnij pamiec wskazywana przez wskaznik line
    free(args); //zwolnij pamiec wskazywana przez wskaznik args
  } while (status);
}
void simpleSigHandler(){
  printf("\n");
  return;
}
/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  //Przechwyc sygnal SIGINT, czyli Control+C
  //I zakoncz proces potomny
  signal(SIGINT, simpleSigHandler);
	signal(SIGCHLD, SIG_IGN);
  //Uruchom glowna petle
  lsh_loop();

  return EXIT_SUCCESS;
}
/**
 * Klient z szyfrowaniem
 * Komplikowac:
 * $ gcc client2.c -o client2 -L/usr.lib -lssl -lcrypto
 * Uruchamiac wpisujac:
 * $ ./client 127.00.1 8888 
 * 
 **/

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
 #include <unistd.h>
 #include<openssl/ssl.h>
 #include <openssl/err.h>
/*for getting file size using stat()*/
#include<sys/stat.h>
 
/*for sendfile()*/
#include<sys/sendfile.h>
 
/*for O_RDONLY*/
#include<fcntl.h>

char* loadFile(char* filename);

//Funkcja zwracaja dlugosc tablicy
int getSize(char *args){
  int size=0;

  while(args[size] != '\0'){
    size++;
  }
  return size;
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
SSL_CTX* InitCTX(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLSv1_2_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}
void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}
int main(int argc,char *argv[])
{
    //Zmienne potrzebne do szyfrowania
    SSL_CTX *ctx;
    SSL *ssl;

  	struct sockaddr_in server;
	struct stat obj;
	struct hostent *serv; //dodane
	int sock;
	int lineSize;
	char buf[100], command[5], filename[20], password[20], *f;
	int k, size, status;
	int filehandle;
    int bytes;
	char *line;
  	char **args;
	sock = socket(AF_INET, SOCK_STREAM, 0);

    SSL_library_init(); //SSL
    ctx = InitCTX();

	if(sock == -1){
		printf("socket creation failed");
		exit(1);
	}

	serv = gethostbyname(argv[1]);
	if (serv == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	//Initialiaze
	bzero((char *) &server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = 0;
	k = connect(sock,(struct sockaddr*)&server, sizeof(server));
	if(k == -1){
		printf("Connect Error");
		exit(1);
	}
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, sock);    /* attach the socket descriptor */
    if( SSL_connect(ssl) == -1){ //Polacz
        ERR_print_errors(stderr);
    }
	int i = 1;
    ShowCerts(ssl);
	do{
		printf("Input password: ");
		scanf("%s", password);
        SSL_write(ssl, password, strlen(password));
		//send(sock, password, 20, 0);
		//recv(sock, &status, sizeof(int), 0);
        SSL_read(ssl, &status, sizeof(status));
	}
	while( status != 1);
	lsh_read_line();
	

	while(1){
		printf("\e[1;31m> \e[0m");
		//Pobierz linie polecen  
    	line = lsh_read_line();
    	//Rozdziel linie na poszczegolne argumenty
    	args = lsh_split_line(line);
								
		lineSize = getSize(line);
		if(!strcmp("get", args[0])){
			if(args[1] != NULL ){
				strcpy(buf, "get ");
				strcat(buf, args[1]);
                SSL_write(ssl, buf, sizeof(buf));
                SSL_read(ssl, &size, sizeof(int));
			}else
				printf("get command requires filename\n");
			
			if(!size){
				printf("No such file on the remote directory\n\n");
				continue;
			}
			f = malloc(size);

			SSL_read(ssl, f, size);
			while(1){
				filehandle = open(args[1], O_CREAT | O_EXCL | O_WRONLY, 0666);
				if(filehandle == -1){
					sprintf(args[1] + strlen(args[1]), "%d", i);//needed only if same directory is used for both server and client
				}
				else break;
			}
			write(filehandle, f, size);

		} else if(!strcmp("put", args[0])){
            char* buffer;
			//scanf("%s", filename);
			strcpy(filename, args[1]);
			filehandle = open(filename, O_RDONLY);
			if(filehandle == -1){
				printf("No such file on the local directory\n\n");
				break;
			}
			strcpy(buf, "put ");
			strcat(buf, args[1]);
			//send(sock, buf, 100, 0);
            SSL_write(ssl, buf, 100);
			stat(filename, &obj);
			size = obj.st_size;
			//send(sock, &size, sizeof(int), 0);
            SSL_write(ssl, &size, sizeof(int));
            buffer = loadFile(filename);
            SSL_write(ssl, buffer, size);
			//sendfile(sock, filehandle, NULL, size);
            SSL_read(ssl, &status, sizeof(int));
			//recv(sock, &status, sizeof(int), 0);
			if(status)
				printf("File stored successfully\n");
			else
				printf("File failed to be stored to remote machine\n");
			
		} else if(!strcmp("pwd", args[0])){
			strcpy(buf, args[0]);
			SSL_write(ssl, buf, 100);
			bytes = SSL_read(ssl, buf, 100);
            buf[bytes] = '\0';
			printf("The path of the remote directory is: %s\n", buf);
		} else if(!strcmp("ls", args[0])){
			SSL_write(ssl, args[0], 100);
			SSL_read(ssl, &size, sizeof(int));
			f = malloc(size*size);
			SSL_read(ssl, f, size);
			filehandle = creat("temp.txt", O_WRONLY);
			write(filehandle, f, size);
			close(filehandle);
			printf("The remote directory listing is as follows:\n");
			system("cat temp.txt");
		} else if(!strcmp("cd", args[0])){
            strcpy(buf, "cd ");
			strcat(buf, args[1]);
			SSL_write(ssl, buf, 100);
			SSL_read(ssl, &status, sizeof(int));
			if(status)
				printf("Remote directory successfully changed\n");
			else
				printf("Remote directory failed to change\n");
		} else if(!strcmp("lls", args[0])){
			system("ls");
		} else if(!strcmp("lcd", args[0])){
			strcpy(buf, args[1]);
			chdir(buf);
		} else if(!strcmp("lpwd", args[0])){
			system("pwd");
		}else if(!strcmp("quit", args[0])){
			//send(sock, args[0], 100, 0);
            SSL_write(ssl, args[0], 100);
			//recv(sock, &status, 100, 0);
            SSL_read(ssl, &status, sizeof(int));
			if(status){
				printf("\nQuitting from the server..\n");
				exit(0);
			}
			printf("Server failed to close connection\n");
		}
		free(line);
		free(args);
	}
    close(sock);
    SSL_CTX_free(ctx);
    return 0;
}
/**
 * Funkcja przepisujaca dane z pliku do wskaznika char*
 **/ 
char* loadFile(char* filename){
    char* buffer = 0;
    long length;
    FILE * f = fopen (filename, "rb");

    if (f){
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length+1);
        if (buffer){
            fread (buffer, 1, length, f);
        }
        fclose (f);
    }
    buffer[length]='\0';
    return buffer;
}
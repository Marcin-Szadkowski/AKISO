#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include "openssl/ssl.h"
#include "openssl/err.h"     
#define TRUE   1  
#define FALSE  0  
#define PORT 8888  
/**
 * Server obslugujacy wielu klientow z szyfrowanym polaczeniem
 * Uzyta biblioteka OpenSSL, aby wygenerowac wlasny certyfikat:
 * $ openssl req -x509 -nodes -days 365 -newkey rsa:1024 -keyout mycert.pem -out mycert.pem
 * Certyfikat musi byc w pliku gdzie jest server.
 * Kompilowac:
 * $ gcc multiServer2.c -o multiServer2 -L/usr/lib -lssl -lcrypto
 * Uruchamiac:
 * $ sudo ./multiServer2
 **/
void myServer(int socket, struct sockaddr_in* address, SSL* ssl);
SSL_CTX* InitServerCTX(void);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
char* loadFile( char* filename);

int main(int argc , char *argv[])   
{   
    SSL_CTX *ctx;
    int opt = TRUE;   
    int master_socket , addrlen , new_socket , client_socket[30] ,  
          max_clients = 30 , activity, i , valread , sd;   
    int max_sd, filehandle, size, c;   
    struct sockaddr_in address;
    struct stat obj;
    int status;   
         
    char buffer[1025], command[5], filename[20];  //data buffer of 1K  
    // Initialize the SSL library
    SSL_library_init();    
    ctx = InitServerCTX();        /* initialize SSL */
    LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs */
    //set of socket descriptors  
    fd_set readfds;   
             
    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < max_clients; i++)   
    {   
        client_socket[i] = 0;   
    }   
         
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888  
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", PORT);   
         
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(master_socket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
         
    while(TRUE)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < max_clients ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            SSL *ssl;
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }
            if(fork() != 0){
                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, new_socket);
                //inform user of socket number - used in send and receive commands  
                printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" ,
                    new_socket , inet_ntoa(address.sin_addr) , ntohs 
                  (address.sin_port));
                myServer(new_socket, &address, ssl);
                
            }else{
                printf("Acception fail\n");
            }
                      
            
                      
        }                  
    }   
    close(master_socket);    
    SSL_CTX_free(ctx); 
    return 0;   
}
/**
 * Funkcja obslugujaca Clienta
 **/ 
void myServer(int socket, struct sockaddr_in* address, SSL* ssl){
    char* buffer = malloc(1024 * sizeof(char));
    int valread, filehandle, size, addrlen, i, c;
    char command[5], filename[20];  //data buffer of 1K  
    struct stat obj;
    FILE* fp;
    int sd = socket;
    int bytes;
    int status;

    if( SSL_accept(ssl) == -1)
        ERR_print_errors_fp(stderr);
   // ShowCerts(ssl);

    while(1) {
        //read(socket, buffer, 1024);
        bytes = SSL_read(ssl, buffer, 1024);
        buffer[bytes] = '\0';
        if(strcmp(buffer, "password") == 0){
            status =1;
            printf("Correct password\n");
            SSL_write(ssl, &status, sizeof(int));
            break;
        }
        else{
            printf("Wrong password\n");
            status=0;            
            SSL_write(ssl, &status, sizeof(int));
        }
    }
    
    while(1){
        if ((bytes = SSL_read(ssl , buffer, 1024)) == 0) {
            //Somebody disconnected , get his details and print  
            getpeername(socket , (struct sockaddr*)address ,  (socklen_t*)&addrlen);   
            printf("Host disconnected , ip %s , port %d \n" ,  
                  inet_ntoa((*address).sin_addr) , ntohs((*address).sin_port));   
                 
            //Close the socket and mark as 0 in list for reuse  
            close(socket);  
            SSL_free(ssl); 
            break;
        }else{    
            buffer[bytes] = '\0';    
            sscanf(buffer, "%s", command);
            if(!strcmp(command, "ls"))
            {
                system("ls >temps.txt");
                i = 0;
                stat("temps.txt",&obj);
                size = obj.st_size;   
                buffer = loadFile("temps.txt");                          
                SSL_write(ssl, &size, sizeof(int));                
                SSL_write(ssl, buffer, size);
            }
            else if(!strcmp(command,"get"))
            {
                sscanf(buffer, "%s%s", filename, filename);
                stat(filename, &obj);
                filehandle = open(filename, O_RDONLY);
                size = obj.st_size;
                if(filehandle == -1)
                    size = 0;
                SSL_write(ssl, &size, sizeof(int));
                if(size){
                    //sendfile(sd, filehandle, NULL, size);
                    buffer = loadFile(filename);
                    SSL_write(ssl, buffer, size);
                }                    
            }
            else if(!strcmp(command, "put"))
            {
                int c = 0, len;
                char *f;
                sscanf(buffer+strlen(command), "%s", filename);
                //recv(sd, &size, sizeof(int), 0);
                SSL_read(ssl, &size, sizeof(int));
                i = 1;
                while(1)
                {
                    filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
                    if(filehandle == -1)
                    {
                        sprintf(filename + strlen(filename), "%d", i);
                    }
                    else
                        break;
                }
                f = malloc(size);
                //recv(sd, f, size, 0);
                SSL_read(ssl, f, size);
                c = write(filehandle, f, size);
                close(filehandle);
                //send(sd, &c, sizeof(int), 0);
                SSL_write(ssl, &c, sizeof(int));
            }
            else if(!strcmp(command, "pwd"))
            {
                system("pwd>temp.txt");
                i = 0;
                FILE*f = fopen("temp.txt","r");
                while(!feof(f))
                    buffer[i++] = fgetc(f);
                buffer[i-1] = '\0';
                fclose(f);
                SSL_write(ssl, buffer, 100);
            }
            else if(!strcmp(command, "cd"))
            {
                if(chdir(buffer+3) == 0)
                    c = 1;
                else
                    c = 0;
                SSL_write(ssl, &c, sizeof(int));
            }       
            else if(!strcmp(command, "quit"))
            {
                i = 1;
                //send(sd, &i, sizeof(int), 0);
                SSL_write(ssl, &i, sizeof(int));
            }
        }
    }
}   
SSL_CTX* InitServerCTX(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = TLSv1_2_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}
/**
 * Funkcja ladujaca lokalny certyfikat
 **/ 
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    /* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
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

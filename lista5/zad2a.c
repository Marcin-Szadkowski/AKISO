#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void signalHandler(int sig){
    if( sig == SIGUSR1)
        printf("Received signal SIGUSR1\n");
    else if( sig == SIGSTOP)
        printf("Received signal SIGSTOP\n");
    else if( sig == SIGKILL)
        printf("Received signal SIGKIIL'n");
    
}
int main(){
    //Ten sygnal mozna przechwycic
    signal(SIGUSR1, signalHandler);

    //Tych sygnalow nie da sie przechwycic
    if(signal(SIGSTOP, signalHandler) == SIG_ERR)
        printf("Can`t catch SIGSTOP\n");
    if(signal(SIGKILL, signalHandler) == SIG_ERR)
        printf("Can`t catch SIGSTOP\n");
    
    //Wyslanie sygnalow do procesu (do siebie samego)
    raise(SIGUSR1);
    raise(SIGSTOP);
    raise(SIGCONT);
    raise(SIGKILL);

}
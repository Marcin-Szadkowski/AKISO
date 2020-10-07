#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>


void message(int signum){
    printf("Signal was received\n");
}

int main(){
    //Wywolanie funkcji message jesli dotarl sygnal
    signal(SIGUSR1, message);

    for(int i =0; i< 30; i++){
        //wyslanie sygnalu 
        raise(SIGUSR1);        
    }        
}
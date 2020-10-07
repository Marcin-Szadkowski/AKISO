#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

int main(){
    
    if(kill(1, SIGKILL) == SIG_ERR)
        printf("Blad.\n");
}
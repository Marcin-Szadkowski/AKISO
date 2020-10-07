#include <stdlib.h>
#include <string.h> 

int main()
{
 int close(){
     int close;
     close=system("sudo rmmod hello");
     return close;
 }
 int show(){
     int show;
     show=system("dmesg");
     return show;
 }
 close();
 show();
}

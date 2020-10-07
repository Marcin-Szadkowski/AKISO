#include <stdlib.h>
#include <string.h> 

int main()
{
 
 int open(){
  int open;
  open=system("sudo insmod ./hello.ko");
  return open;
 }

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

  open();
  show();
}

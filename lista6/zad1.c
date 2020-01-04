#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define BUFF_SIZE 1024
/**
 * Liczby w hexa wpisywac wielki literami
 * 
 **/ 
int power(int x, int powerRaised);
int strToInt(char *str, int podstawa);
int myScanf (char * format, ...);
void myPrintf(char* format,...);
char *convert(unsigned int num, int base);

/**
 * Funkcja podnosząca x do potęgi powerRaised
 **/
int power(int x, int powerRaised) {
    if (powerRaised != 0)
        return (x*power(x, powerRaised-1));
    else
        return 1;
}
/**
 * Funkcja konwertujaca stringi na liczby sestemow: binarnego, dziesietnego i hexa
 * Funkcja przyjmuje jako parametry: String oraz podstawe systemu liczbowego
 **/
int strToInt(char *str, int podstawa) {
  int result = 0;
  int int_length = 0;
  int isNegative = 0;
  if (str[0] == '-') {
    isNegative = 1;
  }
  int n = isNegative;
  //Liczymy dlugosc stringa
  while(str[n] != '\0' && str[n] != '\n') {
    int_length++;
    n++;
  }
  //Bierzemy po kolei kazdy char ze stringa
  for (int i = (int_length - 1 + isNegative); i >= isNegative; i--) {
    if (str[i] >= 'A' && str[i] <= 'F') {
      result += (str[i] - 'A' + 10) * power(podstawa, int_length - i + isNegative - 1);
    } else {
      result += (str[i] - '0') * power(podstawa, int_length - i + isNegative - 1);
    }
  }
  return result * power(-1, isNegative);
}
/**
 * Funkcja zamieniajaca inta o danej podstawie na stringa
 **/
char *convert(unsigned int liczba, int podstawa) { 
	//Deklarujemy dostepne znaki
	static char representation[]= "0123456789ABCDEF";
	//Deklarujemy buffer, ktory posluzy do wypelnienia danymi w kolejnosci od konca do poczatku
	static char buffer[50]; 
	char *ptr; 
	//Wskaznik teraz wskazuje na ostatni element 
	ptr = &buffer[49]; 
	//Konczymy buffer znakiem oznaczajacym koniec
	*ptr = '\0'; 
	do { 
		//Przesuwamy sie w strone poczatku tablicy buffer i zapisujemy na kolejnych pozycjach cyfry
		*--ptr = representation[liczba%podstawa]; 
		liczba /= podstawa; 
	}while(liczba != 0); 
	//Zwroc wskazanie na pierwszy element
	return(ptr); 
}
int main(int argc, char *argv[]) {

	int i;
	char *s;
	int x;
	int b;

	myPrintf("Podaj int: ");
	myScanf("%d", &i);
	myPrintf("Oto twoj int: %d", i);

	myPrintf("\nPodaj string: ");
	myScanf("%s", &s);
	myPrintf("Twoj string: %s", s);

	myPrintf("\nPodaj liczbe w systemie heksadecymalnym: ");
	myScanf("%x", &x);	
	myPrintf("Twoja liczba w hexa: %x", x);

	myPrintf("\nPodaj liczbe w systemie binarnym: ");
	myScanf("%b", &b);
	myPrintf("Twoja liczba binarna: %b", b);
	myPrintf("\n");

	return 0;
}

/**
 * Implementacja scanf uzywajaca wywolanie systemowe read
 **/ 
int myScanf (char * format, ...) {
	char* p = (char* ) &format + sizeof format;
	while (*format) {
		 if (*format == '%') {
		   format++;
		   switch (*format) {

		     case 'd': {
		     	int *i = (int *)(*(int *)p);
		     	char *str = malloc(BUFF_SIZE);
		     	read(0, str, BUFF_SIZE);
		     	*i = strToInt(str, 10);
		     	p += sizeof(int *);
		     	break;
		     }
        	 case 's': {
		        char **strArg = (char**) (*(char**) p);
		        char *string = malloc(BUFF_SIZE);
		        read(0, string, BUFF_SIZE);   
		        if (string[strlen(string) - 1] == '\n') {
		          	string[strlen(string) - 1] = '\0';
		        }
		        *strArg = string;
		        p += sizeof(*strArg);
		        break;
			 }
	         case 'x': {
		        int *x = (int*)(*(int*)p);
		        char *str = malloc(BUFF_SIZE);
		        read(0, str, BUFF_SIZE);
		        *x = strToInt(str, 16);
		        p += sizeof(int*);
		        break;
			 }
			 case 'b': {
	            int *b = (int*)(*(int*)p);
	            char *str = malloc(BUFF_SIZE);
	            read(0, str, BUFF_SIZE);
	            *b = strToInt(str, 2);
	            p += sizeof(int*);
	          break;
			 }

	 		}
	 		format++;
		} else {
			format++;
		}
	}
	p = NULL;
	return 0;
}
/**
 * Implementacja funkcji printf, korzystajaca z wywolania systemowego write
 **/ 
void myPrintf(char* format,...) {
	char *traverse; 
	int i; 
	char *s; 
	char* p = (char* ) &format + sizeof format;	
	for(traverse = format; *traverse != '\0'; traverse++) { 
		while( *traverse != '%') { 
			if( *traverse != '\0'){
				write(1, traverse, 1);
				traverse++; 
			}else {
				break;
			}			
		}
		if(*traverse != '\0') 		
			traverse++; 
		else 
			break;		
		switch(*traverse) { 				
			case 'd' : i = *((int *)p);
				p += sizeof(int);
				if(i < 0) {
					i = -i;
					char* sign = "-";
					write(1,sign,1);
				}
				write(1, convert(i,10), strlen(convert(i,10)));
				break;  						
			case 's': s = *((char* *)p);
				p += sizeof(char*);
				write(1, s, strlen(s)); 
				break; 						
			case 'x': i = *((int *)p);
				p += sizeof(int);
				write(1, convert(i, 16), strlen(convert(i, 16)));
				break; 
			case 'b': i = *((int *)p);
				p += sizeof(int);
				write(1, convert(i, 2), strlen(convert(i, 2)));
				break;
		}	
	} 
	p = NULL;
} 
 

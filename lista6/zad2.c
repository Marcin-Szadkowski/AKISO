#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

//gcc -o zad2 zad2.c -lpthread

int rowsDone = 0;
pthread_mutex_t muteks=PTHREAD_MUTEX_INITIALIZER;

/**
 * Struktura zwierajaca mnozone macierze,
 * macierz wynikowa oraz rozmiar macierzy
 **/ 
typedef struct {
  int **matrix1;
  int **matrix2;
  int **result;
  int size;
} Matrices;

/**
 * Deklaracje uzywanych funkcji
 **/ 
int ** placeForMatrix(int size);
void matrixFill(int **matrix1, int **matrix2, int size);
void *matrixMultiplication(void *vargp);
void showMatrix(int **matrix, int size);

int main(int argc, char *argv[]) {

  if (argc != 3) {
    printf("Bledne argumenty!\n");
    exit(1);
  }
  struct timespec start, end;
  //Przypisujemy rozmiar macierzy
  int matrixSize = atoi(argv[1]);
  //Przypisujemy liczbe watkow
  int threadsAmount = atoi(argv[2]);
  Matrices matrix;
  //Alokujemy miejsce dla macierzy
  matrix.matrix1 = placeForMatrix(matrixSize);
  matrix.matrix2 = placeForMatrix(matrixSize);
  matrix.result = placeForMatrix(matrixSize);
  matrix.size = matrixSize;

  //Wypelnianie macierzy lsoowymi danymi

  matrixFill(matrix.matrix1, matrix.matrix2, matrixSize);
  
  //Deklarujemy tablice watkow o rozmiarze takiej ile watkow podal uzytkownik
  pthread_t ptIDs[threadsAmount];
  //Pokazujemy macierze
  printf("\nOto twoje macierze: ");
  showMatrix(matrix.matrix1, matrixSize);
  showMatrix(matrix.matrix2, matrixSize);

  printf("Mnozenie...\n");

  //Tworzymy tyle watkow ile wskazal uzytkownik
  //Pierwszy argument o id watku a trzeci do funkcja jaka bedzie watkiem,
  for (int i = 0; i < threadsAmount; i++) {
    if(pthread_create(&ptIDs[i], NULL, matrixMultiplication, &matrix))
      printf("Blad przy tworzeniu watku\n");
     // abort();
  }
  //Czekamy na zakonczenie watkow
  for(int i =0; i <threadsAmount; i++){
    if(pthread_join(ptIDs[i], NULL))
      printf("Blad w oczekiwaniu na watek\n");
  }
  //Pokazujemy macierz wynikowa
  showMatrix(matrix.result, matrixSize);
  //pthread_exit(NULL);
  return 0;
}
/**
 * Funkcja wyswietlajaca macierz
 **/
void showMatrix(int **matrix, int size){
  printf("\n");
  for(int i=0; i< size; i++){
    for(int j=0; j < size; j++){
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
}
/**
 * Funkcja rezerwujaca miejsce dla macierzy
 **/ 
int ** placeForMatrix(int size) {
  int **matrix = (int **)malloc(size * sizeof(int *));
  for (int i = 0; i < size; i++) {
    matrix[i] = (int *)malloc(size * sizeof(int));
  }
  return matrix;
}
/*
 * Funkcja wypelniajaca macierze losowymi danymi
 **/
void matrixFill(int **matrix1, int **matrix2, int size) {
  srand((unsigned)time(NULL));
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      matrix1[i][j] = (rand()%2);
    }
  }
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      matrix2[i][j] = (rand()%2);
    }
  }
}
/**
 * Funkcja uzywana przez watki do mnozenia macierzy
 **/ 
void *matrixMultiplication(void *vargp) {
    Matrices *matrix = (Matrices*)vargp;

  pthread_mutex_lock(&muteks);
  int i = rowsDone;
  rowsDone++;
  pthread_mutex_unlock(&muteks);

  while(i < (*matrix).size){
      for (int j = 0; j < (*matrix).size; j++) {
          int value = 0;
          for (int k = 0; k < (*matrix).size; k++) {
              value |= ((*matrix).matrix1[i][k] & (*matrix).matrix2[k][j]);
              if (value) {
                break;
              }
          }
          //printf("%d ", value);
          (*matrix).result[i][j] = value;      
      }
      //Synchronizacja
      pthread_mutex_lock(&muteks);
      i = rowsDone;
      rowsDone++;
      pthread_mutex_unlock(&muteks);
  }
    return NULL;
}
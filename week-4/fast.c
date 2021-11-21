#include <stdio.h>

#define SIZE 10000

void main(){
    int fast[SIZE][SIZE];
    int num = 0;

    for (int i = 0; i< SIZE;i++){
        for (int j = 0; j < SIZE; j++){
            fast[i][j] = j;
        }
    }

    for (int i = 0; i < SIZE;i++){
        for (int j = 0; j < SIZE; j++){
            num == fast[i][j];
        }
    }
}
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int length = 5;

void buggy_code_3(){
        int i = 0;
        int* arr = malloc(length * sizeof(int));

        for (i = 0; i < length; i++);{
                arr[i] = i*2;
        }
        printf("\nDoubled array last element: %d\n\n", arr[length - 1]);
}

int main(){
        buggy_code_3();

        return 0;
}

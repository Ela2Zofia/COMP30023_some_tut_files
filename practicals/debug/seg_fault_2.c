#include<stdio.h>

int num_loops = 5;

void seg_fault_2(){
        int i = 0;
        int arr[5] = {1, 2, 3, 5, 7};
        for (i = 0; i <= num_loops; i++){
                printf("%d ", arr[i]);
        }
}

int main(){
        seg_fault_2();

        return 0;
}

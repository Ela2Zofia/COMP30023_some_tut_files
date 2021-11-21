#include<stdio.h>

void seg_fault_1(){
        char *x = 0; 
        *x = 3;  
}

int main(){
        seg_fault_1();

        return 0;
}

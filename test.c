#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    FILE *temp = fopen("temp.txt","r");
    printf("%p\n",temp);
    fclose(temp);
}
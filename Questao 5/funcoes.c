#include <stdio.h>
#include<stdlib.h>
#define TAM 10
#define true 1
#define false 0
int Vetor[TAM];

typedef struct par{
    int first,second;
}par;

int print(void* arg){

    int inicio = ((par*)arg)->first;
    int fim = ((par*)arg)->second;
    if(inicio < fim){
        return false;
    }
    for(int i=inicio; i<fim; i++){
        printf("%d ",Vetor[i]);
    }
    printf("\n");
    return true;
}
int minimo(void *arg){

    int inicio = ((par*)arg)->first;
    int fim = ((par*)arg)->second;
    
    int min = Vetor[0];
    for(int i=inicio; i < fim; i++){
        min = min > Vetor[i] ? Vetor[i] : min;
    }
    return min;
}

int maximo(void *arg){
    int inicio = ((par*)arg)->first;
    int fim = ((par*)arg)->second;
    
    int max = Vetor[0];
    for(int i=inicio; i < fim; i++){
        max = max < Vetor[i] ? Vetor[i] : max;
    }
    return max;
}


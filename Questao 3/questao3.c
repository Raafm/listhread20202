#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <math.h>

typedef struct Estatisticas_sala{
    int Media ;
    int Moda ;
    int Mediana ;
    int desvio;
}Estatisticas_sala;


Estatisticas_sala* estatistica;
int N_salas; //numero de arquivos
int salas_vistas=0;
int Nthreads;
pthread_mutex_t mutex_sala = PTHREAD_MUTEX_INITIALIZER; // para a sala


//funcao auxiliar para o qsort
float comparadora (const void * a, const void * b){
    return ( *(float*)a - *(float*)b );
}


//funcoes matematicas para as estatisticas de cada sala. Vale lembrar que nota[N] esta sortido (passou pelo quick sort antes das funcoes serem chamadas)
float media(float* nota,int N){
    float soma = 0;
    for(int i =0; i < N; i++){
        soma+=nota[i];
    }
    return (float)soma/N;
}

float moda(float* nota,int N){
    int i =0;
    int mais_frequente;
    int freq_max = -1;
    int atual;
    int frequencia;
    while(i < N){

        atual = nota[i];
        frequencia=0;
        while(i < N && nota[i] == atual){
            frequencia++;
            i++;
        }
        if ( freq_max < frequencia ){
            mais_frequente = atual;
            freq_max =frequencia;
        }
    }
    return mais_frequente;

}  

float mediana(float*nota, int N){
    if( N%2 == 0){
        return nota[N/2];
    }
    return (nota[N/2]+nota[N/2+1])/2;
}

float desvio(float*nota, int N){

    float soma =0;
    int M = media(nota,N);
    for(int i=0; i<N; i++){
        soma+=(nota[i]-M)*(nota[i]-M);
    }
    return sqrt(soma/N);
}



void ler_arquivo(int num_sala, float* nota,int* tam,int* top){

    char salArquivo[10];

    sprintf(salArquivo, "sala%d.txt", num_sala);

    //variaveis para fazer o recolhimento dos dados do arquivo
    FILE *sala;         //
    char linha[100];    //    receber informacao da linha do arquivo
    char aluno[100];    //    receber nome do aluno
    int eof = 0;        // 
    
    sala = fopen(salArquivo, "r");
    while(!eof){ 

        for(int i = 0; i < *tam; i++ ){

            if(fgets(linha, 100, sala) != NULL){
                sscanf(linha, "%s %d", aluno , &nota[(*top)++]);  
            }
            else eof = 1;
        }

        if(!eof){       //ainda tem linha para ler com notas, mas acabou o espaco, dobro o tamanho e continuo
            (*tam) *= 2;
            nota = realloc(nota, (*tam)*sizeof(nota));
        }
    }
    fclose(sala);
}

void operacoes(float * nota, int tam, int num_sala){
        qsort (nota, tam, sizeof(int), comparadora);

        estatistica[num_sala].Media    =   media(nota,tam);
        estatistica[num_sala].Moda     =   moda(nota,tam);
        estatistica[num_sala].Mediana  =   mediana(nota,tam);
        estatistica[num_sala].desvio   =   desvio(nota,tam);
}

void consultar_sala(void* arg){

    while(1){

        // escolhi a abordagem de ler os arquivos sob demanda
        pthread_mutex_lock(mutex_sala);
        if(salas_vistas == N_salas){return NULL;}       //ja vimos todas as salas
            
        int num_sala = salas_vistas; 
        salas_vistas++;
        pthread_mutex_unlock(mutex_sala);

     
        float* nota = (float*)malloc(sizeof(float)*20); //vetor de notas para depois tirar media, mediana, etc
        if(nota == NULL){
            printf("\nMemory Error when allocating estatistica\n");
            exit(-1);
        }
        int tam = 20;   //variavel auxiliar para o vetor de notas
        int top = 0;    //variavel auxiliar para o vetor de notas

        ler_arquivo(num_sala,nota,&tam,&top);//tam e top sao passados por referencias pois sao alterados na funcao

        operacoes(nota,tam,num_sala);
 
        free(nota);
    }
}


int main(){

    printf("numero de salas: ");
    scanf("%d", &N_salas);
    printf("numero de threads: ");
    scanf("%d", &Nthreads);
    
    estatistica = (int*)malloc(N_salas* sizeof(Estatisticas_sala));



    pthread_t* threads = (pthread_t*)malloc(Nthreads*sizeof(pthread_t));
    

    for(int sala_Vazia = 0; sala_Vazia < Nthreads; sala_Vazia++){

        pthread_create(&threads[sala_Vazia], NULL,consultar_sala,NULL);
    }

    for(int i = 0; i < Nthreads; i++)
        pthread_join(threads[i], NULL);

    for(int i =0; i < N_salas; i++){
        printf("Sala X\nMedia: %f\nModa: %f\nMediana: %f\nDesvio Padrão: %f\n\n ");
    }
    return 0;
}


#include<stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define true 1
#define false 0

#define TAM 1    //tamanho do array (SGBD)


int M,N;                            // threads N leitoras e M escritoras
pthread_mutex_t mutex[TAM];         // mutex para cada posicao 
pthread_cond_t ocupado[TAM];        // variavel de condicao para cada posicao
int escrevendo[TAM];                // indica se a posicao esta ocupada ou nao para leitura (quantas threads estao querendo escrever/escrevendo)
pthread_t* escritora ;              // array de threads escritoras
pthread_t* leitora  ;               // array de threads leitoras
pthread_mutex_t* mutex_aux;         // mutex auxiliares para threads leitoras

int* id_escritora;                  // array de identificadores para threads escritoras
int* id_leitora ;                   // array de identificadores para threads leitoras

int SGBD[TAM];                      



//LER:
// se a tem thread escritora escrevendo, espero.
// se nao tem thread escrevendo, sigo.

void* ler(void* arg){

    int self_id = *((int*)arg);


    while(true){
        int posicao = rand() %TAM;

        pthread_mutex_trylock(&mutex_aux[self_id]);     // travei meu proprio mutex (sem efeito em nenhuma thread)                   
        while(escrevendo[posicao]>0){                      // se nao tem thread escrevendo ou querendo escrever, leio. Se tem thread escrevendo ou querendo escrever, espero
            pthread_cond_wait(&ocupado[posicao],&mutex_aux[self_id]);
        }

        int valor = SGBD[posicao];                      //le
        pthread_mutex_unlock(&mutex_aux[self_id]);      // este mutex nao bloqueia ninguem , uma vez que esta relacionada apenas com esta thread

    }

    return NULL;
}

//ESCREVER:
// travo mutex, ou seja, travo outras escritoras 
// travo as leitoras
// escrevo
// destravo todos

void* escrever(void* arg){  //arg aponta para id da thread
    
    int self_id = *((int*)arg);


    while(true){

        int posicao = rand() %TAM;
        int valor   = rand()%10;
        escrevendo[posicao] += 1;                   // mais um thread querendo escrever, travei leitoras

        pthread_mutex_lock(&mutex[posicao]);        // travei threads escritoras
        

        SGBD[posicao] = valor;                      // escrevo

  
        escrevendo[posicao] -= 1;

        pthread_mutex_unlock(&mutex[posicao]);  
        if(escrevendo[posicao] == false){
            pthread_cond_broadcast(&ocupado[posicao]);
        }
        printf("\n");
        sleep(1);
    }
    return NULL;
}

void input(){

    scanf(" %d %d",&M,&N);
    
}

void prepara(){

    escritora  = (pthread_t*)malloc(M*sizeof(pthread_t));
    leitora   = (pthread_t*)malloc(N*sizeof(pthread_t));
    mutex_aux = (pthread_mutex_t*)malloc(N*sizeof(pthread_mutex_t));

    for(int i=0; i < TAM; i++){
        escrevendo[i] = false; //ninguem esta escrevendo
        SGBD[i] = i; //coloca qualquer coisa no SGBD previamente
    }

    id_escritora= (int*)malloc(M*sizeof(int));
    id_leitora = (int*)malloc(N*sizeof(int));

}

int main(){
    srand((unsigned) time(NULL));
    input();

    prepara();

    // cria threads e faz join
    for(int i =0; i < M; i++){
        id_escritora[i] = i;
        pthread_create(&escritora[i],NULL,escrever,&id_escritora[i]);
    }
    for(int i = 0; i < N; i++){
        id_leitora[i] = i;
        pthread_create(&leitora[i],NULL,ler, &id_leitora[i]);
    }
    for(int i =0; i < M; i++){
        pthread_join(escritora[i],NULL);
    }
    for(int i = 0; i < N; i++){
        pthread_join(leitora[i],NULL);
    }    

    free(escritora);
    free(leitora);
    free(mutex_aux);
    free(id_escritora);
    free(id_leitora);
    return 0;
}
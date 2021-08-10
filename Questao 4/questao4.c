#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include<time.h>


pthread_mutex_t* mutex;
pthread_cond_t* condition;

int* SGBD;

pthread_t* leitora;
pthread_t* escritora;
int* escrevendo;
int* lendo;

int N,M,TAM;


void* ler(void *arg){

    while(1){
        int pos = rand()%TAM;
        
        
        while(escrevendo[pos]){                          // enquanto tem uma escritora escrevendo esperamos
            pthread_cond_wait(&condition[pos],&mutex[pos]);
        }

        lendo[pos] +=1;                                  // uma leitora a mais
        pthread_mutex_trylock(&mutex[pos]);              // lock as threads escritoras, mas nao as leitoras
        printf("lendo:SGBD[%d]: %d\n",pos, SGBD[pos]);   //
        pthread_mutex_unlock(&mutex[pos]);               // unlock
        lendo[pos] -=1;
    }

    return NULL;
}

void *escrever(void *arg){

    while(1){
        int pos = rand()%TAM;

        //lock outras escritoras, ou espera outras threads terminarem.
        pthread_mutex_lock(&mutex[pos]);


        while(escrevendo[pos] || lendo[pos]){               //enquanto tem outra thread, espera
            pthread_cond_wait(&condition[pos],&mutex[pos]);
        }
        escrevendo[pos] = 1; //thread ocupada

        // escreve
        SGBD[pos] = rand()%TAM;                 
        printf("escrevendo: SGBD[%d]=%d\n",pos,SGBD[pos]);

        pthread_mutex_unlock(&mutex[pos]);          // unlock escritoras
        escrevendo[pos] = 0;                        // wakeup escritoras e leitoras
        pthread_cond_broadcast(&condition[pos]);    // wakeup escritoras

    }
    return NULL;
}

void input(){
    printf("numero de threads leitoras: ");
    scanf("%d",&N);
    printf("numero de threads: escritoras: ");
    scanf("%d",&M);
    printf("numero de posicoes no array(SGBD): ");
    scanf("%d",&TAM);    
}

int main() {

    srand(time(NULL));

    input();
    
    leitora   = (pthread_t*)malloc(sizeof(pthread_t)*N);
    escritora = (pthread_t*)malloc(sizeof(pthread_t)*M)

    mutex     = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*TAM);
    condition = ( pthread_cond_t*)malloc(sizeof(pthread_cond_t)*TAM);

    escrevendo = (int*)malloc(sizeof(int)*TAM);
    lendo      = (int*)malloc(sizeof(int)*TAM);

    SGBD = (int*)malloc(sizeof(int)*TAM);

    //criar threads
    for(int i =0; i < N; i++ ){
        pthread_create(&leitora[i], NULL, ler,NULL);
    }
    for(int i =0; i < M; i++ ){
        pthread_create(&escritora[i], NULL, escrever,NULL);
    }
  

    //join nas threas
    for(int i =0; i < N; i++){
        pthread_join(leitora[i],NULL);
    }
    for(int i =0; i < M ; i++){
        pthread_join(escritora[i],NULL);
    }

    free(escrevendo);
    free(lendo);
    free(mutex);
    free(condition);
    free(escritora);
    free(leitora);
    free(SGBD);
        
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
 


pthread_mutex_t mtx_intersecao[6] = {PTHREAD_MUTEX_INITIALIZER,
PTHREAD_MUTEX_INITIALIZER,
PTHREAD_MUTEX_INITIALIZER,
PTHREAD_MUTEX_INITIALIZER,
PTHREAD_MUTEX_INITIALIZER,
PTHREAD_MUTEX_INITIALIZER};
pthread_cond_t cond_intersecao[6] ={PTHREAD_COND_INITIALIZER,
PTHREAD_COND_INITIALIZER,
PTHREAD_COND_INITIALIZER,
PTHREAD_COND_INITIALIZER,
PTHREAD_COND_INITIALIZER,
PTHREAD_COND_INITIALIZER};
int intersecao[6] = {0,0,0,0,0,0};
int TRENS =10;







void *Trem( void *arg) // onde as threads(trens) acessam as interserções 
{

    int id = *((int*)arg);
    int atual = 1;
    printf("\ninciando trem %d\n\n",id);
         //intersecao que o trem esta agora

   while (1)
   {     
        // regiao critica
        pthread_mutex_lock(&mtx_intersecao[atual]); 
        

        while (intersecao[atual]==2)
        { 
            pthread_cond_wait(&cond_intersecao[atual],&mtx_intersecao[atual]);  //libera o mutex
        }

       
        intersecao[atual]++;

        pthread_mutex_unlock(&mtx_intersecao[atual]);
        // fim regiao critica
        
        
        printf("Trem : %d na intersecao: %d\n", id, atual);
        usleep(500000);
        intersecao[atual]--;               
        pthread_cond_signal(&cond_intersecao[atual]);
        atual = ( (atual == 5 ) ?  1 : atual + 1); 
        
       
    }

    printf("fim da thread %d\n",id);
    return NULL;

    
}


int main()
{

    pthread_t threads[TRENS];

    int id_trem[TRENS];

        
    for(int i = 0; i < TRENS; i++) {
        id_trem[i]=i+1;
        pthread_create(&threads[i], NULL, Trem,&id_trem[i]);
    }

    for (int i = 0; i < TRENS; i++){
        printf("join %d\n",i);
       pthread_join(threads[i], NULL);
       printf("i =%d\n",i);
    }
     
       
   return 0;
} 

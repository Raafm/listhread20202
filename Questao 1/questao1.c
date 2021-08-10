/* Esta nao e a versao user friendly, rodar ela pode fazer com que nao seja observadas claramente detalhes do codigo no terminal,
pois ela foi feita para ser eficiente e nao vaoltada principalmente para I/O.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define false 0
#define true 1

int contador = 5000000;
int terminou = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void* decrementar(void* arg){

    printf("thread%d criada\n\n\n",*((int*)arg));
 
    //loop externo para manter a thread ativa (so termina quando contador == 0)
    while(contador){    // se o contador ja foi zerado, a thread termina
        
        pthread_mutex_lock(&mutex);

        // loop interno para decremento
        while(contador){             
            contador--;
        }
        
        pthread_mutex_unlock(&mutex);
        
        
        if(terminou)return NULL;
        if(contador == 0) {
            terminou= true;
            printf("\nterminado com a thread %d\n",*((int*)arg));
        }      
    }


    return NULL;
}

int main() {
    
    //criando 2 threads
    pthread_t dec1,dec2;
    int num1=1,num2=2;
    pthread_create(&dec1,NULL,decrementar,&num1); 
    pthread_create(&dec2,NULL,decrementar,&num2);

    //terminar main somente depois das threads
    pthread_join(dec1,NULL);
    pthread_join(dec2,NULL);

    printf("contador = %d\n", contador);
  return 0;
}
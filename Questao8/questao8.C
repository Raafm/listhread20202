/* 
Esta versao nao e a user friendly, recomendo ver a user friendly primeiro, pois tem mais explicacoes, e o codigo e mais adequado para ser executado.
Este nao tem tanta interacao com I/O.
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>


#define MILHAO 1000000
#define false 0
#define true 1


int T;
int* em_execucao;// vector de primos sob os quais as threads estao operando
int* e_primo;

pthread_mutex_t mutex_prox_primo,mutex_aux;
pthread_cond_t crivo_pronto;

int num_primo=0;
int primoN=1;               //primo de numero N. O valor comeca como 1, mas deve ser ignorado serve somente par primeira thread
int N;                      // numero de primos que faltam ver
int threads_prontas = 0;


int prox_primo(int ultimo_primo,int thread_id){
    
    for(int i = ultimo_primo+1 ; i < MILHAO ; i++){ //comecamos a busca pelo primeiro numero depois do ultimo primo
        
        if( e_primo[i] ){ 
        
            int outra_thread = false;   //supomos que nenhuma outra thread cuidara deste elemento
            for(int j = 0; (j < T) && outra_thread == 0; j++){
                if(em_execucao[j]){
                    if( (i%em_execucao[j]) == false) outra_thread = true;  //outra thread cuidara desse elemento
                }
            }

            if (outra_thread == false){  //se nenhuma outra thread for eliminar o elemento, temos o proximo primo
                N--;
                num_primo += 1;
                em_execucao[thread_id] = i;
                return i;
            }
        }    
    }
    return 0;
}

void elimina_multiplos(int primo){
    
    for(int multiplo = 2*primo; multiplo < MILHAO; multiplo+= primo){

        e_primo[multiplo] = false;
    }
}

void* crivo(void*arg){
    
    pthread_mutex_trylock(&mutex_aux);
    int id = *((int*)arg);
    int primoLocal; //serve para guardar a informacao do primo que a thread esta tratando
     
    threads_prontas++;
    pthread_cond_wait(&crivo_pronto,&mutex_aux);   //espera vetor booleano de primos ficar pronto
    
    if(threads_prontas == T)threads_prontas++;     // parar a thread prepara_crivo()
    
    pthread_mutex_unlock(&mutex_aux);


    while( N ){

        //PRIMEIRO PASSO: PROCURAR PROXIMO PRIMO
        pthread_mutex_lock(&mutex_prox_primo);
  
        
        if(N == 0){         //ja encontramos o N-esimo primo

            pthread_mutex_unlock(&mutex_prox_primo);
            break;
        }
        
        primoLocal = primoN = prox_primo(primoN,id);       //para as outras threads poderem atualizar primoN
        
        printf("primo %d): %d\n",num_primo,primoN);


        //SEGUNDO PASSO: CHECA SE TERMINAMOS
        if( N == 0 || primoN == 0 ) {

            printf("N-esimo primo: %d\n",primoN);
            pthread_mutex_unlock(&mutex_prox_primo);
            break;    
        }

        pthread_mutex_unlock(&mutex_prox_primo);

        //TERCEIRO PASSO: ELIMINAR OS MULTIPLOS DO PRIMO
        elimina_multiplos(primoLocal);

    }

    return  NULL;
}

void* prepara_crivo(){

    e_primo = malloc(sizeof(int)*MILHAO);
    for(int i = 2; i < MILHAO; i++){
        e_primo[i] = true;
    }

    while(threads_prontas <= T){

        pthread_cond_broadcast(&crivo_pronto);
    }
    return NULL;
}

int main(){

    scanf(" %d",&T);//numero de threads

     //perparamos o crivo enquanto preparamos outras threads
    pthread_t prepara; //como I/O demora um pouco, usamos esse tempo para ja ir preparando o crivo
    pthread_create(&prepara, NULL, prepara_crivo, NULL);

    scanf(" %d",&N); //numero de primos que quer

    em_execucao = (int*)calloc(T, sizeof(int)); //inicializo todos como 0

    pthread_t thread[T];
    int id[T];

    //criar threads:
    for(int i = 0; i < T; i++){
        id[i]=i;
        pthread_create(&thread[i], NULL,crivo,&id[i]);
    }

    //join
    for(int i = 0; i < T; i++){
        pthread_join(thread[i], NULL);
    }

    free(e_primo);
    free(em_execucao);
    
    return 0;
}
// User Friendly version
/* Seguem duas curtas animacoes (autoria de Rodirgo Abreu, exceto a composicao e execucao das musicas) ilustrando os conceitos abaixo.
Em ambas temos um enorme conjunto de numeros representados por quadrados, eles sao coloridos de acordo com o que foi feito nos algoritmos.

A maior (3 minutos aproximadamente) delas compara o crivo ('Sieve' em ingles) para achar os primos ate 8400 com a abordagem de dividir por todos os numeros ate a raiz
para achar todos os primos ate 2100 (4 vezes menos que o crivo).
Nela, durante a execucao da abordagem de testar todos (ate a raiz): amarelo == numero nao visto, verde == primo,    branco == numero nao primo
Porem, durante a abordagem do crivo, azul significa que foi marcado provisoriamente no array do crivo (e_primo) como verdade (seguindo a logica do algoritmo),
vermelho == foi eliminado no algoritmo do crivo, preto == nao e primo, branco ==numero primo


A menor animacao mostra a atucao conjunta de 4 threads para fazer as contas do crivo.
Nesse caso: branco == primo no array e_primo (no algoritmo supostamente todos comecam como primos), preto ou cinza == nao primo, qualquer outra cor == eliminado
No final do algoritmo eu passei linearmente pelo array coletando todos os primos dele (deixando de branco quem realmente era e pintando de cinza quem nao era)

*/
/*
    LOGICA DO CRIVO:

    1) criamos um array de bool com o maximo de elementos que pudermos. Chamemos o array de: e_primo,
    ou seja, e_primo[elemento] == true significa que o elemento e primo, false significa o contrario.

    2) consideramos, a principio, todos os elementos como primos. Fazemos um loop com: e_primo[i] = true.

    3) depois procuramos (comecando em 2) o primeiro primo.

    4) ao achar um primo (P) iteramos sobre todos os seus multiplos: e_primo[k*P] = false,
    todos nao sao multiplos.

    Com isso, o array contera corretamente a informacao se cada elemento e ou nao primo.
    Temos aqui um limite de espaco em compensacao pelo aumento de eficiencia. Como o array tem limite, temos um numero maximo de primos
    (No caso: 78498 primos). Segue foto de codigo mostrando isso. 

    LOGICA QUE USEI:

    1) pego a infrormacao de quantas threads poderei fazer.
    2) enquanto o user toma seu tempo para digitar o numero do primo, crio uma thread para fazer o loop que prepara o array.
    depois que a thread termina de criar o array, ela libera as threads que farao as contas.
    3) depois de criar a thread para preparar o array, criamos as threads que farao as contas do crivo (que ficam esperando o array).
    4) uma so thread procura o proximo primo ainda nao avaliado
    5) a thread que achou o proximo primo inicia o processo de eliminar os multiplos.
    6) outra thread procura o proximo primo, e faz o mesmo que a anterior.

    Vemos que nao teremos problema com achar qual sera o N-esimo primo, pois ao encontrar o primo estamos na regio critica.

    Para escolher o proximo primo, precisamos ver se o elemento que encontramos e um "falso primo", isto e,
    o elemento nao e primo, mas ainda nao foi marcado como tal pela thread correspondente.
    Para resolver esse caso, guardamos num array (*em_exercicio) qual primo cada thread esta procurando,
    e vemos se ele eliminara posteriormente tal "falso primo".
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>


#define MILHAO 1000000
#define false 0
#define true 1


int T;                  // numero de threads
int* em_execucao;       // vector de primos sob os quais as threads estao operando
int* e_primo;           // crivo

pthread_mutex_t mutex_prox_primo , mutex_aux; // mutex_aux e apenas auxiliar. mutex_prox_primo serve para gerenciar a regiao critica
pthread_cond_t crivo_pronto;                  // serve para indicar que o crivo esta pronto          

int num_primo = 0;
int primoN = 1;               // primo de numero N. O valor comeca como 1, mas deve ser ignorado serve somente par primeira thread
int N;                        // numero de primos que faltam ver
int threads_prontas = 0;      // numero de threads de contas que estao prontas para tal.


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
    
    printf("criada thread %d\n",id);    
    threads_prontas++;
    pthread_cond_wait(&crivo_pronto,&mutex_aux);   //espera vetor booleano de primos ficar pronto
    
    if(threads_prontas == T)threads_prontas++;     // parar a thread prepara_crivo()
    
    pthread_mutex_unlock(&mutex_aux);
    printf("iniciando thread %d\n",id);

    while( N ){

        printf("thread %d trabalhando\n",id);
        sleep(2);//user friendly

        //PRIMEIRO PASSO: PROCURAR PROXIMO PRIMO
        pthread_mutex_lock(&mutex_prox_primo);
        printf("thread %d com mutex\n",id);
        
        if(N == 0){         //ja encontramos o N-esimo primo
            printf("thread %d liberando mutex\n",id);
            pthread_mutex_unlock(&mutex_prox_primo);
            break;
        }
        
        primoLocal = primoN = prox_primo(primoN,id);       //para as outras threads poderem atualizar primoN
        
        printf("primo %d): %d\n",num_primo,primoN);


        //SEGUNDO PASSO: CHECA SE TERMINAMOS
        if( N == 0 || primoN == 0 ) {

            printf("N-esimo primo: %d\n",primoN);
            printf("thread %d liberando mutex\n",id);
            pthread_mutex_unlock(&mutex_prox_primo);
            break;    
        }
        printf("thread %d liberando mutex\n",id);
        pthread_mutex_unlock(&mutex_prox_primo);

        //TERCEIRO PASSO: ELIMINAR OS MULTIPLOS DO PRIMO
        elimina_multiplos(primoLocal);

    }
    printf("terminando thread: %d\n",id);
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

    printf("numero de threads: ");
    scanf("%d",&T);

     //perparamos o crivo enquanto preparamos outras threads
    pthread_t prepara; //como I/O demora um pouco, usamos esse tempo para ja ir preparando o crivo
    pthread_create(&prepara, NULL, prepara_crivo, NULL);

    printf("qual o primo: ");
    scanf("%d",&N);

    em_execucao = (int*)calloc(T, sizeof(int)); //inicializo todos como 0

    pthread_t thread[T];
    int id[T];

    //criar threads:
    for(int i = 0; i < T; i++){
        id[i]=i;
        sleep(1);
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
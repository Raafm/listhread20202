#include<stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include<unistd.h>

#define true 1
#define false 0

#define TAM 10    //tamanho do array (SGBD)

srand((unsigned) time(NULL));
int M,N;
pthread_mutex_t mutex[TAM];
pthread_cond_t ocupado[TAM];

pthread_t* escritor ;
pthread_t* leitora  ;
pthread_mutex_t* mutex_aux;

int* id_escritora[M];
int* id_leitora[N]  ;

int SGBD[TAM];    

typedef struct Nodes {

    Node* next;
    int id;
}Node;

typedef struct linked_list{
    Node* head;
    Node* tail;
}linked_list;

linked_list fila_espera[TAM];

int empty(int posicao){
    return fila_espera[posicao].head == NULL;
}

void inserir(int posicao,int ID){   //insere na frente
    
    if(fila_espera[posicao].head == NULL){ // se esta vazio
        fila_espera[posicao].head = fila_espera[posicao].tail = (Node*)malloc(sizeof(Node));
    }
    else{ // se nao esta vazio
        fila_espera[posicao].tail->next = (Node*)malloc(sizeof(Node));
        fila_espera[posicao].tail = fila_espera[posicao].tail->next;  
    }
    fila_espera[id].tail->id = ID;
    fila_espera[id].tail->next = NULL; // serve para sabermos quando estiver vazio
}

void remover(int posicao){
    if(fila_espera[posicao].head == NULL){  //se esta vazio retorna
        return;
    }
    fila_espera[posicao].head = fila_espera[posicao].head->next;
}

int id_front(int posicao){
    return  fila_espera[posicao].head->id;
}

//LER:
// criar fila para cada posicao,
// se a fila esta vazia (condicao), pode ler.
// se a fila nao esta vazia continua escrevendo ate o fim

void* ler(void* arg){

    int self_id = *((int*)arg);

    while(true){
        int posicao = rand() %TAM;

        pthread_mutex_trylock(&mutex_aux[self_id]);     // travei meu proprio mutex (antes ate de me inserir na fila)
        inserir(posicao,self_id);                       // inseri-me na fila de espera da posicao do SGBD 
        while(false == empty(posicao)){                          // se nao estou na vez, espero. Se estou na vez continuo
            pthread_cond_wait(&ocupado[posicao],&mutex_aux[self_id]);
        }
        int valor = SGBD[posicao];                      //le
        pthread_mutex_unlock(&mutex_aux[self_id]);      // este mutex nao bloqueia ninguem , uma vez que esta relacionado com apenas esta thread
        printf("na posicao: %d, lido: %d\n",posicao,valor);
    }

    return NULL;
}

//ESCREVER:
// travo mutex, coloco-me na lista de espera para escrever na posicao correspondente
// se sou a cabeca da lista, escrevo e acordo todos
// se nao sou a cabeca da lista , espero

void* escrever(void* arg){  //arg aponta para id da thread
    
    int self_id = *((int*)arg);

    while(true){

        int posicao = rand() %TAM;
        int valor   = rand() %TAM;

        pthread_mutex_lock(&mutex[posicao]);        // travei mutex (antes ate de me inserir na fila)
        inserir(posicao,self_id);                   // inseri-me na fila de espera da posicao do SGBD 
        while(self_id != id_front(posicao)){        // se nao estou na vez, espero. Se estou na vez continuo
            pthread_cond_wait(&ocupado[posicao],&mutex[posicao]);
        }
        SGBD[posicao] = valor;                      // escrevo
        remover(posicao);                           // removo da fila
        pthread_mutex_unlock(&mutex[posicao]);     
        pthread_cond_broadcast(&ocupado[posicao]);
    }
    return NULL;
}

void input(){

    printf("Numero de threads escritoras (M): ");
    scanf(" %d",&M);
    printf("Numero de threads leitoras (N): ");
    scanf(" %d",&N);    
}

void prepara(){

    escritor  = (pthread_t*)malloc(M*sizeof(pthread_t));
    leitora   = (pthread_t*)malloc(N*sizeof(pthread_t));
    mutex_aux = (pthread_mutex_t*)malloc(N*sizeof(pthread_mutex_t));

    for(int i=0; i < TAM; i++){
        fila_espera[i].tail = fila_espera[i].head = NULL;
    }

    id_escritora[M]= (int*)malloc(M*sizeof(int));
    id_leitora[N]  = (int*)malloc(N*sizeof(int));
}

int main(){

    input();

    prepara();

    // cria threads e faz join
    for(int i =0; i < M; i++){
        id_escritora[i] = i;
        pthread_create(&escritora[i],NULL,escrever,&id_escritora[i]);
    }
    for(int i = 0; i < N; i++){
        id_leitora[i] = NULL;
        pthread_create(&leitora[i],NULL,ler, &id_leitora[i]);
    }
    for(int i =0; i < M; i++){
        pthread_join(&escritora[i],NULL);
    }
    for(int i = 0; i < N; i++){
        pthread_join(&leitora[i],NULL);
    }    

    free(escritor);
    free(leitora);
    free(mutex_aux);
    free(id_escritora);
    free(id_leitora);
    return 0;
}
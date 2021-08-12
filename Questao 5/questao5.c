#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


//================================================================ INICIO DA API =================================================================

#define true 1
#define false 0
#define N 5                             // numero maximo de threads em execucao

int executando = 0;                     // numero de threads em execucao
pthread_cond_t despache_ok;             // variavel de condicao relacionada a thread despachante
pthread_mutex_t mutex_despachante;      // mutex relacionado a thread despachante
pthread_t despache;                     //thread de despache

pthread_mutex_t inserir_resposta;       // mutex para exclusao mutua para produzir na fila de respostas     
pthread_mutex_t espera_resultado;       // mutex para exclusao mutua para consumir na fila de resposta

pthread_mutex_t espera_agendamento;     // mutex para exclusao mutua na fila de tarefas agendadas 



 // -------------------------------------------------------   estruturas de dados auxiliares  -------------------------------------------------------


                                                            //linked list

typedef struct Node {

    struct Node* next;
    struct Node* prev;

    void*funcao;
    void*arg;
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    int* resposta;
}Node;


void Unlink(Node* node1,Node* node2){
    if(node1)node1->next = NULL;
    if(node2)node2->prev = NULL;
}

void Link(Node* node1,Node* node2){
    if(node1)node1->next = node2;
    if(node2)node2->prev = node1;
}

typedef struct linked_list{
    Node* head;
    Node* tail;
}linked_list;


int empty(linked_list* fila){
    return (*fila).head == NULL;
}

void inserir(linked_list* fila, Node*new_node){   //insere na frente
    
    if((*fila).head == NULL){ // se esta vazio
        (*fila).head = (*fila).tail = new_node;
    }
    else{ // se nao esta vazio
        Link((*fila).tail , new_node);
        (*fila).tail = (*fila).tail->next;  
    }
    (*fila).tail->next = NULL; // serve para sabermos quando estiver vazio
}

Node* remover(linked_list* fila){//remeove de tras
    if((*fila).head == NULL){  //se esta vazio retorna
        return NULL;
    }
    Node* retornado = (*fila).head;
    (*fila).head = (*fila).head->next;
    Unlink(retornado,(*fila).head); //evitar problemas, quebrar o vinculo coma linked list
    return retornado; //nao se esquecer de dar free depois de usar
}

Node* id_front(linked_list* fila){  // retorna id do head da list
    return  (*fila).head;
}
Node* id_back(linked_list* fila){ // retorna id do back da list
    return (*fila).tail;
}

void print_list(linked_list* fila){
    Node* temp = (*fila).head;
    while(temp != NULL){
        printf("%d ",(int)temp);
        temp = temp->next;
    }
    printf("\n");
}

void apagar_fila(linked_list* fila){
    Node* temp = remover(fila);
    while(temp != NULL){
        free(temp);
        temp = remover(fila);
    }
    printf("fila apagada\n");
}


linked_list lista_espera;           //duas linked_lists criadas
linked_list lista_resposta;







typedef struct par{
    int first,second;
}par;

void* pointer_pair(int x, int y){
    struct par* P;
    P->first = x;
    P->second = y;
    return (void*)P;
}



int somar(par P);
int subtrair(par P);
int multiplicar(par P);
int dividir(par P);

int (*operacao[4])(par P);           //array de ponteiro para as funcoes





//-----------------------------------------------------------------    funcoes da interface da API    ---------------------------------------------------------

Node* agendarExecucao(void* funexec, void *arg){

    Node* new_node = (Node*)malloc(sizeof(Node));

    pthread_mutex_t personal_mutex;
    pthread_cond_t personal_cond;

    new_node->funcao = funexec;
    new_node->arg = arg;
    
    new_node->mutex = &personal_mutex;
    new_node->cond = &personal_cond;
    new_node->resposta = NULL;

    pthread_mutex_lock(&espera_agendamento);
    inserir(&lista_espera,new_node);
    pthread_mutex_unlock(&espera_agendamento);

    pthread_cond_signal(&despache_ok); //acorda despachante

    Node* id = new_node;
    return id;
}






                                             

int pegarResultadoExecucao(Node* id){//ninguem disse que o ID tem que ser um numero sequencial
    
    pthread_mutex_t* personal_mutex = id->mutex;
    pthread_cond_t* personal_cond = id->cond;
    int resposta;
    while(true){

        pthread_mutex_lock(&espera_resultado); //tenta pegar resultado
        pthread_mutex_lock(personal_mutex);
        
        if (id->resposta){
            resposta = *(id->resposta);
            pthread_mutex_unlock(&espera_resultado);
            free(id); //finalmente o Node pode ser apagado
            return resposta;
        }
        else{
            pthread_mutex_unlock(&espera_resultado);  //unlock para outros tentarem pegar 
            pthread_cond_wait(personal_cond,personal_mutex);//acordem esta thread quando terminar.
            //colocar VARIAVEL DE CONDICAO UNICA PARA CADA THREAD QUE ESPERA DENTRO DA LINKED LIST
            //QUANDO CHEGAR NA RESPOSTA, ACORDA PRECISAMENTE A QUE DORMIU 

        }

    }

}






                                                            // thread despachante

void* threadespaxe(void* primeiro){

    int(*funcexec)(par) = ((struct Node*)primeiro)->funcao;                       // pega funcao
    void* argumento = (((struct Node*)primeiro)->arg);
    ((struct Node*)primeiro)->resposta = (int*)malloc(sizeof(int));
    *(((struct Node*)primeiro)->resposta) = funcexec(  *((par*)argumento)  );                  // executa a funcao e guarda ela no Node

    pthread_mutex_lock(&inserir_resposta);                 // exclusao mutua na regiao critica
    inserir(&lista_resposta,primeiro);                      // insere na lista de resposta
    pthread_mutex_unlock(&inserir_resposta);

    pthread_cond_signal(((struct Node*)primeiro)->cond);    //acorda apenas a thread do usuario que fez esta requisicao
    
    executando--;
    pthread_cond_signal(&despache_ok);
    return NULL;
}



void* despachar(void*arg){
                              //numeros de threads sendo executadas
    
    while(true){

        pthread_mutex_lock(&mutex_despachante);

        while(executando == N || empty(&lista_espera)){
            pthread_cond_wait(&despache_ok,&mutex_despachante);
        }
        //pega da lista
        executando++;
        Node*primeiro = remover(&lista_espera);
        pthread_mutex_unlock(&mutex_despachante);

        pthread_t thread;
        pthread_create(&thread,NULL,threadespaxe,primeiro);


    }
    return NULL;
}


// fim thread despachante

void inicia_API(){
    lista_espera.head = lista_espera.tail = lista_resposta.head = lista_resposta.tail = NULL;
    pthread_create(&despache,NULL,despachar,NULL);
}

void free_API(){
    apagar_fila(&lista_resposta);
    apagar_fila(&lista_espera);
    free(&lista_resposta);
    free(&lista_espera);
    printf("API finalizada\n");
}

// ========================================================================  FIM DA API  =======================================================================



// ====================================================================== FUNCAO DO USUARIO =======================================================================



void* importunar(void* arg){
    
    int self_id = *((int*)arg);

    while(true){
        int N_operacao = rand() % 4;
        void*funcao = operacao[N_operacao];
        
        int x = rand() % 10,    y = rand() % 10;

        printf("ususario[%d] tenta agendar\n",self_id);

        Node* id = agendarExecucao(funcao,pointer_pair(x,y));   

        printf("usario[%d] tenta pegar resultado\n",self_id);

        int resposta = pegarResultadoExecucao(id);   
        printf("usuario[%d] pegou resultado:\noperacao[%d](%d,%d) = %d\n\n",self_id,N_operacao,x,y,resposta);

    }

}

//================================================================ FIM DA FUNCAO DO USUARIO ==================================================================



#define N_users 3
pthread_t usuario[N_users];
int id_usuario[N_users];

                                                         
int main(){

    inicia_API();

    for(int i = 0; i < N_users; i++){
        id_usuario[i] = i;
        pthread_create(&usuario[i], NULL, importunar,&id_usuario[i]);
    }

    for(int i =0; i < N_users; i++){
        pthread_join(usuario[i],NULL);
    }

    free_API();
    free(operacao);
    return 0;
}


                                                        // declaracoes de funcoes

int somar (struct par P) {
    int x = P.first;
    int y = P.second;
    
    return(x + y);
}

int subtrair (struct par P) {
    int x = P.first;
    int y = P.second;
    return(x - y);
}

int multiplicar (struct par P) {
    int x = P.first;
    int y = P.second;
    return(x * y);
}

int dividir (struct par P) {
    int x = P.first;
    int y = P.second;
    if (y != 0){
        return (x / y);
    }
    else{
        return 0;
    }
}

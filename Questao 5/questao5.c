#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


/*
Ideia:
Temos threads de usuario que ficam constantemente requisitando que a API faca alguma operaca.
Os usuarios possuem acesso a 2 funcoes: 
    (I) agendarExecucao, que retorna um Node com as informacoes da requisicao.
    (II)pegarResultadoExecucao que retorna a resposta da operacao solicitada.

A API trata as resquisicoes da seguinte forma:
A funcao de agendarExecucao cria um Node para o usuario com:
    (I)   a operacao e os parametros dela
    (II)  um mutex e uma variavel de condicao feitos para acordar a thread do usuario e somente ela.
    (III) uma variavel que indica o estado (pronto = false), e outra que indica a resposta.

Coloca-se o node numa linked list de espera para ser executado. A linked list é uma regiao critica, portanto usa-se exclusao mutua para inserir nela.
Por fim acorda-se a thread despachante, e retorna-se o endereco do node da requisicao para thread usuario.

A thread despachante avalia se pode cirar threads para fazer o trabalho e depois ve se existe alguma operacao pendente na lista de espera para execucao.
    se tiver como criar thread e houver requisicao,
        a despachante remove o primeiro node da lista e cria uma thread para ele.
        Ela repete isto ate nao poder mais (porque nao tem como fazer mais threads ou nao tem mais trabalhos).
    se nao tiver a thread imediatamente volta ao wait().


A thread que executa coloca os argumentos na funcao passada para ela e coloca no node a resposta, e muda o estado do node para pronto.
A thread que executa coloca o node na lista de resposta (outra regiao critica), sinaliza 2 outras:
    I)  a thread do usuario que fez a requisicao:  usando a funcao pegarResultadoExecucao, apos acordar para pegar a resposta no node que esta na lista de resposta.
    II) a thread despachante: esta poderia estar dormindo esprando uma thread acabar a execucao para criar outra.

A funcao pegarResultadoExecucao retorna a resposta se esta pronta, se nao esta pronta, wait().
Apenas quando a resposta vai para o usuario, o node da requisicao e apagado.


*/

//================================================================ INICIO DA API =================================================================

#define true 1
#define false 0
#define N 5                             // numero maximo de threads em execucao

int executando = 0;                     // numero de threads em execucao
pthread_cond_t despache_ok;             // variavel de condicao relacionada a thread despachante
pthread_mutex_t mutex_despachante = PTHREAD_MUTEX_INITIALIZER;      // mutex relacionado a thread despachante
pthread_t despache;                     //thread de despache

pthread_mutex_t mutex_fila_resultado = PTHREAD_MUTEX_INITIALIZER;       // mutex para exclusao mutua na fila de respostas prontas   
pthread_mutex_t mutex_fila_agendamento =PTHREAD_MUTEX_INITIALIZER;     // mutex para exclusao mutua na fila de tarefas agendadas 



 // -------------------------------------------------------   estruturas de dados auxiliares  -------------------------------------------------------


                                                            //linked list


typedef struct par{
    int first,second;
}par;

void* pointer_pair(int x, int y){
    struct par* P = (par*)malloc(sizeof(struct par));
    P->first = x;
    P->second = y;
    return (void*)P;
}



typedef struct Node {

    struct Node* next;
    struct Node* prev;

    int(*funcao)(struct par);
    void*arg;

    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    
    int resposta;
    int pronto; //indica que a resposta esta pronta e thread usuario pode pega-la
}Node;

void apaga_node(Node*node){
    free(node->funcao);
    free(node->arg);
    free(node->mutex);
    free(node->cond);
    free(node);
}

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

Node* remover_node(linked_list *fila,Node *node){ ///remove node de qualquer posicao, ele pode remover de qualquer lista mas passar como parametro organiza
    if(node){
        Node* anterior = node->prev;
        Node* posterior = node->next;

        if(anterior )anterior->next = posterior;
        if(posterior)posterior->prev = anterior;
        node->prev = node->next = NULL;
    }
    return node;
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
        printf("%d --->",(int)temp);
        temp = temp->next;
    }
    printf("NULL\n");
}

void apagar_fila(linked_list* fila){
    Node* temp = remover(fila);
    while(temp != NULL){
        free(temp);
        temp = remover(fila);
    }
    printf("fila apagada\n");
}


linked_list lista_agendamento;           //duas linked_lists criadas
linked_list lista_resposta;




int somar(par P);
int subtrair(par P);
int multiplicar(par P);
int dividir(par P);     





//-----------------------------------------------------------------    funcoes da interface da API    ---------------------------------------------------------

Node* agendarExecucao(void* funexec, void *arg){

    printf("\ncriando new_node\n");
    Node* new_node = (Node*)malloc(sizeof(Node)); //cria node

    // mutex e variavel de condicao pessoais para acordar exatamente a thread usuario desta requisicao
    pthread_mutex_t personal_mutex = PTHREAD_MUTEX_INITIALIZER; 
    pthread_cond_t personal_cond;

    // informacoes para executar as funcoes
    new_node->funcao = funexec;
    new_node->arg = arg;

    // guarda no node info para retornar resposta e acordar thread usuario para recebe-la
    new_node->mutex = &personal_mutex;
    new_node->cond = &personal_cond;
    new_node->pronto = false;  // enquanto nao tem resposta 

    // lista de espera para agendar: regiao critica. 
    pthread_mutex_lock(&mutex_fila_agendamento);

    inserir(&lista_agendamento,new_node);
    
    printf("estado da lista de espera:\n");
    print_list(&lista_agendamento);

    pthread_mutex_unlock(&mutex_fila_agendamento);

    // acordar thread despachante para colocar a solicitacao em alguma thread para executar
    printf("\nacorda despachante para tratar requisicao\n"); 
    pthread_cond_signal(&despache_ok);

    printf("\nretornando new_node\n");
    Node*id = new_node; // retorna node para poder pegar a resposta
    
    
    
    
    return id;
}






                                             
// o id usado pra pegar o resultado e o endereco de um Node
int pegarResultadoExecucao(Node* id){

    pthread_mutex_t* personal_mutex = id->mutex;
    pthread_cond_t* personal_cond = id->cond;

    int Resposta;
    while(true){
  
        pthread_mutex_lock(&mutex_fila_resultado); //tenta pegar resultado
        pthread_mutex_lock(personal_mutex); //auxiliar, serve apenas para wait() quando resultado nao estiver prontos
  
        if (id->pronto){
            Resposta = (id->resposta);

            remover_node(&lista_resposta,id); // remove node de qualquer posicao que ele esteja na linked list
            pthread_mutex_unlock(&mutex_fila_resultado); //unlock para outros poderem pegar seus resultados

            apaga_node(id); //finalmente o Node pode ser apagado
            return Resposta;
        }
        else{
            printf("\nresposta nao esta pronta\n");
            pthread_mutex_unlock(&mutex_fila_resultado);  //unlock para outros poderem pegar 
            printf("dorme enquanto espera\n\n");
            pthread_cond_wait(personal_cond,personal_mutex);//acordo exatamente esta thread quando terminar.
        }
    }
}






                                                            // thread despachante

void* threadespaxe(void* primeiro){

    //variaveis criadas para deixar codigo mais limpo
    struct Node* node = (struct Node*)primeiro;
    par argumento;
        
  
    argumento = *((par*)(node->arg));
    
    
    printf("argumento: (%d,%d)\n", argumento.first,argumento.second);    


    node->resposta = (node->funcao)(argumento);    

    printf("resposta: %d\n\n",node->resposta);

    pthread_mutex_lock(&mutex_fila_resultado);                  // exclusao mutua na regiao critica
    inserir(&lista_resposta,primeiro);                      // insere na lista de resposta
    pthread_mutex_unlock(&mutex_fila_resultado);
    
    
    node->pronto = true;
    pthread_cond_signal(node->cond);    //acorda apenas a thread do usuario que fez esta requisicao
    
    executando--;
    

    pthread_cond_signal(&despache_ok);                     // acorda despachante, para executar nova requisicao
    return NULL;
}



void* despachar(void*arg){
                              //numeros de threads sendo executadas
    printf("\ndespachante on\n");
    while(true){

        pthread_mutex_lock(&mutex_despachante);

        while(executando == N || empty(&lista_agendamento)){
            printf("\nlista vazia\n"); 
            pthread_cond_wait(&despache_ok,&mutex_despachante);
        }
        
        
        //pega da lista
        printf("\ndespachante pega primeiro da lista para executar\n");
        executando++;
        
        pthread_mutex_lock(&mutex_fila_agendamento);
        Node*primeiro = remover(&lista_agendamento);     //lista e regiao critica
        pthread_mutex_unlock(&mutex_fila_agendamento);

        pthread_mutex_unlock(&mutex_despachante);

        printf("cria thread para executar\n");
        pthread_t thread;
        pthread_create(&thread,NULL,threadespaxe,primeiro);


    }
    return NULL;
}


// fim thread despachante

void inicia_API(){
    lista_agendamento.head = lista_agendamento.tail = lista_resposta.head = lista_resposta.tail = NULL;
    pthread_create(&despache,NULL,despachar,NULL);

}

void free_API(){
    apagar_fila(&lista_resposta);
    apagar_fila(&lista_agendamento);
    free(&lista_resposta);
    free(&lista_agendamento);
    printf("API finalizada\n");
    
}

// ========================================================================  FIM DA API  =======================================================================



// ====================================================================== FUNCAO DO USUARIO =======================================================================



void* importunar(void* arg){
    
    int self_id = *((int*)arg);
    printf("\nusuario %d conectado\n", self_id);
    while(true){

        int N_operacao = rand()%4;
        int x = rand() % 10,    y = rand() % 10;

        printf("\nususario[%d] tenta agendar:\noperacao[%d](%d,%d)\n",self_id,N_operacao,x,y);   
        
        Node* id;
        
        switch(N_operacao){

            case 0:
                id = agendarExecucao(somar,pointer_pair(x,y)); 
                break;
            case 1:
                id = agendarExecucao(subtrair,pointer_pair(x,y)); 
                break;
            case 2:
                id = agendarExecucao(multiplicar,pointer_pair(x,y)); 
                break;
            case 3:
                id = agendarExecucao(dividir,pointer_pair(x,y)); 
                break;

        }
  

        printf("\nusario[%d] tenta pegar resultado\n",self_id);

        int resposta = pegarResultadoExecucao(id);   
        printf("\nusuario[%d] pegou resultado:\noperacao[%d](%d,%d) = %d\n\n",self_id,N_operacao,x,y,resposta);

    }
    return NULL;

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

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
pthread_mutex_t mutex_despachante = PTHREAD_MUTEX_INITIALIZER;;      // mutex relacionado a thread despachante
pthread_t despache;                     //thread de despache

pthread_mutex_t inserir_resposta = PTHREAD_MUTEX_INITIALIZER;;       // mutex para exclusao mutua para produzir na fila de respostas     
pthread_mutex_t espera_resultado = PTHREAD_MUTEX_INITIALIZER;;       // mutex para exclusao mutua para consumir na fila de resposta

pthread_mutex_t espera_agendamento =PTHREAD_MUTEX_INITIALIZER;;     // mutex para exclusao mutua na fila de tarefas agendadas 



 // -------------------------------------------------------   estruturas de dados auxiliares  -------------------------------------------------------


                                                            //linked list

typedef struct Node {

    struct Node* next;
    struct Node* prev;

    void*funcao;
    void*arg;

    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
    
    int resposta;
    int pronto;
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
    struct par* P = (par*)malloc(sizeof(struct par));
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

    Node* new_node = (Node*)malloc(sizeof(Node)); //cria node

    // mutex e variavel de condicao pessoais para acordar exatamente a thread usuario desta requisicao
    pthread_mutex_t personal_mutex; 
    pthread_cond_t personal_cond;

    // informacoes para executar as funcoes
    new_node->funcao = funexec;
    new_node->arg = arg;

    // guarda no node info para retornar resposta e acordar thread usuario para recebe-la
    new_node->mutex = &personal_mutex;
    new_node->cond = &personal_cond;
    new_node->pronto = false;  // enquanto nao tem resposta 

    // lista de espera para agendar: regiao critica. 
    pthread_mutex_lock(&espera_agendamento);
    inserir(&lista_espera,new_node);
    pthread_mutex_unlock(&espera_agendamento);

    // acordar thread despachante para colocar a solicitacao em alguma thread para executar 
    pthread_cond_signal(&despache_ok);

    Node* id = new_node; // retorna node para poder pegar a resposta

    return id;
}






                                             
// o id usado pra pegar o resultado e o endereco de um Node
int pegarResultadoExecucao(Node* id){

    pthread_mutex_t* personal_mutex = id->mutex;
    pthread_cond_t* personal_cond = id->cond;

    int Resposta;
    while(true){
  
        pthread_mutex_lock(&espera_resultado); //tenta pegar resultado
        pthread_mutex_lock(personal_mutex);
  
        if (id->pronto){
            Resposta = (id->resposta);
            pthread_mutex_unlock(&espera_resultado);
            apaga_node(id); //finalmente o Node pode ser apagado
            return Resposta;
        }
        else{
            printf("\nresposta nao esta pronta, libera mutex\n");
            pthread_mutex_unlock(&espera_resultado);  //unlock para outros poderem pegar 
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
    void* funexec;
  
    argumento = *((par*)(node->arg));
    
    funexec = node->funcao;

    
    printf("argumento: (%d,%d)\n", argumento.first,argumento.second);

    node->resposta = funexec(argumento);    

    printf("resposta: %d\n\n",node->resposta);

    pthread_mutex_lock(&inserir_resposta);                  // exclusao mutua na regiao critica
    inserir(&lista_resposta,primeiro);                      // insere na lista de resposta
    pthread_mutex_unlock(&inserir_resposta);

    pthread_cond_signal(node->cond);    //acorda apenas a thread do usuario que fez esta requisicao
    
    executando--;
    node->pronto = true;

    pthread_cond_signal(&despache_ok);                     // acorda despachante, para executar nova requisicao
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
    printf("\nusuario %d conectado\n", self_id);
    while(true){
        int N_operacao = rand()%4;

        void*funcao = (void*)operacao[N_operacao];
        
        int x = rand() % 10,    y = rand() % 10;

        printf("\nususario[%d] tenta agendar:\noperacao[%d](%d,%d)\n",self_id,N_operacao,x,y);

        Node* id = agendarExecucao(funcao,pointer_pair(x,y));   

        printf("\nusario[%d] tenta pegar resultado\n",self_id);

        int resposta = pegarResultadoExecucao(id);   
        printf("\nusuario[%d] pegou resultado:\noperacao[%d](%d,%d) = %d\n\n",self_id,N_operacao,x,y,resposta);

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

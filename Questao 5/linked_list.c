// linked_list e suas funcoes

typedef struct Node {

    struct Node* next;
    struct Node* prev;
    int id;
    void*funcao;
    void*arg;
    pthread_mutex_t* mutex;
    pthread_cond_t* cond;
}Node;

void unlink(Node* node1,Node* node2){
    if(node1)node1->next = NULL;
    if(node2)node2->prev = NULL;
}
void link(Node* node1,Node* node2){
    node1->next = node2;
    node2->prev = node1;
}

typedef struct linked_list{
    Node* head;
    Node* tail;
}linked_list;

linked_list lista_espera;

int empty(linked_list* fila){
    return (*fila).head == NULL;
}

void inserir(linked_list* fila, Node*new_node){   //insere na frente
    
    if((*fila).head == NULL){ // se esta vazio
        (*fila).head = (*fila).tail = new_node
    }
    else{ // se nao esta vazio
        link((*fila).tail , new_node);
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
    unlink(retornado,(*fila).head); //evitar problemas, quebrar o vinculo coma linked list
    return retornado; //nao se esquecer de dar free depois de usar
}

int id_front(linked_list* fila){  // retorna id do head da list
    return  (*fila).head->id;
}
int id_back(linked_list* fila){ // retorna id do back da list
    return (*fila).tail->id;
}
void print_list(linked_list* fila){
    Node* temp = (*fila).head;
    while(temp != NULL){
        printf("%d ",temp->id);
        temp = temp->next;
    }
    printf("\n");
}

// fim da linked list e suas funcoes
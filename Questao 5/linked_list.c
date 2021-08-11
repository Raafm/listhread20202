// linked_list e suas funcoes

typedef struct Node {

    struct Node* next;
    int id;
    void*funcao;
    void*arg;
}Node;

typedef struct linked_list{
    Node* head;
    Node* tail;
}linked_list;

linked_list fila_espera;

int empty(){
    return fila_espera.head == NULL;
}

void inserir(void* funexec,void* ARG,int ID){   //insere na frente
    
    if(fila_espera.head == NULL){ // se esta vazio
        fila_espera.head = fila_espera.tail = (Node*)malloc(sizeof(Node));
    }
    else{ // se nao esta vazio
        fila_espera.tail->next = (Node*)malloc(sizeof(Node));
        fila_espera.tail = fila_espera.tail->next;  
    }
    fila_espera.tail->id = ID;
    fila_espera.tail->funcao = funexec;
    fila_espera.tail->arg = ARG;
    fila_espera.tail->next = NULL; // serve para sabermos quando estiver vazio
}

Node* remover(){//remeove de tras
    if(fila_espera.head == NULL){  //se esta vazio retorna
        return NULL;
    }
    Node* retornado = fila_espera.head;
    fila_espera.head = fila_espera.head->next;
    return retornado; //nao se esquecer de dar free depois de usar
}

int id_front(){  // retorna id do head da list
    return  fila_espera.head->id;
}
int id_back(){ // retorna id do back da list
    return fila_espera.tail->id;
}
void print_list(){
    Node* temp = fila_espera.head;
    while(temp != NULL){
        printf("%d ",temp->id);
        temp = temp->next;
    }
    printf("\n");
}

// fim da linked list e suas funcoes
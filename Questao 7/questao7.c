//Esta nao e a versao user friendly. Nao recomendo usar para rodar o codigo e entender seu funcionamento.
//Esta versao e feita para ser eficiente (talvez passar por um vjudge).

#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include <unistd.h>

int* vetor;         //vetor a ser ordenado
int N;              //tamanho do vetor a ser ordenado


void  print(int V[],int inicio,int fim){    //funcao auxiliar para print
    for(int i = inicio; i < fim; i++){ 
        printf("%d ", V[i]);
    }
    printf("\n");
}


struct pair {
    int left, right;
};

struct pair make_pair(int l, int r){
    struct pair P;
    P.left = l;
    P.right = r;
    return P;
}

void merge(int l, int r, int m){        //faz merge de dois trechos ordenados do array

    int  j  = m;
    int  i  = l;
    int aux = 0;

    int vetorAUXILIAR[r-l];             //guardarei o merge dos trechos nesse array e depois copiarei no original

    while(i < m && j < r){ //ate que todos os elementos de um dos trechos sejam colocados no vetorAUXILIAR
        //escolho o menor elemento entre os menores dos dois trechos, e coloco no vetorAUXILIAR
        if(vetor[i] < vetor[j]) { vetorAUXILIAR[aux++] = vetor[i++]; }
        else                    { vetorAUXILIAR[aux++] = vetor[j++]; }   
    }

    //coloco o resto do trecho que sobrou elementos no array
    if(i == m ){
        while(j < r){ vetorAUXILIAR[aux++] = vetor[j++]; }
    }
    else{
        while(i < m){ vetorAUXILIAR[aux++] = vetor[i++]; }
    }

    int tam = r - l;
    i = l;
    for(aux = 0; aux < tam; aux++){// retornando ao original
        vetor[i++] = vetorAUXILIAR[aux];
    } 
}


void* sort(void*arg){

    int l = ((struct pair*)arg)->left;   //indice do primeiro elemento do trecho.
    int r = ((struct pair*)arg)->right;  //indice do primeiro elemento depois do ultimo do trecho. Ou seja  o ultimo elemento tem indice r-1.

    if(l == r-1) return NULL; //lembremos-nos que o ultimo elemento do trecho tem indice r-1, e nao r.
    
    int m = (l+r)/2;

    pthread_t organizadora_left , organizadora_right; //crio uma thread para fazer o sort da particao esquerda e outra da direita

    struct pair P_left = make_pair(l,m);  //primeiro elemento da particao esquerda tem indice l e o ultimo tem indice m-1
    pthread_create(&organizadora_left , NULL,sort,&P_left); 

    struct pair P_right = make_pair(m,r);   //primeiro elemento da particao direita tem indice m e o ultimo tem indice r-1
    pthread_create(&organizadora_right, NULL,sort,&P_right);    

    pthread_join(organizadora_left, NULL); //espero as 2 trheads terminarem de fazer o sort para depois fazer o merge
    pthread_join(organizadora_right, NULL);

    merge(l,r,m);

    
    return NULL;
}

void input(){
 
    scanf(" %d",&N);

    vetor = (int*)malloc(sizeof(int)*N);    
    for(int i = 0 ; i < N ; i++){
        scanf(" %d",&vetor[i]);
    }

}

void Multi_thread_merge_sort(int* vetor,int N){

    pthread_t organizadora;
    struct pair P = make_pair(0,N);      //guarda limites da particao. O primeiro elemento do array tem indice 0, o ultimo elemento tem indice N-1.
    pthread_create(&organizadora , NULL,sort,&P);     
    pthread_join(organizadora , NULL);
}

int main(){

    input();

    Multi_thread_merge_sort(vetor,N);

    printf("\narray final: ");
    print(vetor,0,N);

    free(vetor);
    return 0;
}
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

pthread_mutex_t mutex_despachante;

//linked list dos resultados

void* threadespaxe(void* primeiro){
    void*funcexec = primeiro->funcao;
    
    primeiro->arg = funcexec(arg); //resposta fica guardada no node

    pthread_mutex_lock(&mutex_despachante);
    inserir(&lista_resposta,retorno);
    pthread_cond_signal(primeiro->cond,primeiro->mutex);
    pthread_mutex_unlock(&mutex_despachante);
    return NULL;
}

void* despachar(void*arg){
    int executando = 0;
    while(true)
    {
        pthread_mutex_lock(&mutex_despachante);
        while(executando == N || empty(&lista_espera)){
            pthread_cond_wait(&cond,&mutex_despachante);
        }
        //pega da lista
        executando++;
        Node*primeiro = remover(&lista_espera);

        pthread_t thread;
        pthread_create(&thread,NULL,threadespaxe,primeiro);
        pthread_mutex_unlock(&mutex_despachante);
    }
    return NULL;
}
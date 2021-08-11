#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

pthread_mutex_t mutex_despachante;

//linked list dos resultados

void* threadespaxe(void* primeiro){
    void*funcexec = primeiro->funcao;
    void*arg = primeriro->arg;
    int id = primeiro->id; 
    int retorno = funcexec(arg);
    inserir(lista_resposta,retorno);
    return NULL;
}

void* despachar(void*arg){
    while(true)
    {
        pthread_mutex_lock(&mutex_despachante);
        while(executando == N || empty()){
            pthread_cond_wait(&cond,&mutex_despachante);
        }
        //pega da lista
        executando++;
        Node*primeiro = remover();

        pthread_t thread;
        pthread_create(&thread,NULL,threadespaxe,primeiro);
        pthread_mutex_unlock(&mutex_despachante);
    }
    return NULL;
}
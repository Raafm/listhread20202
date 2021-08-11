#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

int agendarExecucao(void* funcao, void *arg){

    Node* new_node = (Node*)malloc(sizeof(Node));

    pthread_mutex_t personal_mutex;
    pthread_cond_t personal_cond;

    new_node->funcao = funexec;
    new_node->arg = arg;
    
    new_node->mutex = personal_mutex;
    new_node->cond = personal_cond;
    new_node->pronto = false;
    insert(new_node);

    Node* id = new_node;
    return id;
}
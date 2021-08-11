#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

int agendarExecucao(void* funcao, void *arg){

    int id;
    if(id_back()==INT_MAX){
        id=1;
    }
    else{
        id = 1+id_top();
    }
    insert(funcao,arg,id);

    return id;
}
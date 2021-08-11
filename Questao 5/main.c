#include<stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N_users 3
pthread_t usuario[N_users];
int id_usuario[N_users];
pthread_mutex_t espera_resultado;

int main(){

    for(int i = 0; i < N_users; i++){
        id_usuario[i] = i;
        pthread_create(&usuario[i], NULL, importunar,&id_usuario[i]);
    }

    for(int i =0; i < N_users; i++){
        pthread_join(usuario[i],NULL);
    }
    return 0;
}
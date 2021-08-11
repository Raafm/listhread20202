
void pegarResultadoExecucao(Node* id){//ninguem disse que o ID tem que ser um numero sequencial
    
    personal_mutex = id->mutex;
    personal_cond = id->cond;
    
    while(true){

        pthread_mutex_lock(&espera_resultado); //tenta pegar resultado
        pthread_mutex_lock(&personal_mutex);
        
        if (id.pronto){
            pthread_mutex_unlock(&espera_resultado);
            return resposta;
        }
        else{
            pthread_mutex_unlock(&espera_resultado);  //unlock para outros tentarem pegar 
            pthread_cond_wait(&personal_cond,&personal_mutex);//acordem esta thread quando terminar.
            //colocar VARIAVEL DE CONDICAO UNICA PARA CADA THREAD QUE ESPERA DENTRO DA LINKED LIST
            //QUANDO CHEGAR NA RESPOSTA, ACORDA PRECISAMENTE A QUE DORMIU 

        }

    }

}
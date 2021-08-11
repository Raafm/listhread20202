
void pegarResultadoExecucao(int id){
    
    pthread_mutex_t espera_resultado;
    pthread_mutex_lock(&espera_resultado);

    while(true){
        
        Node* resposta = search(lista_resposta,id);
        if (resposta){
            return resposta;
        }
        pthread_cond_wait(&cond,&espera_resultado);//acordem esta thread quando terminar.
        //colocar VARIAVEL DE CONDICAO UNICA PARA CADA THREAD QUE ESPERA DENTRO DA LINKED LIST
        //QUANDO CHEGAR NA RESPOSTA, ACORDA PRECISAMENTE A QUE DORMIU
    }

}
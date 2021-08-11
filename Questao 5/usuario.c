
void importunar(void* arg){
    
    int self_id = *((int*)arg);

    while(true){
        int N_operacao = rand() % 4;
        void*funcao = operacao[N_operacao];
        
        int x = rand() % 10,    y = rand() % 10;

        printf("ususario[%d] tenta agendar\n",self_id);

        Node* id = agendarExecucao(funcao,pointer_pair(x,y));   

        printf("usario[%d] tenta pegar resultado\n",self_id);

        int resposta = pegarResultadoExecucao(id);   
        printf("usuario[%d] pegou resultado:\noperacao[%d](%d,%d) = %d\n\n",self_id,N_operacao,x,y,resposta);

    }

}
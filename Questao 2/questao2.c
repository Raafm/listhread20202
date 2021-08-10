
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include<random.h>

#define limite_inferior(id) id*1000/T
#define limite_superior(id) ((id+1)*1000/T)

srand(time(NULL));



char senha[11]; //11 ints porque temos o ultimo: \0
int T; //numero de threads
    


void* brute_force(void* arg){

    int id = *((int*)arg);
    char tentativa[11];
    int p0,p1,p2,p3,p4,p5,p6,p7,p8,p9;
    int posicao = 0;
    
    int inf = limite_inferior(id);
    int sup = limite_superior(id);
    
    
    //variando cada casa
    for(p0=48;p0<57;p0++){
        tentativa[0] = p0;
        
        for(p1=48;p1<57;p1++){
            tentativa[1] = p1;

            for(p2=48;p2<57;p2++){
                tentativa[2] = p2;

                for(p3=48;p3<57;p3++){
                    tentativa[3] = p3;

                    for(p4=48;p4<57;p4++){
                        tentativa[4] = p4;

                        for(p5=48;p5<57;p5++){
                            tentativa[5] = p5;

                            for(p6=48;p6<57;p6++){
                                tentativa[6] = p6;
                        
                                for(int numero = inf; numero < sup; numero++){
                                    tentativa[7] = 48 + numero/100;                 //soma 48 para casting
                                    tentativa[8] = 48 + (numero%100)/10;            //soma 48 para casting        
                                    tentativa[9] = 48 + (numero%10);                //soma 48 para casting    
                                    
                                    printf("%s",tentativa);
                                    if(!strcmp(senha,tentativa)){                   
                                        printf("senha correta\n");
                                    }
                                    else{
                                        printf("senha incorreta");
                                    }
                                }

                            }   
                        }                
                    }    
                }
            }
        }
    }






    return NULL;
}

float quebra_senha(){
    clock_t start, end, tempo;

    start = clock();
    pthread_t atacante[T];

    for(int i = 0; i < T; i++){
        int id = i;
        pthread_create(&atacante[i],NULL,brute_force,&id);
    }


    for(int i = 0; i < T; i++){

        pthread_join(atacante,NULL);
    }
    end = clock();

    tempo = (float)(end-start)/CLOCKS_PER_SEC;
    printf("tempo gasto com %d threads: %f \n",tempo, T); 

    return tempo;
}

void set_senha(){

    for(int i = 0; i < 10; i++){
        senha[i] = 48 + rand()%10;
    }
}

int main(){


    int tempo_medio;

    T = 1;
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
     
    tempo_medio /= 3;
    printf("tempo medio com %d threads foi: %d",T,tempo_medio);

    T = 2;
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
         
    tempo_medio /= 3;
    printf("tempo medio com %d threads foi: %d",T,tempo_medio);
 

    T = 4;
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
     
    tempo_medio /= 3;
    printf("tempo medio com %d threads foi: %d",T,tempo_medio);
     

    T = 8;
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
     
    set_senha();
    tempo_medio += quebra_senha();
     
    tempo_medio /= 3;
    printf("tempo medio com %d threads foi: %d",T,tempo_medio);
     
    


    return 0;
}


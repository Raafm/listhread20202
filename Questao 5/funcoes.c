#include <stdio.h>
#include<stdlib.h>

typedef struct par{
    int first,second;
}par;

int somar(int num1, int num2);
int subtrair(int num1, int num2);
int multiplicar(int num1, int num2);
int dividir(int num1, int num2);

int (*operacao[4])(int, int);

int main() 
{
    int x, y, choice, result;


    operacao[0] = somar;
    operacao[1] = subtrair;
    operacao[2] = multiplicar;
    operacao[3] = dividir;

    result = operacao[choice](x, y);
    printf("\n %d", result);

    return 0;

}

int somar (struct par P) {
    int x = P.first;
    int y = P.second;
    
    return(x + y);
}

int subtrair (struct par P) {
    int x = P.first;
    int y = P.second;
    return(x - y);
}

int multiplicar (struct par P) {
    int x = P.first;
    int y = P.second;
    return(x * y);
}

int dividir (struct par P) {
    int x = P.first;
    int y = P.second;
    if (y != 0){
        return (x / y);
    }
    else{
        return 0;
    }
} 
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include "expressao.h" // ALTERADO

int comparar_floats(float a, float b, float epsilon) {
    return fabs(a - b) < epsilon;
}

void testar_expressao(Calculadora* calc, const char* infixa, float valor_esperado, int testar_erro) {
    printf("----------------------------------------\n");
    printf("Expressao Infixa: \"%s\"\n", infixa);

    char* posfixa = converter_infixo_para_posfixo(calc, infixa);
    if (!posfixa) {
        if (testar_erro) {
            printf(">> SUCESSO: Erro de sintaxe na conversao capturado como esperado.\n");
        } else {
            printf(">> FALHA: Erro inesperado na conversao para posfixa.\n");
        }
        return;
    }
    printf("Forma Posfixa   : \"%s\"\n", posfixa);

    float resultado;
    CalcStatus status = calcular_valor_posfixo(calc, posfixa, &resultado);

    if (testar_erro) {
        if (status != CALC_SUCESSO) {
            printf(">> SUCESSO: Erro de calculo capturado como esperado (Status: %d).\n", status);
        } else {
            printf(">> FALHA: Um erro era esperado, mas o calculo foi bem sucedido (Resultado: %f).\n", resultado);
        }
    } else {
        if (status == CALC_SUCESSO) {
            printf("Resultado       : %f\n", resultado);
            if (comparar_floats(resultado, valor_esperado, 0.01f)) {
                printf(">> SUCESSO: Valor calculado corresponde ao esperado (%f).\n", valor_esperado);
            } else {
                printf(">> FALHA: Valor calculado (%f) diferente do esperado (%f).\n", resultado, valor_esperado);
            }
        } else {
            printf(">> FALHA: Erro inesperado no calculo (Status: %d).\n", status);
        }
    }
    
    free(posfixa);
}

int main() {
    printf("Criando instancia da calculadora...\n");
    Calculadora* calc = criar_calculadora();
    if (!calc) {
        fprintf(stderr, "Falha critica: Nao foi possivel criar a calculadora!\n");
        return 1;
    }

    printf("\n--- Testes de Sucesso ---\n");
    testar_expressao(calc, "(3 + 4) * 5", 35.0f, 0);
    testar_expressao(calc, "7 * 2 + 4", 18.0f, 0);
    testar_expressao(calc, "8 + (5 * (2 + 4))", 38.0f, 0);
    testar_expressao(calc, "(6 / 2 + 3) * 4", 24.0f, 0);
    testar_expressao(calc, "9 + (5 * (2 + 8 * 4))", 179.0f, 0);
    testar_expressao(calc, "log(2 + 3) / 5", 0.13979f, 0);
    testar_expressao(calc, "log(10) ^ 3 + 2", 3.0f, 0);
    testar_expressao(calc, "(45 + 60) * cos(30)", 90.9326f, 0);
    testar_expressao(calc, "sen(45)^2 + 0.5", 1.0f, 0);
    testar_expressao(calc, "raiz(64) % 3", 2.0f, 0);
    testar_expressao(calc, "-5 * (-3 + 1)", 10.0f, 0);

    printf("\n--- Testes de Erro ---\n");
    testar_expressao(calc, "10 / 0", 0.0f, 1); // Espera-se um erro de cálculo
    testar_expressao(calc, "5 + * 3", 0.0f, 1); // Espera-se um erro de sintaxe
    testar_expressao(calc, "(10 + 2", 0.0f, 1); // Espera-se um erro de sintaxe

    printf("----------------------------------------\n");
    printf("Destruindo instancia da calculadora...\n");
    destruir_calculadora(calc);

    return 0;
}

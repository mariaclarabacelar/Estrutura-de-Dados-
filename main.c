#include <stdio.h>
#include <string.h>
#include <math.h> // Para isnan
#include "calculadora.h" // Inclui o cabeçalho do projeto

// Protótipo da função auxiliar interna para conversão de infix para posfix,
// pois ela não está exposta no calculadora.h.
// Ela é necessária para testar a avaliação de expressões infixadas.
static char *getFormaPosFixa_Internal(char *Str);


// Função auxiliar para executar e exibir os testes de avaliação
void executarTesteAvaliacao(const char *expressaoInfixaOriginal, const char *expressaoPosFixaEntrada, float valorEsperado) {
    Expressao exp;
    
    // Configura a expressão pós-fixada de entrada
    strcpy(exp.posFixa, expressaoPosFixaEntrada);

    // Converte a expressão pós-fixada para infixada para exibição
    char *convertedInfixa = getFormaInFixa((char*)expressaoPosFixaEntrada);
    if (convertedInfixa != NULL) {
        strcpy(exp.inFixa, convertedInfixa);
    } else {
        strcpy(exp.inFixa, "ERRO NA CONVERSAO POSFIXA -> INFIXA");
    }

    // Avalia a expressão pós-fixada
    exp.Valor = getValor((char*)expressaoPosFixaEntrada);

    printf("--- Teste de Avaliacao ---\n");
    printf("Expressao Infixa Original: %s\n", expressaoInfixaOriginal); // Para referência
    printf("Expressao Pos-Fixa de Entrada: %s\n", exp.posFixa);
    printf("Expressao Infixa Convertida: %s\n", exp.inFixa); // Convertida da pos-fixa
    if (isnan(exp.Valor)) {
        printf("Valor Calculado: ERRO\n");
    } else {
        printf("Valor Calculado: %.2f\n", exp.Valor);
    }
    printf("Valor Esperado: %.2f\n", valorEsperado);
    printf("\n");
}

// Função auxiliar para testar a conversão de infixa para posfixa
void testarConversaoInfixaPosFixa(const char *infixaInput, const char *posFixaEsperada) {
    char *posFixaConvertida = getFormaPosFixa_Internal((char*)infixaInput);
    printf("--- Teste de Conversao Infixa -> Pos-Fixa ---\n");
    printf("Entrada Infixa: %s\n", infixaInput);
    if (posFixaConvertida != NULL) {
        printf("Pos-Fixa Convertida: %s\n", posFixaConvertida);
        printf("Pos-Fixa Esperada: %s\n", posFixaEsperada);
    } else {
        printf("Erro na conversao de infixa para pos-fixa.\n");
    }
    printf("\n");
}


int main() {
    printf("Avaliador de Expressoes Numericas\n\n");

    // Testes de Avaliação (usando expressões pos-fixas diretamente para getValor)
    printf("Testes de Avaliacao (usando getValor com entrada pos-fixa):\n\n");
    executarTesteAvaliacao("(3 + 4) * 5", "3 4 + 5 *", 35.0); // Teste 1
    executarTesteAvaliacao("7 * 2 + 4", "7 2 * 4 +", 18.0); // Teste 2
    executarTesteAvaliacao("8 + (5 * (2 + 4))", "8 5 2 4 + * +", 38.0); // Teste 3
    executarTesteAvaliacao("(6 / 2 + 3) * 4", "6 2 / 3 + 4 *", 24.0); // Teste 4
    executarTesteAvaliacao("9 + (5 * (2 + 8 * 4))", "9 5 2 8 * 4 + * +", 109.0); // Teste 5
    // Teste 6: log(2 + 3) / 5
    executarTesteAvaliacao("log(2 + 3) / 5", "2 3 + log 5 /", 0.13979); // Aproximadamente 0.14
    // Teste 7: (log10)^3 + 2 => log(10) ^ 3 + 2. Em pos-fixa 10 log 3 ^ 2 +
    executarTesteAvaliacao("log(10) ^ 3 + 2", "10 log 3 ^ 2 +", 3.0); // log10(10) = 1; 1^3 + 2 = 3
    // Teste 8: (45 + 60) * cos(30)
    executarTesteAvaliacao("(45 + 60) * cos(30)", "45 60 + 30 cos *", 90.9347); // Aproximadamente 90.93
    // Teste 9: sen(45)^2 + 0.5
    executarTesteAvaliacao("sen(45)^2 + 0.5", "0.5 45 sen 2 ^ +", 0.5 + pow(sin(45.0 * M_PI / 180.0), 2.0)); // Aproximadamente 1.0


    printf("Testes de Conversao Infixa para Pos-Fixa (usando funcao auxiliar interna):\n\n");
    testarConversaoInfixaPosFixa("(3 + 4) * 5", "3 4 + 5 *");
    testarConversaoInfixaPosFixa("7 * 2 + 4", "7 2 * 4 +");
    testarConversaoInfixaPosFixa("8 + (5 * (2 + 4))", "8 5 2 4 + * +");
    testarConversaoInfixaPosFixa("(6 / 2 + 3) * 4", "6 2 / 3 + 4 *");
    testarConversaoInfixaPosFixa("9 + (5 * (2 + 8 * 4))", "9 5 2 8 * 4 + * +");
    testarConversaoInfixaPosFixa("log(2 + 3) / 5", "2 3 + log 5 /");
    testarConversaoInfixaPosFixa("log(10) ^ 3 + 2", "10 log 3 ^ 2 +");
    testarConversaoInfixaPosFixa("(45 + 60) * cos(30)", "45 60 + 30 cos *");
    testarConversaoInfixaPosFixa("sen(45)^2 + 0.5", "45 sen 2 ^ 0.5 +"); // Note: a ordem dos operandos para '+' na posfixa pode variar dependendo da implementação

    return 0;
}

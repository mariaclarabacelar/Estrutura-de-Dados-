#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Para sin, cos, tan, sqrt, log10, pow, isnan, fmod
#include <ctype.h> // Para isdigit, isalpha
#include "calculadora.h" // Inclui o cabeçalho do projeto

// Define _USE_MATH_DEFINES para M_PI no Visual Studio (para PI)
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

// --- Implementação da Pilha de Caracteres (para operadores e parênteses na conversão infixa->posfixa) ---
#define MAX_PILHA_CHAR_SIZE 256
static char pilhaChar[MAX_PILHA_CHAR_SIZE];
static int topoChar = -1;

// Empilha um caractere na pilha de caracteres
static void empilhaChar(char c) {
    if (topoChar < MAX_PILHA_CHAR_SIZE - 1) {
        pilhaChar[++topoChar] = c;
    } else {
        fprintf(stderr, "Erro: Pilha de caracteres cheia!\n");
    }
}

// Desempilha um caractere da pilha de caracteres
static char desempilhaChar() {
    if (topoChar != -1) {
        return pilhaChar[topoChar--];
    } else {
        fprintf(stderr, "Erro: Pilha de caracteres vazia!\n");
        return '\0'; // Retorno de erro
    }
}

// Retorna o caractere no topo da pilha de caracteres sem removê-lo
static char topoPilhaChar() {
    if (topoChar != -1) {
        return pilhaChar[topoChar];
    } else {
        return '\0';
    }
}

// Verifica se a pilha de caracteres está vazia
static int pilhaCharVazia() {
    return topoChar == -1;
}

// Limpa a pilha de caracteres
static void limparPilhaChar() {
    topoChar = -1;
}

// --- Implementação da Pilha de Floats (para operandos na avaliação) ---
#define MAX_PILHA_FLOAT_SIZE 256
static float pilhaFloat[MAX_PILHA_FLOAT_SIZE];
static int topoFloat = -1;

// Empilha um float na pilha de floats
static void empilhaFloat(float f) {
    if (topoFloat < MAX_PILHA_FLOAT_SIZE - 1) {
        pilhaFloat[++topoFloat] = f;
    } else {
        fprintf(stderr, "Erro: Pilha de floats cheia!\n");
    }
}

// Desempilha um float da pilha de floats
static float desempilhaFloat() {
    if (topoFloat != -1) {
        return pilhaFloat[topoFloat--];
    } else {
        fprintf(stderr, "Erro: Pilha de floats vazia!\n");
        return NAN; // Retorno Not-a-Number para erro
    }
}

// Verifica se a pilha de floats está vazia
static int pilhaFloatVazia() {
    return topoFloat == -1;
}

// Limpa a pilha de floats
static void limparPilhaFloat() {
    topoFloat = -1;
}

// --- Implementação da Pilha de Strings (para conversão posfixa para infixa) ---
#define MAX_PILHA_STRING_SIZE 256
static char *pilhaString[MAX_PILHA_STRING_SIZE];
static int topoString = -1;

// Empilha uma string na pilha de strings
static void empilhaString(char *s) {
    if (topoString < MAX_PILHA_STRING_SIZE - 1) {
        // Aloca memória para a nova string e copia o conteúdo
        pilhaString[++topoString] = (char *)malloc(strlen(s) + 1);
        if (pilhaString[topoString] == NULL) {
            fprintf(stderr, "Erro: Falha na alocacao de memoria para string da pilha!\n");
            return;
        }
        strcpy(pilhaString[topoString], s);
    } else {
        fprintf(stderr, "Erro: Pilha de strings cheia!\n");
    }
}

// Desempilha uma string da pilha de strings
static char *desempilhaString() {
    if (topoString != -1) {
        char *s = pilhaString[topoString--];
        return s; // Retorna a string alocada (quem chama deve liberar)
    } else {
        fprintf(stderr, "Erro: Pilha de strings vazia!\n");
        return NULL; // Retorno de erro
    }
}

// Verifica se a pilha de strings está vazia
static int pilhaStringVazia() {
    return topoString == -1;
}

// Limpa a pilha de strings e libera a memória alocada para as strings
static void limparPilhaString() {
    while (!pilhaStringVazia()) {
        free(desempilhaString()); // Libera a memória de cada string
    }
    topoString = -1;
}

// --- Funções Auxiliares Comuns ---

// Verifica se um caractere é um operador binário
static int ehOperador(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^');
}

// Verifica se uma string corresponde a uma função unária
static int ehFuncao(const char *s) {
    return (strcmp(s, "raiz") == 0 || strcmp(s, "sen") == 0 ||
            strcmp(s, "cos") == 0 || strcmp(s, "tg") == 0 ||
            strcmp(s, "log") == 0);
}

// Retorna a precedência de um operador
static int precedencia(char operador) {
    if (operador == '+' || operador == '-') return 1;
    if (operador == '*' || operador == '/' || operador == '%') return 2;
    if (operador == '^') return 3;
    // Caracteres especiais para funções na pilha de char
    if (operador == 'R' || operador == 'S' || operador == 'C' || operador == 'T' || operador == 'L') return 4;
    return 0; // Para parênteses ou outros
}

// Realiza a operação binária (op1 operador op2)
static float realizaOperacao(char operador, float op2, float op1) {
    switch (operador) {
        case '+': return op1 + op2;
        case '-': return op1 - op2;
        case '*': return op1 * op2;
        case '/':
            if (op2 == 0) {
                fprintf(stderr, "Erro: Divisao por zero!\n");
                return NAN; // Retorna Not-a-Number para erro
            }
            return op1 / op2;
        case '%':
            // fmod é usado para o resto da divisão de floats
            if (op2 == 0) {
                fprintf(stderr, "Erro: Modulo por zero!\n");
                return NAN;
            }
            return fmod(op1, op2);
        case '^': return pow(op1, op2);
        default: return NAN; // Erro, operador desconhecido
    }
}

// Realiza a operação de função unária (funcao(operando))
static float realizaFuncao(const char *funcao, float operando) {
    if (strcmp(funcao, "raiz") == 0) {
        if (operando < 0) {
            fprintf(stderr, "Erro: Raiz quadrada de numero negativo!\n");
            return NAN;
        }
        return sqrt(operando);
    } else if (strcmp(funcao, "sen") == 0) {
        return sin(operando * M_PI / 180.0); // Converte graus para radianos
    } else if (strcmp(funcao, "cos") == 0) {
        return cos(operando * M_PI / 180.0); // Converte graus para radianos
    } else if (strcmp(funcao, "tg") == 0) {
        // Tangente de 90 + k*180 graus é indefinida
        if (fmod(operando, 180.0) == 90.0 || fmod(operando, 180.0) == -90.0) {
            fprintf(stderr, "Erro: Tangente de angulo indefinido (90 + k*180)!\n");
            return NAN;
        }
        return tan(operando * M_PI / 180.0); // Converte graus para radianos
    } else if (strcmp(funcao, "log") == 0) {
        if (operando <= 0) {
            fprintf(stderr, "Erro: Logaritmo de numero nao positivo!\n");
            return NAN;
        }
        return log10(operando);
    }
    return NAN; // Erro, função desconhecida
}


// --- Funções principais do calculadora.h ---

// Retorna a forma inFixa de Str (posFixa)
char *getFormaInFixa(char *Str) {
    static char inFixaOutput[512]; // Buffer estático para o resultado
    char *token;
    char *temp_str; // Ponteiro para a cópia modificável da string
    char buffer[256]; // Buffer para construir sub-expressões

    limparPilhaString();
    
    // Aloca memória para a cópia da string original, para que strtok possa modificá-la
    temp_str = strdup(Str); 
    if (temp_str == NULL) {
        fprintf(stderr, "Erro: Falha na alocacao de memoria para getFormaInFixa!\n");
        return NULL;
    }

    token = strtok(temp_str, " "); // Tokeniza por espaços

    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '-' && strlen(token) > 1 && isdigit(token[1])) || (token[0] == '.' && strlen(token) > 1 && isdigit(token[1]))) { 
            // É um número (considera números negativos e decimais que começam com '.')
            empilhaString(token);
        } else if (ehOperador(token[0]) && strlen(token) == 1) { // É um operador
            if (pilhaStringVazia()) { 
                fprintf(stderr, "Erro: Expressao posfixa invalida (operador sem operandos)!\n"); 
                free(temp_str); // Libera a memória da cópia
                limparPilhaString();
                return NULL; 
            }
            char *op2 = desempilhaString();
            if (pilhaStringVazia()) { 
                fprintf(stderr, "Erro: Expressao posfixa invalida (operador binario com apenas um operando)!\n"); 
                free(op2); // Libera o que foi desempilhado
                free(temp_str);
                limparPilhaString();
                return NULL; 
            }
            char *op1 = desempilhaString();

            // Construir a sub-expressão com parênteses
            sprintf(buffer, "( %s %c %s )", op1, token[0], op2);
            empilhaString(buffer);
            
            free(op1); // Libera a memória das sub-expressões desempilhadas
            free(op2);
        } else if (ehFuncao(token)) { // É uma função unária
            if (pilhaStringVazia()) { 
                fprintf(stderr, "Erro: Expressao posfixa invalida (funcao sem operando)!\n"); 
                free(temp_str);
                limparPilhaString();
                return NULL; 
            }
            char *op1 = desempilhaString();
            
            // Construir a sub-expressão para a função
            sprintf(buffer, "%s( %s )", token, op1);
            empilhaString(buffer);
            free(op1);
        } else {
            // Token inválido ou não reconhecido
            fprintf(stderr, "Erro: Token invalido na expressao posfixa para conversao: '%s'!\n", token);
            free(temp_str);
            limparPilhaString();
            return NULL;
        }
        token = strtok(NULL, " ");
    }

    if (!pilhaStringVazia() && topoString == 0) { // Deve sobrar apenas um elemento na pilha
        char *final_expr = desempilhaString();
        strcpy(inFixaOutput, final_expr);
        free(final_expr); // Libera a última string alocada
    } else {
        fprintf(stderr, "Erro: Expressao posfixa malformada (operandos sobrando/faltando)!\n");
        strcpy(inFixaOutput, ""); // Expressão vazia ou erro
    }
    
    free(temp_str); // Libera a cópia da string original
    limparPilhaString(); // Garante que a pilha está vazia

    return inFixaOutput;
}


// Calcula o valor de Str (na forma posFixa)
float getValor(char *Str) {
    char *token;
    char *temp_str; // Ponteiro para a cópia modificável da string
    float op1, op2, resultado;

    limparPilhaFloat();
    
    // Aloca memória para a cópia da string original, para que strtok possa modificá-la
    temp_str = strdup(Str);
    if (temp_str == NULL) {
        fprintf(stderr, "Erro: Falha na alocacao de memoria para getValor!\n");
        return NAN;
    }

    token = strtok(temp_str, " ");

    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '-' && strlen(token) > 1 && isdigit(token[1])) || (token[0] == '.' && strlen(token) > 1 && isdigit(token[1]))) {
            // É um número (considera números negativos e decimais que começam com '.')
            empilhaFloat(atof(token));
        } else if (ehOperador(token[0]) && strlen(token) == 1) { // É um operador binário
            if (pilhaFloatVazia()) { 
                fprintf(stderr, "Erro: Expressao posfixa invalida (operador sem operandos)!\n"); 
                free(temp_str); 
                return NAN; 
            }
            op2 = desempilhaFloat();
            if (pilhaFloatVazia()) { 
                fprintf(stderr, "Erro: Expressao posfixa invalida (operador binario com apenas um operando)!\n"); 
                free(temp_str); 
                return NAN; 
            }
            op1 = desempilhaFloat();
            resultado = realizaOperacao(token[0], op2, op1);
            if (isnan(resultado)) { 
                free(temp_str); 
                limparPilhaFloat(); 
                return NAN; 
            } // Propaga erro
            empilhaFloat(resultado);
        } else if (ehFuncao(token)) { // É uma função unária
            if (pilhaFloatVazia()) { 
                fprintf(stderr, "Erro: Expressao posfixa invalida (funcao sem operando)!\n"); 
                free(temp_str); 
                return NAN; 
            }
            op1 = desempilhaFloat();
            resultado = realizaFuncao(token, op1);
            if (isnan(resultado)) { 
                free(temp_str); 
                limparPilhaFloat(); 
                return NAN; 
            } // Propaga erro
            empilhaFloat(resultado);
        } else {
            // Token inválido
            fprintf(stderr, "Erro: Token invalido na expressao posfixa para avaliacao: '%s'!\n", token);
            free(temp_str);
            limparPilhaFloat();
            return NAN;
        }
        token = strtok(NULL, " ");
    }

    if (!pilhaFloatVazia() && topoFloat == 0) { // Deve sobrar apenas um elemento na pilha
        resultado = desempilhaFloat();
        free(temp_str); // Libera a cópia da string original
        limparPilhaFloat();
        return resultado;
    } else {
        // Erro na expressao (ex: operadores faltando operandos ou operandos sobrando)
        fprintf(stderr, "Erro: Expressao posfixa malformada ou operandos sobrando/faltando.\n");
        free(temp_str);
        limparPilhaFloat();
        return NAN;
    }
}


// --- Funções Auxiliares Internas (NÃO fazem parte do calculadora.h, mas são necessárias) ---

// Retorna a forma posFixa de Str (infixa) - FUNÇÃO AUXILIAR INTERNA
// Esta função é interna e não é exposta no .h, mas é utilizada para permitir
// que o main.c teste a avaliação de expressões infixadas, convertendo-as
// primeiro para pós-fixada e depois avaliando com getValor.
static char *getFormaPosFixa_Internal(char *Str) {
    static char posFixa[512]; // Buffer estático para o resultado
    char token_str[64]; // Para armazenar números ou funções lidos
    int i = 0, k = 0, j = 0;
    
    limparPilhaChar();
    memset(posFixa, 0, sizeof(posFixa)); // Limpa o buffer

    while (Str[i] != '\0') {
        if (Str[i] == ' ') { // Ignora espaços
            i++;
            continue;
        }
        
        if (isdigit(Str[i]) || Str[i] == '.') { // É um número
            j = 0;
            while (isdigit(Str[i]) || Str[i] == '.') {
                token_str[j++] = Str[i++];
            }
            token_str[j] = '\0';
            strcat(posFixa, token_str);
            strcat(posFixa, " "); // Adiciona espaço após o número
            continue;
        }
        
        // Verifica se é uma função (ex: sen, cos, log, raiz, tg)
        if (isalpha(Str[i])) {
            j = 0;
            // Lê a string da função
            while (isalpha(Str[i]) && j < 63) { 
                token_str[j++] = Str[i++];
            }
            token_str[j] = '\0';

            if (ehFuncao(token_str)) {
                // Empilha um caractere marcador para cada função
                if (strcmp(token_str, "raiz") == 0) empilhaChar('R');
                else if (strcmp(token_str, "sen") == 0) empilhaChar('S');
                else if (strcmp(token_str, "cos") == 0) empilhaChar('C');
                else if (strcmp(token_str, "tg") == 0) empilhaChar('T');
                else if (strcmp(token_str, "log") == 0) empilhaChar('L');
                
            } else {
                fprintf(stderr, "Erro: Caracteres invalidos ou funcao nao reconhecida na expressao infixada: '%s'!\n", token_str);
                return NULL;
            }
            continue; // Continua para o próximo caractere
        }

        if (Str[i] == '(') {
            empilhaChar(Str[i++]);
            continue;
        }

        if (Str[i] == ')') {
            while (!pilhaCharVazia() && topoPilhaChar() != '(') {
                char op = desempilhaChar();
                if (op != 'R' && op != 'S' && op != 'C' && op != 'T' && op != 'L') { // Não é uma função (operador normal)
                    posFixa[k++] = op;
                    posFixa[k++] = ' ';
                } else { // É uma função (adiciona a string da função)
                    if (op == 'R') strcat(posFixa, "raiz ");
                    else if (op == 'S') strcat(posFixa, "sen ");
                    else if (op == 'C') strcat(posFixa, "cos ");
                    else if (op == 'T') strcat(posFixa, "tg ");
                    else if (op == 'L') strcat(posFixa, "log ");
                }
            }
            if (!pilhaCharVazia() && topoPilhaChar() == '(') {
                desempilhaChar(); // Desempilha o '('
            } else {
                fprintf(stderr, "Erro: Parenteses desbalanceados na expressao infixada!\n");
                return NULL; // Erro de parênteses desbalanceados
            }
            i++;
            continue;
        }

        if (ehOperador(Str[i])) {
            while (!pilhaCharVazia() && 
                   topoPilhaChar() != '(' && 
                   precedencia(topoPilhaChar()) >= precedencia(Str[i])) {
                char op = desempilhaChar();
                if (op != 'R' && op != 'S' && op != 'C' && op != 'T' && op != 'L') { // Não é uma função
                    posFixa[k++] = op;
                    posFixa[k++] = ' ';
                } else { // É uma função
                    if (op == 'R') strcat(posFixa, "raiz ");
                    else if (op == 'S') strcat(posFixa, "sen ");
                    else if (op == 'C') strcat(posFixa, "cos ");
                    else if (op == 'T') strcat(posFixa, "tg ");
                    else if (op == 'L') strcat(posFixa, "log ");
                }
            }
            empilhaChar(Str[i++]);
            continue;
        }
        
        // Caractere inesperado
        fprintf(stderr, "Erro: Caractere inesperado na expressao infixada: '%c'!\n", Str[i]);
        return NULL; // Ou trate o erro de outra forma
    }

    while (!pilhaCharVazia()) {
        char op = desempilhaChar();
        if (op == '(') {
            fprintf(stderr, "Erro: Parenteses desbalanceados na expressao infixada!\n");
            return NULL; // Erro de parênteses desbalanceados
        }
        if (op != 'R' && op != 'S' && op != 'C' && op != 'T' && op != 'L') { // Não é uma função
            posFixa[k++] = op;
            posFixa[k++] = ' ';
        } else { // É uma função
            if (op == 'R') strcat(posFixa, "raiz ");
            else if (op == 'S') strcat(posFixa, "sen ");
            else if (op == 'C') strcat(posFixa, "cos ");
            else if (op == 'T') strcat(posFixa, "tg ");
            else if (op == 'L') strcat(posFixa, "log ");
        }
    }
    posFixa[k] = '\0'; // Termina a string
    // Remove o espaço extra no final, se houver
    if (k > 0 && posFixa[k-1] == ' ') posFixa[k-1] = '\0';

    return posFixa;
}

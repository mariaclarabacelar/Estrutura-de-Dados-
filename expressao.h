#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "expressao.h" // ALTERADO

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif

#ifndef NAN
#define NAN (0.0f/0.0f)
#endif

// --- Estrutura de Dados Interna ---

#define MAX_PILHA_SIZE 256

struct Calculadora {
    char pilhaChar[MAX_PILHA_SIZE];
    int topoChar;

    float pilhaFloat[MAX_PILHA_SIZE];
    int topoFloat;

    char* pilhaString[MAX_PILHA_SIZE];
    int topoString;
};

// --- Funções de Pilha (static) ---

static void limparPilhaChar(Calculadora *calc) { calc->topoChar = -1; }
static int empilhaChar(Calculadora *calc, char c) {
    if (calc->topoChar >= MAX_PILHA_SIZE - 1) return 0;
    calc->pilhaChar[++calc->topoChar] = c;
    return 1;
}
static char desempilhaChar(Calculadora *calc) { return calc->topoChar != -1 ? calc->pilhaChar[calc->topoChar--] : '\0'; }
static char topoPilhaChar(Calculadora *calc) { return calc->topoChar != -1 ? calc->pilhaChar[calc->topoChar] : '\0'; }
static int pilhaCharVazia(Calculadora *calc) { return calc->topoChar == -1; }

static void limparPilhaFloat(Calculadora *calc) { calc->topoFloat = -1; }
static int empilhaFloat(Calculadora *calc, float f) {
    if (calc->topoFloat >= MAX_PILHA_SIZE - 1) return 0;
    calc->pilhaFloat[++calc->topoFloat] = f;
    return 1;
}
static float desempilhaFloat(Calculadora *calc) { return calc->topoFloat != -1 ? calc->pilhaFloat[calc->topoFloat--] : NAN; }
static int pilhaFloatVazia(Calculadora *calc) { return calc->topoFloat == -1; }

static void limparPilhaString(Calculadora *calc) {
    while (calc->topoString != -1) {
        free(calc->pilhaString[calc->topoString--]);
    }
}
static int empilhaString(Calculadora *calc, const char *s) {
    if (calc->topoString >= MAX_PILHA_SIZE - 1) return 0;
    calc->pilhaString[++calc->topoString] = strdup(s);
    return calc->pilhaString[calc->topoString] != NULL;
}
static char* desempilhaString(Calculadora *calc) { return calc->topoString != -1 ? calc->pilhaString[calc->topoString--] : NULL; }
static int pilhaStringVazia(Calculadora *calc) { return calc->topoString == -1; }

// --- Funções Auxiliares (static) ---

static int ehOperador(char c) { return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^'; }
static int ehFuncao(const char *s) {
    const char *funcoes[] = {"raiz", "sen", "cos", "tg", "log", NULL};
    for (int i = 0; funcoes[i]; i++) if (strcmp(s, funcoes[i]) == 0) return 1;
    return 0;
}
static int precedencia(char operador) {
    switch(operador) {
        case '+': case '-': return 1;
        case '*': case '/': case '%': return 2;
        case '^': return 3;
        case 'R': case 'S': case 'C': case 'T': case 'L': return 4; // Funções
        default: return 0;
    }
}
static int adicionar_operador_a_saida(char op, char* buf, int idx, size_t max_len) {
    switch (op) {
        case 'R': return snprintf(buf + idx, max_len - idx, "raiz ");
        case 'S': return snprintf(buf + idx, max_len - idx, "sen ");
        case 'C': return snprintf(buf + idx, max_len - idx, "cos ");
        case 'T': return snprintf(buf + idx, max_len - idx, "tg ");
        case 'L': return snprintf(buf + idx, max_len - idx, "log ");
        default:  return snprintf(buf + idx, max_len - idx, "%c ", op);
    }
}
static float realizaOperacao(char op, float op2, float op1) {
    switch (op) {
        case '+': return op1 + op2;
        case '-': return op1 - op2;
        case '*': return op1 * op2;
        case '/': return op2 != 0 ? op1 / op2 : NAN;
        case '%': return op2 != 0 ? fmod(op1, op2) : NAN;
        case '^': return pow(op1, op2);
        default: return NAN;
    }
}
static float realizaFuncao(const char *func, float op) {
    if (strcmp(func, "raiz") == 0) return op >= 0 ? sqrt(op) : NAN;
    if (strcmp(func, "log") == 0) return op > 0 ? log10(op) : NAN;
    double ang_rad = op * M_PI / 180.0;
    if (strcmp(func, "sen") == 0) return sin(ang_rad);
    if (strcmp(func, "cos") == 0) return cos(ang_rad);
    if (strcmp(func, "tg") == 0) {
        if (fmod(op, 180.0) == 90.0 || fmod(op, 180.0) == -90.0) return NAN;
        return tan(ang_rad);
    }
    return NAN;
}


// --- Implementação da API Pública ---

Calculadora* criar_calculadora(void) {
    Calculadora* calc = (Calculadora*)malloc(sizeof(Calculadora));
    if (calc) {
        calc->topoChar = -1;
        calc->topoFloat = -1;
        calc->topoString = -1;
    }
    return calc;
}

void destruir_calculadora(Calculadora* calc) {
    if (calc) {
        limparPilhaString(calc);
        free(calc);
    }
}

char* converter_infixo_para_posfixo(Calculadora* calc, const char* infixa) {
    if (!calc || !infixa) return NULL;

    size_t buffer_len = strlen(infixa) * 2 + 100;
    char *posfixa = (char*)malloc(buffer_len);
    if (!posfixa) return NULL;

    limparPilhaChar(calc);
    posfixa[0] = '\0';
    int k = 0; // Índice para a string de saída
    int i = 0;
    int esperando_operando = 1;

    while (infixa[i] != '\0' && (size_t)k < buffer_len -1) {
        if (isspace(infixa[i])) { i++; continue; }

        if (isdigit(infixa[i]) || (infixa[i] == '.' && isdigit(infixa[i+1]))) {
            char num_buf[64];
            int j = 0;
            while (j < 63 && (isdigit(infixa[i]) || infixa[i] == '.')) num_buf[j++] = infixa[i++];
            num_buf[j] = '\0';
            k += snprintf(posfixa + k, buffer_len - k, "%s ", num_buf);
            esperando_operando = 0;
            continue;
        }
        
        if (esperando_operando && infixa[i] == '-') {
            char num_buf[64];
            int j = 0;
            num_buf[j++] = infixa[i++];
            while (j < 63 && (isdigit(infixa[i]) || infixa[i] == '.')) num_buf[j++] = infixa[i++];
            num_buf[j] = '\0';
            k += snprintf(posfixa + k, buffer_len - k, "%s ", num_buf);
            esperando_operando = 0;
            continue;
        }

        if (isalpha(infixa[i])) {
            char func_buf[10];
            int j = 0;
            while (j < 9 && isalpha(infixa[i])) func_buf[j++] = infixa[i++];
            func_buf[j] = '\0';
            if (!ehFuncao(func_buf)) { free(posfixa); return NULL; }
            char marcador = 0;
            if (strcmp(func_buf, "raiz") == 0) marcador = 'R';
            else if (strcmp(func_buf, "sen") == 0) marcador = 'S';
            else if (strcmp(func_buf, "cos") == 0) marcador = 'C';
            else if (strcmp(func_buf, "tg") == 0) marcador = 'T';
            else if (strcmp(func_buf, "log") == 0) marcador = 'L';
            empilhaChar(calc, marcador);
            continue;
        }

        if (infixa[i] == '(') {
            empilhaChar(calc, '(');
            esperando_operando = 1;
            i++;
            continue;
        }
        
        if (infixa[i] == ')') {
            while (!pilhaCharVazia(calc) && topoPilhaChar(calc) != '(') {
                k += adicionar_operador_a_saida(desempilhaChar(calc), posfixa, k, buffer_len);
            }
            if (pilhaCharVazia(calc)) { free(posfixa); return NULL; } // Parênteses desbalanceados
            desempilhaChar(calc); // Pop '('
            if (!pilhaCharVazia(calc) && isalpha(topoPilhaChar(calc))) {
                 k += adicionar_operador_a_saida(desempilhaChar(calc), posfixa, k, buffer_len);
            }
            i++;
            esperando_operando = 0;
            continue;
        }

        if (ehOperador(infixa[i])) {
            int assoc_dir = (infixa[i] == '^');
            while (!pilhaCharVazia(calc) && topoPilhaChar(calc) != '(' &&
                   (precedencia(topoPilhaChar(calc)) > precedencia(infixa[i]) ||
                   (precedencia(topoPilhaChar(calc)) == precedencia(infixa[i]) && !assoc_dir))) {
                k += adicionar_operador_a_saida(desempilhaChar(calc), posfixa, k, buffer_len);
            }
            empilhaChar(calc, infixa[i]);
            i++;
            esperando_operando = 1;
            continue;
        }
        
        free(posfixa); return NULL; // Caractere inválido
    }

    while(!pilhaCharVazia(calc)) {
        char op = desempilhaChar(calc);
        if (op == '(') { free(posfixa); return NULL; } // Parênteses desbalanceados
        k += adicionar_operador_a_saida(op, posfixa, k, buffer_len);
    }

    if (k > 0) posfixa[k-1] = '\0'; else posfixa[k] = '\0';
    return posfixa;
}

char* converter_posfixo_para_infixo(Calculadora* calc, const char* posfixa) {
    if (!calc || !posfixa) return NULL;
    limparPilhaString(calc);

    char* copia_posfixa = strdup(posfixa);
    if (!copia_posfixa) return NULL;

    char* token = strtok(copia_posfixa, " ");
    while(token) {
        if (isdigit(token[0]) || (token[0] == '-' && strlen(token) > 1) || token[0] == '.') {
            if (!empilhaString(calc, token)) {
                free(copia_posfixa); limparPilhaString(calc); return NULL;
            }
        } else if (ehOperador(token[0]) && strlen(token) == 1) {
            char* op2 = desempilhaString(calc);
            char* op1 = desempilhaString(calc);
            if (!op1 || !op2) { free(op1); free(op2); free(copia_posfixa); limparPilhaString(calc); return NULL; }
            
            size_t len = strlen(op1) + strlen(op2) + 10;
            char* buffer = malloc(len);
            if(!buffer) { free(op1); free(op2); free(copia_posfixa); limparPilhaString(calc); return NULL; }

            snprintf(buffer, len, "( %s %s %s )", op1, token, op2);
            if (!empilhaString(calc, buffer)) {
                free(buffer); free(op1); free(op2); free(copia_posfixa); limparPilhaString(calc); return NULL;
            }
            free(buffer); free(op1); free(op2);
        } else if (ehFuncao(token)) {
            char* op1 = desempilhaString(calc);
            if (!op1) { free(copia_posfixa); limparPilhaString(calc); return NULL; }

            size_t len = strlen(op1) + strlen(token) + 10;
            char* buffer = malloc(len);
             if(!buffer) { free(op1); free(copia_posfixa); limparPilhaString(calc); return NULL; }

            snprintf(buffer, len, "%s( %s )", token, op1);
             if (!empilhaString(calc, buffer)) {
                free(buffer); free(op1); free(copia_posfixa); limparPilhaString(calc); return NULL;
            }
            free(buffer); free(op1);
        } else {
            free(copia_posfixa); limparPilhaString(calc); return NULL; // Token inválido
        }
        token = strtok(NULL, " ");
    }
    free(copia_posfixa);

    if (calc->topoString != 0) { limparPilhaString(calc); return NULL; }
    return desempilhaString(calc);
}

CalcStatus calcular_valor_posfixo(Calculadora* calc, const char* posfixa, float* resultado) {
    if (!calc || !posfixa || !resultado) return CALC_ERRO_DESCONHECIDO;
    limparPilhaFloat(calc);

    char* copia_posfixa = strdup(posfixa);
    if (!copia_posfixa) return CALC_ERRO_MEMORIA;

    char* token = strtok(copia_posfixa, " ");
    while(token) {
        if (isdigit(token[0]) || (token[0] == '-' && strlen(token) > 1) || token[0] == '.') {
            if (!empilhaFloat(calc, atof(token))) { free(copia_posfixa); return CALC_ERRO_MEMORIA; }
        } else if (ehOperador(token[0]) && strlen(token) == 1) {
            float op2 = desempilhaFloat(calc);
            float op1 = desempilhaFloat(calc);
            if (isnan(op1) || isnan(op2)) { free(copia_posfixa); return CALC_ERRO_SINTAXE; }
            float res = realizaOperacao(token[0], op2, op1);
            if (isnan(res)) { free(copia_posfixa); return CALC_ERRO_MATEMATICO; }
            if (!empilhaFloat(calc, res)) { free(copia_posfixa); return CALC_ERRO_MEMORIA; }
        } else if (ehFuncao(token)) {
            float op1 = desempilhaFloat(calc);
            if (isnan(op1)) { free(copia_posfixa); return CALC_ERRO_SINTAXE; }
            float res = realizaFuncao(token, op1);
            if (isnan(res)) { free(copia_posfixa); return CALC_ERRO_MATEMATICO; }
            if (!empilhaFloat(calc, res)) { free(copia_posfixa); return CALC_ERRO_MEMORIA; }
        } else {
            free(copia_posfixa); return CALC_ERRO_SINTAXE;
        }
        token = strtok(NULL, " ");
    }
    free(copia_posfixa);

    float final_res = desempilhaFloat(calc);
    if (isnan(final_res) || !pilhaFloatVazia(calc)) {
        return CALC_ERRO_SINTAXE;
    }

    *resultado = final_res;
    return CALC_SUCESSO;
}

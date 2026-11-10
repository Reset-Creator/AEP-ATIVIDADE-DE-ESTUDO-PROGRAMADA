#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ACOES      100
#define MAX_SUGESTOES  20
#define TAM_CODIGO     11
#define TAM_TEXTO      60
#define TAM_DATA       11
#define TAM_SUGESTAO   200
#define NUM_OPCOES_ENQUETE 4

enum Situacao { PLANEJADA = 1, REALIZADA = 2, CANCELADA = 3 };

typedef struct {
    char codigo[TAM_CODIGO];
    char escola[TAM_TEXTO];
    char tema[TAM_TEXTO];
    char data[TAM_DATA];
    char publico[TAM_TEXTO];
    char responsavel[TAM_TEXTO];
    int  prevista;
    int  efetiva;
    int  situacao;
} Acao;

/*Armazenamento em memória*/
static Acao acoes[MAX_ACOES];
static int totalAcoes = 0;

static char sugestoes[MAX_SUGESTOES][TAM_SUGESTAO];
static int totalSugestoes = 0;

static const char *opcoesEnquete[NUM_OPCOES_ENQUETE] = {
    "Palestras sobre alimentacao saudavel",
    "Atividades fisicas no patio",
    "Rodas de conversa sobre saude mental",
    "Campanha de saude bucal"
};
static int votosEnquete[NUM_OPCOES_ENQUETE] = {0, 0, 0, 0};

/*informativo*/
static const char *diasSemana[5] = {
    "Segunda-feira", "Terca-feira", "Quarta-feira", "Quinta-feira", "Sexta-feira"
};
static const char *cardapio[5] = {
    "Arroz, feijao, carne e salada",
    "Macarrao, frango e legumes",
    "Arroz, feijao, carne e frutas",
    "Arroz, frango, legumes e suco",
    "Macarrao, carne e salada"
};

/*Entrada de dados e validacoes*/
static void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

static void lerLinha(char *dest, int tam) {
    if (fgets(dest, tam, stdin) == NULL) {
        printf("\nEntrada encerrada. Saindo do programa.\n");
        exit(0);
    }
    size_t len = strlen(dest);
    if (len > 0 && dest[len - 1] == '\n') {
        dest[len - 1] = '\0';
    } else {
        limparBuffer(); /*linha maior que o buffer*/
    }
}

static void aparar(char *s) {
    int ini = 0, fim = (int)strlen(s) - 1;
    while (s[ini] != '\0' && isspace((unsigned char)s[ini])) ini++;
    while (fim >= ini && isspace((unsigned char)s[fim])) fim--;
    int n = fim - ini + 1;
    if (n < 0) n = 0;
    memmove(s, s + ini, (size_t)n);
    s[n] = '\0';
}

/*texto obrigatório*/
static void lerTextoObrigatorio(const char *rotulo, char *dest, int tam) {
    for (;;) {
        printf("%s", rotulo);
        lerLinha(dest, tam);
        aparar(dest);
        if (dest[0] != '\0') return;
        printf("  [ERRO] Campo obrigatorio. Nao pode ficar vazio.\n");
    }
}

/*intervalo, repetindo até ficar válido*/
static int lerInteiro(const char *rotulo, int min, int max) {
    char buf[32];
    for (;;) {
        printf("%s", rotulo);
        lerLinha(buf, sizeof(buf));
        aparar(buf);
        if (buf[0] == '\0') {
            printf("  [ERRO] Digite um numero.\n");
            continue;
        }
        char *fim;
        long v = strtol(buf, &fim, 10);
        if (*fim != '\0') {
            printf("  [ERRO] Valor invalido. Digite apenas numeros inteiros.\n");
        } else if (v < min || v > max) {
            if (v < 0) printf("  [ERRO] Quantidade negativa nao e permitida.\n");
            else printf("  [ERRO] Valor fora do intervalo permitido (%d a %d).\n", min, max);
        } else {
            return (int)v;
        }
    }
}

static int dataValida(const char *s) {
    if (strlen(s) != 10 || s[2] != '/' || s[5] != '/') return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    int dia = atoi(s), mes = atoi(s + 3), ano = atoi(s + 6);
    int diasMes[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (ano < 2026 || ano > 2100 || mes < 1 || mes > 12) return 0;
    if ((ano % 4 == 0 && ano % 100 != 0) || ano % 400 == 0) diasMes[1] = 29;
    return dia >= 1 && dia <= diasMes[mes - 1];
}

static void lerData(const char *rotulo, char *dest) {
    for (;;) {
        printf("%s", rotulo);
        lerLinha(dest, TAM_DATA + 20);
        aparar(dest);
        if (dataValida(dest)) return;
        printf("  [ERRO] Data invalida. Use o formato DD/MM/AAAA (ano a partir de 2026).\n");
    }
}

static void paraMaiusculas(char *s) {
    for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

/*ignora maiusculas/minusculas*/
static int contem(const char *texto, const char *trecho) {
    size_t n = strlen(trecho);
    if (n == 0) return 1;
    for (size_t i = 0; texto[i] != '\0'; i++) {
        size_t j = 0;
        while (j < n && texto[i + j] != '\0' &&
               tolower((unsigned char)texto[i + j]) == tolower((unsigned char)trecho[j])) j++;
        if (j == n) return 1;
    }
    return 0;
}

static const char *nomeSituacao(int s) {
    switch (s) {
        case PLANEJADA: return "Planejada";
        case REALIZADA: return "Realizada";
        case CANCELADA: return "Cancelada";
        default:        return "?";
    }
}

static void pausar(void) {
    printf("\nPressione ENTER para continuar...");
    char tmp[8];
    lerLinha(tmp, sizeof(tmp));
}

/*Modulo: Acoes de Saude (funcionalidades obrigatorias)*/

static int buscarPorCodigo(const char *codigo) {
    for (int i = 0; i < totalAcoes; i++) {
        if (strcmp(acoes[i].codigo, codigo) == 0) return i;
    }
    return -1;
}

static void imprimirCampo(const char *s, int largura) {
    int chars = 0;
    const char *p = s;
    while (*p != '\0' && chars < largura) {
        int bytes = 1;
        unsigned char c = (unsigned char)*p;
        if (c >= 0xF0) bytes = 4;
        else if (c >= 0xE0) bytes = 3;
        else if (c >= 0xC0) bytes = 2;
        fwrite(p, 1, (size_t)bytes, stdout);
        p += bytes;
        chars++;
    }
    for (; chars < largura; chars++) putchar(' ');
    putchar(' ');
}

static void cabecalhoTabela(void) {
    imprimirCampo("CODIGO", 7);
    imprimirCampo("ESCOLA", 22);
    imprimirCampo("TEMA", 20);
    imprimirCampo("DATA", 10);
    imprimirCampo("PUBLICO", 10);
    imprimirCampo("RESPONSAVEL", 18);
    imprimirCampo("PREV.", 5);
    imprimirCampo("EFET.", 5);
    imprimirCampo("SITUACAO", 9);
    printf("\n");
    printf("--------------------------------------------------------------------------------------------------------------\n");
}

static void imprimirAcao(const Acao *a) {
    char prev[12], efet[12];
    snprintf(prev, sizeof(prev), "%d", a->prevista);
    if (a->situacao == REALIZADA) snprintf(efet, sizeof(efet), "%d", a->efetiva);
    else snprintf(efet, sizeof(efet), "-");
    imprimirCampo(a->codigo, 7);
    imprimirCampo(a->escola, 22);
    imprimirCampo(a->tema, 20);
    imprimirCampo(a->data, 10);
    imprimirCampo(a->publico, 10);
    imprimirCampo(a->responsavel, 18);
    imprimirCampo(prev, 5);
    imprimirCampo(efet, 5);
    imprimirCampo(nomeSituacao(a->situacao), 9);
    printf("\n");
}

/*Cadastrar*/
static void cadastrarAcao(void) {
    printf("\n=== CADASTRAR ACAO DO PSE ===\n");
    if (totalAcoes >= MAX_ACOES) {
        printf("[ERRO] Limite de armazenamento atingido (%d acoes). Nao e possivel cadastrar.\n", MAX_ACOES);
        return;
    }

    Acao nova;
    char codigo[TAM_CODIGO + 10];

    lerTextoObrigatorio("Codigo da acao (ex.: A001): ", codigo, sizeof(codigo));
    if (strlen(codigo) >= TAM_CODIGO) {
        printf("[ERRO] Codigo muito longo (maximo %d caracteres).\n", TAM_CODIGO - 1);
        return;
    }
    paraMaiusculas(codigo);
    if (buscarPorCodigo(codigo) != -1) {
        printf("[ERRO] Codigo duplicado: ja¡ existe uma acao com o codigo %s.\n", codigo);
        return;
    }
    strcpy(nova.codigo, codigo);

    lerTextoObrigatorio("Escola: ", nova.escola, TAM_TEXTO);
    lerTextoObrigatorio("Tema (ex.: Alimentacao saudavel): ", nova.tema, TAM_TEXTO);
    lerData("Data prevista (DD/MM/AAAA): ", nova.data);
    lerTextoObrigatorio("Publico-alvo (ex.: 6º ano): ", nova.publico, TAM_TEXTO);
    lerTextoObrigatorio("Responsavel (funcao/equipe, sem dados de alunos): ", nova.responsavel, TAM_TEXTO);
    nova.prevista = lerInteiro("Quantidade prevista de participantes: ", 1, 100000);
    nova.efetiva = 0;
    nova.situacao = PLANEJADA;

    acoes[totalAcoes++] = nova;
    printf("\n[OK] acao %s cadastrada com sucesso (situacao inicial: Planejada).\n", nova.codigo);
}

/*Listar*/
static void listarAcoes(void) {
    printf("\n=== LISTA DE ACOES CADASTRADAS ===\n");
    if (totalAcoes == 0) {
        printf("Nenhuma acao cadastrada.\n");
        return;
    }
    cabecalhoTabela();
    for (int i = 0; i < totalAcoes; i++) imprimirAcao(&acoes[i]);
    printf("\nTotal de acoes: %d\n", totalAcoes);
}

/*Pesquisar*/
static void pesquisarAcoes(void) {
    printf("\n=== PESQUISAR ACOES ===\n");
    if (totalAcoes == 0) {
        printf("Nenhuma acao cadastrada.\n");
        return;
    }
    printf("1 - Por codigo\n2 - Por escola\n3 - Por tema\n");
    int tipo = lerInteiro("Escolha o tipo de pesquisa: ", 1, 3);

    char termo[TAM_TEXTO];
    lerTextoObrigatorio("Digite o termo de busca: ", termo, TAM_TEXTO);

    int achou = 0;
    for (int i = 0; i < totalAcoes; i++) {
        int corresponde = 0;
        if (tipo == 1) {
            char tmp[TAM_TEXTO];
            strncpy(tmp, termo, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            paraMaiusculas(tmp);
            corresponde = (strcmp(acoes[i].codigo, tmp) == 0);
        } else if (tipo == 2) {
            corresponde = contem(acoes[i].escola, termo);
        } else {
            corresponde = contem(acoes[i].tema, termo);
        }
        if (corresponde) {
            if (!achou) { printf("\n"); cabecalhoTabela(); }
            imprimirAcao(&acoes[i]);
            achou++;
        }
    }
    if (!achou) printf("\n[AVISO] Registro inexistente: nenhuma acao encontrada para \"%s\".\n", termo);
    else printf("\n%d registro(s) encontrado(s).\n", achou);
}

/*Atualizar situacao*/
static void atualizarSituacao(void) {
    printf("\n=== ATUALIZAR SITUACAO DA ACAO ===\n");
    if (totalAcoes == 0) {
        printf("Nenhuma acao cadastrada.\n");
        return;
    }
    char codigo[TAM_TEXTO];
    lerTextoObrigatorio("Codigo da acao: ", codigo, sizeof(codigo));
    paraMaiusculas(codigo);
    int idx = buscarPorCodigo(codigo);
    if (idx == -1) {
        printf("[ERRO] Registro inexistente: nao e acao com o codigo %s.\n", codigo);
        return;
    }

    printf("Situacao atual: %s\n", nomeSituacao(acoes[idx].situacao));
    printf("1 - Planejada\n2 - Realizada\n3 - Cancelada\n");
    int nova = lerInteiro("Nova situacao: ", 1, 3);

    acoes[idx].situacao = nova;
    if (nova == REALIZADA) {
        acoes[idx].efetiva = lerInteiro("Quantidade efetiva de participantes: ", 0, 100000);
    } else {
        acoes[idx].efetiva = 0;
    }
    printf("\n[OK] Situacao da acao %s atualizada para %s.\n", acoes[idx].codigo, nomeSituacao(nova));
}

/*Resumo geral*/
static void gerarResumo(void) {
    int plan = 0, real = 0, canc = 0;
    long totalEfetivo = 0, previstoRealizadas = 0;

    for (int i = 0; i < totalAcoes; i++) {
        if (acoes[i].situacao == PLANEJADA) plan++;
        else if (acoes[i].situacao == CANCELADA) canc++;
        else {
            real++;
            totalEfetivo += acoes[i].efetiva;
            previstoRealizadas += acoes[i].prevista;
        }
    }

    printf("\n=== RESUMO GERAL ===\n");
    printf("Total de acoes cadastradas : %d\n", totalAcoes);
    printf("acoes planejadas           : %d\n", plan);
    printf("Acoes realizadas           : %d\n", real);
    printf("Acoes canceladas           : %d\n", canc);
    printf("Total de participantes     : %ld\n", totalEfetivo);
    if (previstoRealizadas > 0) {
        printf("Participacao (realizadas)  : %.1f%% (%ld efetivos de %ld previstos)\n",
               (double)totalEfetivo * 100.0 / (double)previstoRealizadas,
               totalEfetivo, previstoRealizadas);
    } else {
        printf("Participacao (realizadas)  : nao ha acoes realizadas para calcular o percentual.\n");
    }
}

static void menuAcoesSaude(void) {
    int op;
    do {
        printf("\n===== ACOES DE SAUDE (PSE) =====\n");
        printf("1 - Cadastrar acao\n");
        printf("2 - Listar acoes\n");
        printf("3 - Pesquisar acoes (codigo, escola ou tema)\n");
        printf("4 - Atualizar situacao / registrar participantes\n");
        printf("5 - Resumo geral\n");
        printf("0 - Voltar ao menu principal\n");
        op = lerInteiro("Escolha uma opcao: ", 0, 5);
        switch (op) {
            case 1: cadastrarAcao();     pausar(); break;
            case 2: listarAcoes();       pausar(); break;
            case 3: pesquisarAcoes();    pausar(); break;
            case 4: atualizarSituacao(); pausar(); break;
            case 5: gerarResumo();       pausar(); break;
            default: break;
        }
    } while (op != 0);
}

/*Demais modulos do projeto*/

static void mostrarCardapio(void) {
    printf("\n===== CARDAPIO DA ESCOLA =====\n");
    for (int i = 0; i < 5; i++) printf("%d - %s\n", i + 1, diasSemana[i]);
    int dia = lerInteiro("Escolha o dia (1 a 5): ", 1, 5);
    printf("\n%s: %s\n", diasSemana[dia - 1], cardapio[dia - 1]);
}

static void mostrarEventos(void) {
    printf("\n===== EVENTOS DA ESCOLA =====\n");
    printf("- 15/10/2026 - Feira de ciencias\n");
    printf("- 22/10/2026 - Gincana esportiva\n");
    printf("- 05/11/2026 - Semana da cultura de paz\n");
}

static void mostrarAvisos(void) {
    printf("\n===== AVISOS DA ESCOLA =====\n");
    printf("- Reuniao de pais e responsaveis na ultima sexta-feira do mes.\n");
    printf("- Use a garrafinha de agua nas aulas de educacao fisica.\n");
    printf("- Biblioteca aberta das 7h Ã s 17h.\n");
}

static void mostrarConsultas(void) {
    printf("\n===== CONSULTAS E ATENDIMENTOS COLETIVOS =====\n");
    printf("Informativo (dados ficticios e coletivos):\n");
    printf("- Avaliacao de saude bucal em grupo: 2ª quinzena de outubro.\n");
    printf("- Verificacao coletiva da situacao vacinal: mes de novembro.\n");
    printf("Este sistema nao realiza diagnostico, triagem ou atendimento de emergencia.\n");
    printf("Procure a unidade de saude de referencia para atendimento individual.\n");
}

static void menuEnquetes(void) {
    int op;
    do {
        printf("\n===== ENQUETES E SUGESTOES =====\n");
        printf("1 - Votar na enquete\n");
        printf("2 - Ver resultado da enquete\n");
        printf("3 - Enviar sugestoes\n");
        printf("4 - Listar sugestoes recebidas\n");
        printf("0 - Voltar\n");
        op = lerInteiro("Escolha uma opcao: ", 0, 4);

        if (op == 1) {
            printf("\nQual atividade voce gostaria de ter na escola?\n");
            for (int i = 0; i < NUM_OPCOES_ENQUETE; i++)
                printf("%d - %s\n", i + 1, opcoesEnquete[i]);
            int v = lerInteiro("Seu voto: ", 1, NUM_OPCOES_ENQUETE);
            votosEnquete[v - 1]++;
            printf("[OK] Voto registrado. Obrigado por participar!\n");
        } else if (op == 2) {
            printf("\nResultado da enquete:\n");
            for (int i = 0; i < NUM_OPCOES_ENQUETE; i++)
                printf("- %-40s : %d voto(s)\n", opcoesEnquete[i], votosEnquete[i]);
        } else if (op == 3) {
            if (totalSugestoes >= MAX_SUGESTOES) {
                printf("[ERRO] Limite de sugestoes atingido (%d).\n", MAX_SUGESTOES);
            } else {
                printf("Nao informe nomes nem dados pessoais ou de saude.\n");
                lerTextoObrigatorio("Sua sugestao: ", sugestoes[totalSugestoes], TAM_SUGESTAO);
                totalSugestoes++;
                printf("[OK] Sugestao enviada. Obrigado!\n");
            }
        } else if (op == 4) {
            printf("\nSugestoes recebidas:\n");
            if (totalSugestoes == 0) printf("Nenhuma sugestao ate o momento.\n");
            for (int i = 0; i < totalSugestoes; i++)
                printf("%d. %s\n", i + 1, sugestoes[i]);
        }
        if (op != 0) pausar();
    } while (op != 0);
}

/*Dados de demonstracao*/

static void adicionarDemo(const char *cod, const char *esc, const char *tema,
                          const char *data, const char *pub, const char *resp,
                          int prev, int efet, int sit) {
    Acao *a = &acoes[totalAcoes++];
    snprintf(a->codigo, TAM_CODIGO, "%s", cod);
    snprintf(a->escola, TAM_TEXTO, "%s", esc);
    snprintf(a->tema, TAM_TEXTO, "%s", tema);
    snprintf(a->data, TAM_DATA, "%s", data);
    snprintf(a->publico, TAM_TEXTO, "%s", pub);
    snprintf(a->responsavel, TAM_TEXTO, "%s", resp);
    a->prevista = prev;
    a->efetiva = efet;
    a->situacao = sit;
}

static void carregarDadosDemo(void) {
    adicionarDemo("A001", "Escola Municipal Alfa", "Alimentacao saudavel", "15/09/2026",
                  "6º ano", "Equipe Saude", 40, 35, REALIZADA);
    adicionarDemo("A002", "Escola Municipal Beta", "Atividade fisica", "20/09/2026",
                  "7º ano", "Prof. Educ. Fisica", 50, 0, PLANEJADA);
    adicionarDemo("A003", "Escola Municipal Alfa", "Saude mental", "25/09/2026",
                  "9º ano", "Equipe Pedagogica", 30, 0, CANCELADA);
}

/*Menu principal*/

static void exibirMenuPrincipal(void) {
    printf("\n===== SISTEMA SAUDE NA ESCOLA =====\n");
    printf("1 - Cardapio\n");
    printf("2 - Eventos\n");
    printf("3 - Avisos\n");
    printf("4 - Acoes de Saude (cadastro e acompanhamento)\n");
    printf("5 - Consultas\n");
    printf("6 - Enquetes e Sugestoes\n");
    printf("0 - Sair\n");
}

int main(void) {
    int opcao;
    carregarDadosDemo();

    do {
        exibirMenuPrincipal();
        opcao = lerInteiro("Escolha uma opcao: ", 0, 6);
        switch (opcao) {
            case 1: mostrarCardapio();  pausar(); break;
            case 2: mostrarEventos();   pausar(); break;
            case 3: mostrarAvisos();    pausar(); break;
            case 4: menuAcoesSaude();             break;
            case 5: mostrarConsultas(); pausar(); break;
            case 6: menuEnquetes();               break;
            case 0: printf("\nSistema encerrado.\n"); break;
            default: break;
        }
    } while (opcao != 0);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_SALAS 15
#define LINHAS 5
#define COLUNAS 5

#define MAX_INIMIGOS 3

typedef enum {
    ATIRADOR = 1,
    SAMURAI,
    CIBORGUE
} Classe;

typedef enum {
    CHECARSTATUS = 1,
    CHECAREQUIPAMENTO,
    VERIFICARMAPA,
    PROXIMASALA
} OpcaoMenuPadrao;

typedef struct {
    char nome[25];
    Classe classe;
    int vida, vidaMax;
    int forca;
    int defesa;
    int agilidade;
    int inteligencia;
    char habilidadeEspecial[20];
} Personagem;

typedef struct {
    char nome[25];
    Classe classe;
    int vida, vidaMax;
    int forca;
    int defesa;
    int agilidade;
    int inteligencia;
} Inimigo;

typedef struct {
    char tipo;
    int linha;
    int coluna;
    int proxima;
} Sala;

typedef struct {
    char nome[25];
    Sala salas[MAX_SALAS];
    Inimigo inimigos[MAX_INIMIGOS];
    Inimigo boss;
    int totalSalas;
    int jogador;
} Mapa;

void limparTela() {
    printf("\033[2J\033[H");
}

char* obterNomeClasse(Classe classe) {
    switch (classe)
    {
        case ATIRADOR: return "Atirador";
        case SAMURAI: return "Samurai";
        case CIBORGUE: return "Ciborgue";
        default: return "Inexistente";
    }
}

void mostrarStatus(Personagem *personagem) {
    printf(">=================================<\n");
    printf("            [Seus Status]          \n");
    printf(">=================================<\n");
    printf("%15s  %s\n", "[Nome]", personagem->nome);
    printf("%15s  %s\n", "[Classe]", obterNomeClasse(personagem->classe));
    printf("%15s  %d / %d\n", "[Vida]", personagem->vida, personagem->vidaMax);
    printf("%15s  %d\n", "[Forca]", personagem->forca);
    printf("%15s  %d\n", "[Defesa]", personagem->defesa);
    printf("%15s  %d\n", "[Agilidade]", personagem->agilidade);
    printf("%15s  %d\n", "[Inteligencia]", personagem->inteligencia);
    printf("%15s  %s\n", "[Hab. Especial]", personagem->habilidadeEspecial);
}

void apresentarHistoria() {
    printf(">=========================<\n");
    printf("         [Historia]        \n");
    printf(">=========================<\n");
    printf("Estamos em 2085, inumeros fatores como guerras intercontinentais, o crescimento absurdo da humanidade,\n");
    printf("a evolucao da tecnologia e a ganancia por dominar as estrelas tornaram o planeta Terra altamente inabitavel\n");
    printf("levando os seres humanos a migrarem atraves de naves espaciais para planetas e luas distintos.\n");
    printf("\n");
    printf("Diante das mudancas da humanidade, planetas terrestres foram terraformados e os gasosos tiveram suas luas\n");
    printf("habitadas. A antiga lua da Terra (agora chamada de Selene) eh o mundo natal do protagonista, um piloto de\n");
    printf("uma nave de exploracao que trabalha no transporte de cargas e de vez em quando realiza missoes para figuras\n");
    printf("importantes do sistema solar.\n");
    printf("\n");
    printf("Voce (o piloto), em uma de suas missoes, acaba deixando escapar no vazio do espaco um nucleo energetico\n");
    printf("(objeto capaz de gerar energia para grandes polos por centenas de anos) que estava transportando para Marte\n");
    printf("e acaba caindo em um grande prejuizo financeiro, mas recebe uma proposta de quitacao da divida: o proprio\n");
    printf("rei de Marte o propoe de acabar com todos os mercenarios espaciais da gangue Space Hounds para perdoar\n");
    printf("o que voce devia.\n");
}

void aguardarEnter() {
    int enter = 0;
    printf(">=============================<\n");
    printf("Pressione <Enter> para continuar...\n");
    while ((enter = getchar()) != '\n' && enter != EOF);
}

void criarJogador(Personagem *personagem) {
    Personagem classes[] = {
        {"", ATIRADOR, 80, 80, 5, 5, 10, 10, "Rajada de tiros"},
        {"", SAMURAI, 120, 120, 10, 10, 5, 5, "Golpe em espiral"},
        {"", CIBORGUE, 150, 150, 5, 15, 0, 5, "Canhao de plasma"}
    };

    char nome_personagem[25];
    int escolha_classe;

    printf("Digite o nome do seu personagem: ");
    fgets(nome_personagem, sizeof(nome_personagem), stdin);

    nome_personagem[strcspn(nome_personagem, "\n")] = '\0';

    printf(">=======================<\n");
    printf("        [Classes]        \n");
    printf(">=======================<\n");
    printf("[1] - Atirador\n");
    printf("[2] - Samurai\n");
    printf("[3] - Ciborgue\n");
    printf("Digite o numero da sua classe: ");
    scanf("%d", &escolha_classe);
    getchar();

    while (escolha_classe > 3 || escolha_classe < 1) {
        printf(">===============<\n");
        printf("Classe inexistente, Tente novamente!\n");
        printf("Digite o numero da sua classe: ");
        scanf("%d", &escolha_classe);
        getchar();
    }

    *personagem = classes[escolha_classe - 1];
    strcpy(personagem->nome, nome_personagem);
    printf(">=============================<\n");
    printf("Seu personagem foi criado!\n");
    printf("Status abaixo...\n");
    mostrarStatus(personagem);
}

Mapa construirFase(char *nome, Sala salas[], int totalSalas) {
    Mapa mapa;
    strcpy(mapa.nome, nome);
    mapa.totalSalas = totalSalas;
    mapa.jogador = 0;

    for (int i = 0; i < mapa.totalSalas; i++) {
        mapa.salas[i] = salas[i];
    }

    return mapa;
}

void avancarSala(Mapa *mapa) {
    int proxima = mapa->salas[mapa->jogador].proxima;

    printf(">==================================<\n");
    printf("Voce avanca para o proximo destino\n");

    mapa->jogador = proxima;
}

void salaInimigo() {
    printf(">===========================<\n");
    printf("Voce encontra um inimigo!    \n");
}

void construirVisualFase(Mapa *mapa) {
    printf(">=======================<\n");
    printf("       [Fase: %s]\n", mapa->nome);
    printf(">=======================<\n");
    printf("\n");

    char gradeMapa[LINHAS][COLUNAS];
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            gradeMapa[i][j] = ' ';
        }
    }

    for (int i = 0; i < mapa->totalSalas; i++) {
        int linha = mapa->salas[i].linha;
        int coluna = mapa->salas[i].coluna;
        gradeMapa[linha][coluna] = mapa->salas[i].tipo;
    }

    int jogadorLinha = mapa->salas[mapa->jogador].linha;
    int jogadorColuna = mapa->salas[mapa->jogador].coluna;

    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            if (gradeMapa[i][j] != ' ') {
                if (i == jogadorLinha && j == jogadorColuna) {
                    printf("\033[32m[%c]\033[0m", gradeMapa[i][j]);
                }
                else {
                    printf("[%c]", gradeMapa[i][j]);
                }   
            }
            else {
                printf("   ");
            }
        }
        printf("\n");
    }
}

void menuPadrao(Mapa *mapa, Personagem *personagem) {
    int escolha;
    
    do {
        printf(">===============================<\n");
        printf("      [Qual decisao tomar?]      \n");
        printf(">===============================<\n");
        printf("[1] - Checar meus status         \n");
        printf("[2] - Checar meu equipamento     \n");
        printf("[3] - Verificar o mapa           \n");
        printf("[4] - Ir para o proximo destino  \n");
        printf("Digite a sua acao: ");
        scanf("%d", &escolha);

        switch (escolha)
        {
            case CHECARSTATUS:
                limparTela();
                mostrarStatus(personagem);
                break;
            case CHECAREQUIPAMENTO: break;
            case VERIFICARMAPA:
                limparTela();
                construirVisualFase(mapa);
                break;
            case PROXIMASALA:
                limparTela();
                avancarSala(mapa);
                construirVisualFase(mapa);
                break;
            
            default: limparTela(); break;
        }
    } while(escolha > CHECARSTATUS && escolha < PROXIMASALA);
    
}

int girarDado(int qtdLados) {
    int min = 1;
    return rand() % (qtdLados - min + 1) + min;
}

int main() {
    srand(time(NULL));
    limparTela();

    Personagem personagem;
    
    criarJogador(&personagem);
    aguardarEnter();
    limparTela();

    apresentarHistoria();
    aguardarEnter();
    limparTela();
    
    // Primeira fase
    Sala fase1_salas[] = {
        {'E', 0, 0, 1},
        {'T', 0, 1, 2},
        {'T', 0, 2, 3},
        {'R', 0, 3, 4},
        {'X', 0, 4, 5},
        {'T', 1, 4, 6},
        {'T', 2, 4, 7},
        {'X', 2, 3, 8},
        {'R', 2, 2, 9},
        {'T', 2, 1, 10},
        {'T', 3, 1, 11},
        {'X', 4, 1, 12},
        {'R', 4, 2, 13},
        {'X', 4, 3, -1}
    };

    // Segunda fase
    Sala fase2_salas[] = {
        {'E', 0, 0, 1},
        {'T', 1, 0, 2},
        {'X', 2, 0, 3},
        {'R', 2, 1, 4},
        {'T', 3, 1, 5},
        {'T', 4, 1, 6},
        {'X', 4, 2, 7},
        {'T', 4, 3, 8},
        {'R', 4, 4, 9},
        {'T', 3, 4, 10},
        {'X', 2, 4, 11},
        {'R', 1, 4, 12},
        {'T', 1, 3, 13},
        {'X', 0, 3, -1},
    };

    // Terceira fase
    Sala fase3_salas[] = {
        {'E', 4, 0, 1},
        {'T', 4, 1, 2},
        {'X', 4, 2, 3},
        {'R', 3, 2, 4},
        {'T', 3, 3, 5},
        {'T', 3, 4, 6},
        {'X', 2, 4, 7},
        {'T', 1, 4, 8},
        {'R', 0, 4, 9},
        {'T', 0, 3, 10},
        {'X', 0, 2, 11},
        {'R', 0, 1, 12},
        {'T', 1, 1, 13},
        {'X', 1, 0, -1},
    };

    // Quarta fase
    Sala fase4_salas[] = {
        {'E', 4, 0, 1},
        {'T', 3, 0, 2},
        {'T', 2, 0, 3},
        {'R', 1, 0, 4},
        {'X', 0, 0, 5},
        {'T', 0, 1, 6},
        {'T', 0, 2, 7},
        {'X', 1, 2, 8},
        {'R', 2, 2, 9},
        {'T', 3, 2, 10},
        {'T', 3, 3, 11},
        {'X', 3, 4, 12},
        {'R', 2, 4, 13},
        {'X', 1, 4, -1},
    };

    // Quinta fase
    Sala fase5_salas[] = {
        {'E', 0, 0, 1},
        {'T', 1, 0, 2},
        {'T', 2, 0, 3},
        {'X', 3, 0, 4},
        {'T', 4, 0, 5},
        {'T', 4, 1, 6},
        {'R', 4, 2, 7},
        {'X', 3, 2, 8},
        {'T', 2, 2, 9},
        {'T', 1, 2, 10},
        {'R', 0, 2, 11},
        {'T', 0, 3, 12},
        {'X', 0, 4, 13},  
        {'T', 1, 4, 14},
        {'T', 2, 4, 15},
        {'R', 3, 4, 16},
        {'X', 4, 4, -1}
    };

    // Criação dos mapas
    Mapa fase1 = construirFase(
        "Estacao da Lua Phobos",
        fase1_salas,
        sizeof(fase1_salas) / sizeof(fase1_salas[0])
    );

    Mapa fase2 = construirFase(
        "Estacao da Lua Deimos",
        fase2_salas,
        sizeof(fase2_salas) / sizeof(fase2_salas[0])
    );

    Mapa fase3 = construirFase(
        "Planeta Anao Ceres",
        fase3_salas,
        sizeof(fase3_salas) / sizeof(fase3_salas[0])
    );

    Mapa fase4 = construirFase(
        "Cidade do Asteroide Vesta",
        fase4_salas,
        sizeof(fase4_salas) / sizeof(fase4_salas[0])
    );

    Mapa fase5 = construirFase(
        "Lua Ganimedes",
        fase5_salas,
        sizeof(fase5_salas) / sizeof(fase5_salas[0])
    );

    // Inimigos - Fase 1
    fase1.inimigos[0] = (Inimigo){"Hopper", SAMURAI, 120, 120, 10, 10, 5, 5};
    fase1.inimigos[1] = (Inimigo){"Blake", ATIRADOR, 80, 80, 5, 5, 10, 10};
    fase1.inimigos[2] = (Inimigo){"Ralph", CIBORGUE, 150, 150, 5, 15, 0, 5};
    fase1.boss = (Inimigo){"Asterion", CIBORGUE, 180, 180, 10, 15, 5, 5};

    // Inimigos - Fase 2
    fase2.inimigos[0] = (Inimigo){"Sullivan", SAMURAI, 120, 120, 15, 10, 5, 5};
    fase2.inimigos[1] = (Inimigo){"Spencer", CIBORGUE, 150, 150, 8, 15, 0, 5};
    fase2.inimigos[2] = (Inimigo){"Edgar", CIBORGUE, 150, 150, 8, 15, 0, 5};
    fase2.boss = (Inimigo){"Maia", SAMURAI, 160, 160, 15, 12, 8, 5};

    // Inimigos - Fase 3
    fase3.inimigos[0] = (Inimigo){"Marshall", ATIRADOR, 90, 90, 8, 5, 12, 10};
    fase3.inimigos[1] = (Inimigo){"Ron", SAMURAI, 130, 130, 15, 12, 5, 8};
    fase3.inimigos[2] = (Inimigo){"Luke", CIBORGUE, 160, 160, 8, 15, 2, 8};
    fase3.boss = (Inimigo){"Plutus", ATIRADOR, 180, 180, 12, 8, 15, 12};

    // Inimigos - Fase 4
    fase4.inimigos[0] = (Inimigo){"Bond", CIBORGUE, 170, 170, 10, 15, 2, 8};
    fase4.inimigos[1] = (Inimigo){"Maximus", SAMURAI, 140, 140, 15, 12, 8, 10};
    fase4.inimigos[2] = (Inimigo){"Gustaf", ATIRADOR, 100, 100, 10, 8, 12, 12};
    fase4.boss = (Inimigo){"Argos", CIBORGUE, 220, 220, 15, 15, 5, 10};

    // Inimigos - Fase 5
    fase5.inimigos[0] = (Inimigo){"Bruce", SAMURAI, 150, 150, 15, 15, 10, 10};
    fase5.inimigos[1] = (Inimigo){"Klaus", ATIRADOR, 120, 120, 12, 8, 15, 15};
    fase5.inimigos[2] = (Inimigo){"Vivien", CIBORGUE, 200, 200, 12, 15, 5, 10};
    fase5.boss = (Inimigo){"Urano", CIBORGUE, 250, 250, 15, 15, 10, 15};

    return 0;
}

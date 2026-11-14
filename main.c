#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_SALAS 15
#define LINHAS 5
#define COLUNAS 5

typedef enum {
    ATIRADOR = 1,
    SAMURAI,
    CIBORGUE
} Classe;

typedef enum {
    CHECARSTATUS = 1,
    CHECAREQUIPAMENTO,
    VERIFICARMAPA,
    PROXIMAFASE
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
    int vida;
    int forca;
    int defesa;
    int agilidade;
    int inteligencia;
} Equipamento;

typedef struct {
    char tipo;
    int linha;
    int coluna;
    int proxima;
} Sala;

typedef struct {
    char nome[25];
    Sala salas[MAX_SALAS];
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
        printf(">===========================<\n");
        printf("    [Qual decisao tomar?]    \n");
        printf(">===========================<\n");
        printf("[1] - Checar meus status     \n");
        printf("[2] - Checar meu equipamento \n");
        printf("[3] - Verificar o mapa       \n");
        printf("[4] - Ir para a proxima fase \n");
        printf("Digite qual a sua acao: ");
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
            case PROXIMAFASE:
                limparTela();
                avancarSala(mapa); 
                break;
            
            default: limparTela(); break;
        }
    } while(escolha > CHECARSTATUS && escolha < PROXIMAFASE);
    
}

int GirarDado() {
    int min = 1;
    int max = 6;

    return rand() % (max - min + 1) + min;
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
    
    Sala fase1_salas[] = {
        {'R', 0, 0, 1},
        {'O', 0, 1, 2},
        {'O', 0, 2, 3},
        {'X', 0, 3, 4},

        {'O', 1, 3, 5},
        {'O', 2, 3, 6},
        {'O', 2, 2, 7},
        {'O', 3, 2, -1},
    };

    Mapa fase1 = construirFase("Fase1", fase1_salas, 8);
    construirVisualFase(&fase1);

    menuPadrao(&fase1, &personagem);

    return 0;
}

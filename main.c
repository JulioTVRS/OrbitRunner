#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_SALAS 20
#define LINHAS 5
#define COLUNAS 5

#define MAX_INIMIGOS 3
#define RECOMPENSA_INIMIGO 30
#define RECOMPENSA_BOSS 100

// Balanceamento do combate
#define MULT_NORMAL          3   // dano normal  = atributo * 3 + d6 - defesa_alvo/2
#define MULT_ESPECIAL        4   // dano especial = (atr1 + atr2) * 4 + d6 - defesa_alvo/2
#define MULT_ESPECIAL_BOSS   2 
#define RECARGA_ESPECIAL     3   // habilidade especial a cada 3 rounds
#define CHANCE_DEFESA_INIMIGO 25 // % de chance do inimigo escolher defender
#define ESQUIVA_POR_AGI      2   // % de esquiva por ponto de agilidade

// Balanceamento da economia
#define TESOURO_DINHEIRO_BASE 20 // tesouro da $20 + (d6 * 5)
#define TESOURO_SUCATA_MAX    3  // tesouro da de 1 ate 3 sucatas
#define PRECO_SUCATA         10  // $ recebido por cada sucata vendida
#define CUSTO_POR_PONTO       3  // custo de melhoria = (atributo + 5) * 6
#define GANHO_VIDA           15  // cada melhoria de vida da +15 de vida maxima
#define LIMITE_ATRIBUTO      30  // limite de forca, defesa, agilidade e inteligencia
#define LIMITE_VIDA         300  // limite de vida maxima

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

typedef enum {
    POSTO_DESCANSAR = 1,
    POSTO_VENDER,
    POSTO_MELHORAR,
    POSTO_MAPA,
    POSTO_SEGUIR
} OpcaoMenuPosto;

typedef enum {
    MELHORIA_VIDA = 1,
    MELHORIA_FORCA,
    MELHORIA_DEFESA,
    MELHORIA_AGILIDADE,
    MELHORIA_INTELIGENCIA,
    MELHORIA_VOLTAR
} OpcaoMelhoria;

typedef enum {
    ACAO_ATACAR = 1,
    ACAO_DEFENDER,
    ACAO_ESPECIAL
} AcaoCombate;

typedef enum {
    CONTINUA = 0,
    FASE_CONCLUIDA,
    JOGADOR_MORREU
} ResultadoSala;

typedef enum {
    SALA_ENTRADA = 'E',
    SALA_TESOURO = 'T',
    SALA_REABASTECIMENTO = 'R',
    SALA_INIMIGO = 'X'
} TipoSala;

typedef enum {
    PHOBOS,
    DEIMOS,
    CERES,
    VESTA,
    GANIMEDES
} NOMEFASES;

typedef struct {
    char nome[25];
    Classe classe;
    int vida, vidaMax;
    int forca;
    int defesa;
    int agilidade;
    int inteligencia;
    char habilidadeEspecial[20];
    int dinheiro;
    int sucata;
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
    TipoSala tipo;
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

typedef struct {
    const char *nome;
    Classe classe;
    int *vida;
    int forca, defesa, agilidade, inteligencia;
    int defendendo;
    int recarga;
    int multEspecial;
} Lutador;

void limparTela() {
    printf("\033[2J\033[H");
}

int lerInteiro() {
    char buffer[32];
    char *fim;

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        exit(0);
    }

    long valor = strtol(buffer, &fim, 10);
    if (fim == buffer) {
        return -1;
    }
    return (int)valor;
}

int girarDado(int qtdLados) {
    int min = 1;
    return rand() % (qtdLados - min + 1) + min;
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

char* obterNomeHabilidade(Classe classe) {
    switch (classe)
    {
        case ATIRADOR: return "Rajada de tiros";
        case SAMURAI: return "Golpe em espiral";
        case CIBORGUE: return "Canhao de plasma";
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
    printf("%15s  $%d\n", "[Dinheiro]", personagem->dinheiro);
    printf("%15s  %d\n", "[Sucata]", personagem->sucata);
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
    printf("e cai em um grande prejuizo financeiro, mas recebe uma proposta de quitacao da divida: o proprio rei de Marte\n");
    printf("o propoe de acabar com todos os mercenarios espaciais da gangue Space Hounds para perdoar o que voce devia.\n");
}

void aguardarEnter() {
    int enter = 0;
    printf(">=============================<\n");
    printf("Pressione <Enter> para continuar...\n");
    while ((enter = getchar()) != '\n' && enter != EOF);
}

void criarJogador(Personagem *personagem) {
    Personagem classes[] = {
        {"", ATIRADOR, 80, 80, 5, 5, 10, 10, "Rajada de tiros", 0, 0},
        {"", SAMURAI, 120, 120, 10, 10, 5, 5, "Golpe em espiral", 0, 0},
        {"", CIBORGUE, 150, 150, 5, 15, 0, 5, "Canhao de plasma", 0, 0}
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
    escolha_classe = lerInteiro();

    while (escolha_classe > 3 || escolha_classe < 1) {
        printf(">===============<\n");
        printf("Classe inexistente, Tente novamente!\n");
        printf("Digite o numero da sua classe: ");
        escolha_classe = lerInteiro();
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

// Parte do combate
// Dano = atributo base + d6 - metade da defesa do alvo (minimo 1)
int calcularDano(int base, int defesaAlvo) {
    int dano = base + girarDado(6) - defesaAlvo / 2;
    return dano < 1 ? 1 : dano;
}

// Ataque normal: atirador usa inteligencia, samurai e ciborgue usam forca
int danoNormal(Lutador *atacante, Lutador *alvo) {
    int atributo = (atacante->classe == ATIRADOR) ? atacante->inteligencia : atacante->forca;
    return calcularDano(atributo * MULT_NORMAL, alvo->defesa);
}

// Habilidade especial: atirador int+agi, samurai forca+agi, ciborgue defesa+int
int danoEspecial(Lutador *atacante, Lutador *alvo) {
    int soma;
    switch (atacante->classe)
    {
        case ATIRADOR: soma = atacante->inteligencia + atacante->agilidade; break;
        case SAMURAI:  soma = atacante->forca + atacante->agilidade; break;
        default:       soma = atacante->defesa + atacante->inteligencia; break;
    }
    return calcularDano(soma * atacante->multEspecial, alvo->defesa);
}

void executarAcao(Lutador *atacante, Lutador *alvo, int acao) {
    int dano;

    if (acao == ACAO_DEFENDER) {
        return; // a defesa ja foi aplicada na escolha das acoes
    }

    if (acao == ACAO_ESPECIAL) {
        printf("%s usa %s!\n", atacante->nome, obterNomeHabilidade(atacante->classe));
        dano = danoEspecial(atacante, alvo);
        atacante->recarga = RECARGA_ESPECIAL;
    }
    else {
        printf("%s ataca %s!\n", atacante->nome, alvo->nome);
        if (girarDado(100) <= alvo->agilidade * ESQUIVA_POR_AGI) {
            printf("%s esquivou do ataque!\n", alvo->nome);
            return;
        }
        dano = danoNormal(atacante, alvo);
    }

    if (alvo->defendendo) {
        dano /= 2;
        if (dano < 1) dano = 1;
        printf("%s se defendeu e reduziu o dano pela metade!\n", alvo->nome);
    }

    *alvo->vida -= dano;
    if (*alvo->vida < 0) *alvo->vida = 0;
    printf("%s sofreu %d de dano.\n", alvo->nome, dano);
}

int escolherAcaoJogador(Lutador *jogador) {
    int acao;

    do {
        printf("Digite a sua acao: ");
        acao = lerInteiro();

        if (acao == ACAO_ESPECIAL && jogador->recarga > 0) {
            printf("Habilidade especial em recarga (%d round(s))!\n", jogador->recarga);
            acao = 0;
        }
    } while (acao < ACAO_ATACAR || acao > ACAO_ESPECIAL);

    return acao;
}

int escolherAcaoInimigo(Lutador *inimigo, int Boss) {
    if (Boss && inimigo->recarga == 0) {
        return ACAO_ESPECIAL;
    }
    if (girarDado(100) <= CHANCE_DEFESA_INIMIGO) {
        return ACAO_DEFENDER;
    }
    return ACAO_ATACAR;
}

// Retorna 1 se o jogador venceu, 0 se foi derrotado
int combate(Personagem *personagem, Inimigo *inimigo, int ehBoss) {
    Lutador jogador = {
        personagem->nome, personagem->classe, &personagem->vida,
        personagem->forca, personagem->defesa, personagem->agilidade, personagem->inteligencia,
        0, RECARGA_ESPECIAL - 1, MULT_ESPECIAL
    };
    Lutador oponente = {
        inimigo->nome, inimigo->classe, &inimigo->vida,
        inimigo->forca, inimigo->defesa, inimigo->agilidade, inimigo->inteligencia,
        0, RECARGA_ESPECIAL - 1, ehBoss ? MULT_ESPECIAL_BOSS : MULT_ESPECIAL
    };

    int rodada = 1;
    int jogadorPrimeiro = personagem->agilidade >= inimigo->agilidade;

    while (personagem->vida > 0 && inimigo->vida > 0) {
        printf("\n>===========================<\n");
        printf("         [Round %d]          \n", rodada);
        printf(">===========================<\n");
        printf("Sua vida: %d / %d\n", personagem->vida, personagem->vidaMax);
        printf("Vida do %s: %d / %d\n", inimigo->nome, inimigo->vida, inimigo->vidaMax);
        printf("\n");
        printf("[1] - Atacar\n");
        printf("[2] - Defender\n");
        if (jogador.recarga == 0) {
            printf("[3] - Hab. Especial (%s)\n", obterNomeHabilidade(jogador.classe));
        }
        else {
            printf("[3] - Hab. Especial (em recarga: %d round(s))\n", jogador.recarga);
        }

        int acaoJogador = escolherAcaoJogador(&jogador);
        int acaoInimigo = escolherAcaoInimigo(&oponente, ehBoss);

        // Defesas valem para o round inteiro, independente de quem age primeiro
        jogador.defendendo = (acaoJogador == ACAO_DEFENDER);
        oponente.defendendo = (acaoInimigo == ACAO_DEFENDER);

        printf("\n");
        if (jogador.defendendo) printf("%s assume postura defensiva.\n", jogador.nome);
        if (oponente.defendendo) printf("%s assume postura defensiva.\n", oponente.nome);

        // Quem tem mais agilidade age primeiro
        if (jogadorPrimeiro) {
            executarAcao(&jogador, &oponente, acaoJogador);
            if (inimigo->vida > 0) {
                executarAcao(&oponente, &jogador, acaoInimigo);
            }
        }
        else {
            executarAcao(&oponente, &jogador, acaoInimigo);
            if (personagem->vida > 0) {
                executarAcao(&jogador, &oponente, acaoJogador);
            }
        }

        if (jogador.recarga > 0) jogador.recarga--;
        if (oponente.recarga > 0) oponente.recarga--;
        rodada++;

        if (personagem->vida > 0 && inimigo->vida > 0) {
            aguardarEnter();
            limparTela();
        }
    }

    return personagem->vida > 0;
}

// Retorna 1 se o jogador venceu a luta, 0 se morreu
int salaInimigo(Mapa *mapa, int *inimigosEncontrados, Personagem *personagem) {
    int Boss = (mapa->salas[mapa->jogador].proxima == -1);
    Inimigo *inimigo;

    if (Boss) {
        inimigo = &mapa->boss;
    }
    else {
        inimigo = &mapa->inimigos[*inimigosEncontrados];
        (*inimigosEncontrados)++;
    }

    printf(">===========================<\n");
    if (Boss) {
        printf("Voce encontra o CHEFE da fase!\n");
        printf("[Lynx]: Cuidado! Esse eh o %s, um %s e o lider dessa base dos Space Hounds.\n",
               inimigo->nome, obterNomeClasse(inimigo->classe));
    }
    else {
        printf("Voce encontra um inimigo!\n");
        printf("[Lynx]: Esse eh o %s, um %s da gangue dos Space Hounds.\n",
               inimigo->nome, obterNomeClasse(inimigo->classe));
    }
    printf("Voce entra em combate.\n");
    aguardarEnter();
    limparTela();

    int venceu = combate(personagem, inimigo, Boss);

    printf("\n>===========================<\n");
    if (venceu) {
        int recompensa = Boss ? RECOMPENSA_BOSS : RECOMPENSA_INIMIGO;

        if (Boss) {
            printf("Voce derrotou o chefe %s!\n", inimigo->nome);
        }
        else {
            printf("Voce derrotou %s!\n", inimigo->nome);
        }

        personagem->dinheiro += recompensa;

        printf("Voce recebeu $%d pela vitoria!\n", recompensa);
    }
    else {
        printf("%s derrotou voce...\n", inimigo->nome);
    }

    return venceu;
}

// Construção visual do mapa
void construirVisualFase(Mapa *mapa) {
    printf(">=============================<\n");
    printf("[Fase: %s]\n", mapa->nome);
    printf(">=============================<\n");
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
        gradeMapa[linha][coluna] = (char)mapa->salas[i].tipo;
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

// Posto de reabastecimento
// Devolve um ponteiro para o atributo que a melhoria altera
int* obterAtributo(Personagem *personagem, int opcao) {
    switch (opcao)
    {
        case MELHORIA_VIDA:         return &personagem->vidaMax;
        case MELHORIA_FORCA:        return &personagem->forca;
        case MELHORIA_DEFESA:       return &personagem->defesa;
        case MELHORIA_AGILIDADE:    return &personagem->agilidade;
        case MELHORIA_INTELIGENCIA: return &personagem->inteligencia;
        default:                    return NULL;
    }
}

const char* obterNomeAtributo(int opcao) {
    switch (opcao)
    {
        case MELHORIA_VIDA:         return "Vida maxima";
        case MELHORIA_FORCA:        return "Forca";
        case MELHORIA_DEFESA:       return "Defesa";
        case MELHORIA_AGILIDADE:    return "Agilidade";
        case MELHORIA_INTELIGENCIA: return "Inteligencia";
        default:                    return "Inexistente";
    }
}

int obterGanhoMelhoria(int opcao) {
    return (opcao == MELHORIA_VIDA) ? GANHO_VIDA : 1;
}

int obterLimiteMelhoria(int opcao) {
    return (opcao == MELHORIA_VIDA) ? LIMITE_VIDA : LIMITE_ATRIBUTO;
}

// Quanto mais forte o atributo, mais caro melhorar
int calcularCustoMelhoria(int opcao, int valorAtual) {
    if (opcao == MELHORIA_VIDA) {
        return valorAtual / 2;
    }
    return (valorAtual + 5) * CUSTO_POR_PONTO;
}

void comprarMelhoria(Personagem *personagem, int opcao) {
    int *atributo = obterAtributo(personagem, opcao);
    int ganho = obterGanhoMelhoria(opcao);
    int custo = calcularCustoMelhoria(opcao, *atributo);

    if (*atributo >= obterLimiteMelhoria(opcao)) {
        printf("%s ja esta no maximo!\n", obterNomeAtributo(opcao));
        return;
    }

    if (personagem->dinheiro < custo) {
        printf("Dinheiro insuficiente! Voce precisa de $%d.\n", custo);
        return;
    }

    personagem->dinheiro -= custo;
    *atributo += ganho;

    // Aumentar a vida maxima tambem aumenta a vida atual
    if (opcao == MELHORIA_VIDA) {
        personagem->vida += ganho;
    }

    printf("Equipamento melhorado! %s agora eh %d (gasto: $%d).\n",
           obterNomeAtributo(opcao), *atributo, custo);
}

void menuMelhoria(Personagem *personagem) {
    int escolha;

    limparTela();
    do {
        printf(">===============================<\n");
        printf("     [Melhorar Equipamento]      \n");
        printf(">===============================<\n");
        printf("Dinheiro: $%d\n", personagem->dinheiro);
        printf("\n");

        for (int opcao = MELHORIA_VIDA; opcao <= MELHORIA_INTELIGENCIA; opcao++) {
            int *atributo = obterAtributo(personagem, opcao);

            printf("[%d] - %-13s %3d -> %3d", opcao, obterNomeAtributo(opcao),
                   *atributo, *atributo + obterGanhoMelhoria(opcao));

            if (*atributo >= obterLimiteMelhoria(opcao)) {
                printf("  (MAXIMO)\n");
            }
            else {
                printf("  custo: $%d\n", calcularCustoMelhoria(opcao, *atributo));
            }
        }
        printf("[%d] - Voltar\n", MELHORIA_VOLTAR);
        printf("Digite a sua acao: ");
        escolha = lerInteiro();

        limparTela();
        if (escolha >= MELHORIA_VIDA && escolha <= MELHORIA_INTELIGENCIA) {
            comprarMelhoria(personagem, escolha);
        }
    } while (escolha != MELHORIA_VOLTAR);
}

void menuReabastecimento(Mapa *mapa, Personagem *personagem) {
    int escolha;

    do {
        printf(">===============================<\n");
        printf("    [Posto de Reabastecimento]   \n");
        printf(">===============================<\n");
        printf("Vida: %d / %d   Dinheiro: $%d   Sucata: %d\n",
               personagem->vida, personagem->vidaMax, personagem->dinheiro, personagem->sucata);
        printf("\n");
        printf("[1] - Descansar (recupera toda a vida)\n");
        printf("[2] - Vender sucata ($%d cada)\n", PRECO_SUCATA);
        printf("[3] - Melhorar equipamento\n");
        printf("[4] - Verificar o mapa\n");
        printf("[5] - Seguir para o proximo destino\n");
        printf("Digite a sua acao: ");
        escolha = lerInteiro();

        switch (escolha)
        {
            case POSTO_DESCANSAR:
                limparTela();
                if (personagem->vida >= personagem->vidaMax) {
                    printf("Voce ja esta com a vida cheia.\n");
                }
                else {
                    personagem->vida = personagem->vidaMax;
                    printf("Voce descansou e recuperou toda a sua vida!\n");
                }
                break;
            case POSTO_VENDER:
                limparTela();
                if (personagem->sucata <= 0) {
                    printf("Voce nao tem sucata para vender.\n");
                }
                else {
                    int ganho = personagem->sucata * PRECO_SUCATA;
                    printf("Voce vendeu %d sucata(s) por $%d.\n", personagem->sucata, ganho);
                    personagem->dinheiro += ganho;
                    personagem->sucata = 0;
                }
                break;
            case POSTO_MELHORAR:
                menuMelhoria(personagem);
                break;
            case POSTO_MAPA:
                limparTela();
                construirVisualFase(mapa);
                break;
            case POSTO_SEGUIR:
                break;
            default:
                limparTela();
                break;
        }
    } while (escolha != POSTO_SEGUIR);
}

ResultadoSala avancarSala(Mapa *mapa, int *inimigosEncontrados, Personagem *personagem) {
    int proxima = mapa->salas[mapa->jogador].proxima;

    printf(">==================================<\n");
    printf("Voce avanca para o proximo destino\n");

    mapa->jogador = proxima;

    switch (mapa->salas[mapa->jogador].tipo)
    {
        case SALA_INIMIGO: {
            int venceu = salaInimigo(mapa, inimigosEncontrados, personagem);
            if (!venceu) {
                return JOGADOR_MORREU;
            }
            aguardarEnter();
            limparTela();
            if (mapa->salas[mapa->jogador].proxima == -1) {
                return FASE_CONCLUIDA;
            }
            break;
        }
        case SALA_REABASTECIMENTO:
            printf("Voce chegou a um posto de reabastecimento.\n");
            menuReabastecimento(mapa, personagem);
            limparTela();
            return avancarSala(mapa, inimigosEncontrados, personagem);
        case SALA_TESOURO: {
            int sucata = girarDado(TESOURO_SUCATA_MAX);
            int dinheiro = TESOURO_DINHEIRO_BASE + girarDado(6) * 5;

            personagem->sucata += sucata;
            personagem->dinheiro += dinheiro;

            printf("Voce encontrou um local com tesouros!\n");
            printf("Voce recebeu $%d e %d sucata(s).\n", dinheiro, sucata);
            break;
        }
        case SALA_ENTRADA:
        default:
            break;
    }

    return CONTINUA;
}

// Roda uma fase inteira. Retorna FASE_CONCLUIDA ou JOGADOR_MORREU
ResultadoSala executarFase(Mapa *mapa, Personagem *personagem) {
    int inimigosEncontrados = 0;
    int escolha;

    mapa->jogador = 0;
    limparTela();
    construirVisualFase(mapa);

    while (1) {
        printf(">===============================<\n");
        printf("      [Qual decisao tomar?]      \n");
        printf(">===============================<\n");
        printf("[1] - Checar meus status         \n");
        printf("[2] - Checar meu equipamento     \n");
        printf("[3] - Verificar o mapa           \n");
        printf("[4] - Ir para o proximo destino  \n");
        printf("Digite a sua acao: ");
        escolha = lerInteiro();

        switch (escolha)
        {
            case CHECARSTATUS:
                limparTela();
                mostrarStatus(personagem);
                break;
            case CHECAREQUIPAMENTO:
                limparTela();
                printf("Sistema de equipamentos ainda nao disponivel.\n");
                break;
            case VERIFICARMAPA:
                limparTela();
                construirVisualFase(mapa);
                break;
            case PROXIMASALA: {
                limparTela();
                ResultadoSala resultado = avancarSala(mapa, &inimigosEncontrados, personagem);
                if (resultado != CONTINUA) {
                    return resultado;
                }
                construirVisualFase(mapa);
                break;
            }
            default:
                limparTela();
                break;
        }
    }
}

void apresentarTutorial() {
    printf(">==========================<\n"
           "         [Tutorial]         \n"
           ">==========================<\n"
           "[Lynx]: Oi, eu sou o robo-guia da sua nave, lembra de mim?\n"
           "[Lynx]: Enfim, voce ira enfrentar varios mapas com recompensas, inimigos\n"
           "e locais para se curar ou melhorar seus equipamentos. O mapa apresenta\n"
           "quadrados com letras, que representam cada um desses locais.\n"
           "[Lynx]: Sao eles:\n"
           "[E] - Inicio.\n"
           "[T] - Tesouro.\n"
           "[R] - Posto de reabastecimento.\n"
           "[X] - Inimigos e Bosses.\n\n"
           "[Lynx]: Alem disso, eh importante saber que, no dano dos ataques\n"
           "normais, samurais e ciborgues usam forca, enquanto atiradores usam\n"
           "inteligencia. Ja no dano das habilidades especiais, samurais usam\n"
           "forca + agilidade, ciborgues usam forca + defesa e atiradores usam\n"
           "inteligencia + agilidade.\n"
           "[Lynx]: Como o rei disse, se voce derrubar todos os lideres das bases\n"
           "da Space Hounds, voce nao vai ter que pagar pelo estrago que fez.\n"
           "[Lynx]: Essa brincadeira vai ser dificil, qualquer coisa estarei\n"
           "por aqui para tirar duvidas.\n");
}

void dialogoInicioFase(int fase) {
    printf(">=========================================<\n");
    switch(fase) {
        case PHOBOS:
            printf("[Lynx]: Chegamos no nosso primeiro destino, o rei me entregou o mapa\n"
                    "das areas dominadas que a gente vai ter que libertar, e vamos comecar\n"
                    "por aqui.\n"
                    "[Lynx]: Essa eh Phobos, a maior lua de marte, acho que a gente\n"
                    "nunca veio aqui antes. Pelos resultados dos rumores da rede, quem\n"
                    "controla essa area eh o Asterion, um ciborgue meio medonho.\n");
            break;

        case DEIMOS:
            printf("[Lynx]: Chegamos em Deimos, parece que as pessoas vivem em uma cidade\n"
                    "subterranea, melhor tomar cuidado. Na rede nao existe nada sobre\n"
                    "o lider dessa area, eles provavelmente dominam ela nas sombras, tal\n"
                    "qual um samurai?\n");
            break;

        case CERES:
            printf("[Lynx]: Ja consigo ver Ceres, o planeta anao mais proximo de casa.\n"
                    "[Lynx]: Eh de se imaginar que eles dominariam essa area, esse lugar onde\n"
                    "estamos, o cinturao de asteroides, nunca foi muito bem conhecido pela\n"
                    "sua seguranca.\n"
                    "[Lynx]: Ah, e o lider daqui eh o Plutus, dizem que ele nunca erra um tiro.\n");
            break;

        case VESTA:
            printf("[Lynx]: Estamos na nossa penultima parada, o asteroide Vesta. Pelo que eu sei\n"
                    "ele eh bem luxuoso. E parece que Argos, o lider daqui, ja era bem rico antes\n"
                    "de entrar pros Hounds.\n"
                    "[Lynx]: Mais um casca grossa, parece ele eh um ciborgue que nem o Asterion.\n");
            break;

        case GANIMEDES:
            printf("[Lynx]: Aqui eh o fim da linha, chegamos finalmente em Ganimedes, o lar do\n"
                    "lider supremo da Space Hounds, Urano. Os dados na rede revelam que ele foi\n"
                    "um samurai muito habilidoso de Marte.\n"
                    "[Lynx]: Ja eh meio tarde mas eu me pergunto qual motivacao o levou a virar um\n"
                    "mercenario e vir parar aqui.\n"
                    "[Lynx]: Mesmo sendo bem otimista, isso aqui ainda vai ser ralado, boa sorte.\n");
            break;

        default:
            printf("Local nao implementado.\n");
            break;
    }
}

int main() {
    srand(time(NULL));
    limparTela();

    Personagem personagem;

    printf(">=============================<\n");
    printf("     -[[  OrbitRunner  ]]-     \n");
    printf(">=============================<\n");

    criarJogador(&personagem);
    aguardarEnter();
    limparTela();

    apresentarHistoria();
    aguardarEnter();
    limparTela();

    apresentarTutorial();
    aguardarEnter();
    limparTela();

    // Primeira fase
    Sala fase1_salas[] = {
        {SALA_ENTRADA, 0, 0, 1},
        {SALA_TESOURO, 0, 1, 2},
        {SALA_TESOURO, 0, 2, 3},
        {SALA_INIMIGO, 0, 3, 4},
        {SALA_REABASTECIMENTO, 0, 4, 5},
        {SALA_TESOURO, 1, 4, 6},
        {SALA_TESOURO, 2, 4, 7},
        {SALA_INIMIGO, 2, 3, 8},
        {SALA_REABASTECIMENTO, 2, 2, 9},
        {SALA_TESOURO, 2, 1, 10},
        {SALA_TESOURO, 3, 1, 11},
        {SALA_INIMIGO, 4, 1, 12},
        {SALA_REABASTECIMENTO, 4, 2, 13},
        {SALA_INIMIGO, 4, 3, -1}
    };

    // Segunda fase
    Sala fase2_salas[] = {
        {SALA_ENTRADA, 0, 0, 1},
        {SALA_TESOURO, 1, 0, 2},
        {SALA_INIMIGO, 2, 0, 3},
        {SALA_REABASTECIMENTO, 2, 1, 4},
        {SALA_TESOURO, 3, 1, 5},
        {SALA_TESOURO, 4, 1, 6},
        {SALA_INIMIGO, 4, 2, 7},
        {SALA_TESOURO, 4, 3, 8},
        {SALA_REABASTECIMENTO, 4, 4, 9},
        {SALA_TESOURO, 3, 4, 10},
        {SALA_INIMIGO, 2, 4, 11},
        {SALA_REABASTECIMENTO, 1, 4, 12},
        {SALA_TESOURO, 1, 3, 13},
        {SALA_INIMIGO, 0, 3, -1},
    };

    // Terceira fase
    Sala fase3_salas[] = {
        {SALA_ENTRADA, 4, 0, 1},
        {SALA_TESOURO, 4, 1, 2},
        {SALA_INIMIGO, 4, 2, 3},
        {SALA_REABASTECIMENTO, 3, 2, 4},
        {SALA_TESOURO, 3, 3, 5},
        {SALA_TESOURO, 3, 4, 6},
        {SALA_INIMIGO, 2, 4, 7},
        {SALA_TESOURO, 1, 4, 8},
        {SALA_REABASTECIMENTO, 0, 4, 9},
        {SALA_TESOURO, 0, 3, 10},
        {SALA_INIMIGO, 0, 2, 11},
        {SALA_REABASTECIMENTO, 0, 1, 12},
        {SALA_TESOURO, 1, 1, 13},
        {SALA_INIMIGO, 1, 0, -1},
    };

    // Quarta fase
    Sala fase4_salas[] = {
        {SALA_ENTRADA, 4, 0, 1},
        {SALA_TESOURO, 3, 0, 2},
        {SALA_TESOURO, 2, 0, 3},
        {SALA_REABASTECIMENTO, 1, 0, 4},
        {SALA_INIMIGO, 0, 0, 5},
        {SALA_TESOURO, 0, 1, 6},
        {SALA_TESOURO, 0, 2, 7},
        {SALA_INIMIGO, 1, 2, 8},
        {SALA_REABASTECIMENTO, 2, 2, 9},
        {SALA_TESOURO, 3, 2, 10},
        {SALA_TESOURO, 3, 3, 11},
        {SALA_INIMIGO, 3, 4, 12},
        {SALA_REABASTECIMENTO, 2, 4, 13},
        {SALA_INIMIGO, 1, 4, -1},
    };

    // Quinta fase
    Sala fase5_salas[] = {
        {SALA_ENTRADA, 0, 0, 1},
        {SALA_TESOURO, 1, 0, 2},
        {SALA_TESOURO, 2, 0, 3},
        {SALA_INIMIGO, 3, 0, 4},
        {SALA_TESOURO, 4, 0, 5},
        {SALA_TESOURO, 4, 1, 6},
        {SALA_REABASTECIMENTO, 4, 2, 7},
        {SALA_INIMIGO, 3, 2, 8},
        {SALA_TESOURO, 2, 2, 9},
        {SALA_TESOURO, 1, 2, 10},
        {SALA_REABASTECIMENTO, 0, 2, 11},
        {SALA_TESOURO, 0, 3, 12},
        {SALA_INIMIGO, 0, 4, 13},
        {SALA_TESOURO, 1, 4, 14},
        {SALA_TESOURO, 2, 4, 15},
        {SALA_REABASTECIMENTO, 3, 4, 16},
        {SALA_INIMIGO, 4, 4, -1}
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
    fase2.boss = (Inimigo){"Maia", SAMURAI, 170, 170, 15, 12, 8, 5};

    // Inimigos - Fase 3
    fase3.inimigos[0] = (Inimigo){"Marshall", ATIRADOR, 110, 110, 9, 5, 12, 10};
    fase3.inimigos[1] = (Inimigo){"Ron", SAMURAI, 150, 150, 15, 12, 5, 8};
    fase3.inimigos[2] = (Inimigo){"Luke", CIBORGUE, 180, 180, 8, 15, 2, 8};
    fase3.boss = (Inimigo){"Plutus", ATIRADOR, 200, 200, 12, 8, 15, 12};

    // Inimigos - Fase 4
    fase4.inimigos[0] = (Inimigo){"Bond", CIBORGUE, 185, 185, 10, 15, 2, 8};
    fase4.inimigos[1] = (Inimigo){"Maximus", SAMURAI, 155, 155, 14, 12, 8, 10};
    fase4.inimigos[2] = (Inimigo){"Gustaf", ATIRADOR, 112, 112, 10, 8, 12, 13};
    fase4.boss = (Inimigo){"Argos", CIBORGUE, 265, 265, 15, 16, 5, 12};

    // Inimigos - Fase 5
    fase5.inimigos[0] = (Inimigo){"Bruce", SAMURAI, 180, 180, 14, 15, 10, 10};
    fase5.inimigos[1] = (Inimigo){"Klaus", ATIRADOR, 140, 140, 11, 8, 15, 15};
    fase5.inimigos[2] = (Inimigo){"Vivien", CIBORGUE, 230, 230, 11, 15, 5, 10};
    fase5.boss = (Inimigo){"Urano", SAMURAI, 285, 285, 16, 16, 12, 15};

    Mapa *fases[] = {&fase1, &fase2, &fase3, &fase4, &fase5};
    int totalFases = sizeof(fases) / sizeof(fases[0]);

    for (int i = 0; i < totalFases; i++) {
        dialogoInicioFase(i);

        aguardarEnter();
        limparTela();

        ResultadoSala resultado = executarFase(fases[i], &personagem);

        if (resultado == JOGADOR_MORREU) {
            aguardarEnter();
            limparTela();
            printf(">=============================<\n");
            printf("          [GAME OVER]          \n");
            printf(">=============================<\n");
            printf("Os Space Hounds foram mais fortes dessa vez...\n");
            aguardarEnter();
            return 0;
        }

        personagem.vida = personagem.vidaMax;

        printf(">=============================<\n");
        printf("Fase \"%s\" concluida!\n", fases[i]->nome);
        aguardarEnter();
        limparTela();
    }

    limparTela();
    printf(">=============================<\n"
       "          [VITORIA]            \n"
       ">=============================<\n"
       "[Lynx]: Voce conseguiu! A gangue Space Hounds foi eliminada\n"
       "e sua divida com o rei de Marte finalmente foi quitada.\n"
       "[Lynx]: Espera... apareceu uma transmissao ao vivo na rede.\n"
       "Voce precisa ver isso.\n"
       "[Lynx]: O rei de Marte esta sendo anunciado como... Rei do Sol?\n"
       "[Lynx]: Agora tudo faz sentido... As bases que nos \"libertamos\"...\n"
       "O exercito marciano estava assumindo o controle de cada uma delas.\n"
       "[Lynx]: Enquanto pensavamos estar libertando esses lugares,\n"
       "estavamos apenas abrindo caminho para a expansao de Marte.\n"
       "[Lynx]: Parece que nada veio de graca, nao eh?\n");
    return 0;
}
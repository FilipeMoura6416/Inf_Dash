#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>
#define MAXCOMP 301
#define MAXALT 40
#define NUM_FRAMES  3
#define MAX_INPUT_CHARS 30
#define LARGURA_TELA 1200
#define ALTURA_TELA 600

#define GRAVIDADE 6000
#define PLAYER_JUMP_SPD 1300.0f
#define PLAYER_HOR_SPD 380.0f
#define ALTURA 30
#define COMPRIMENTO 100
#define B 40
#define H 40
#define PONTUACAO_INICIAL 1000


/* ------------------------------------Defini��o de estruturas-----------------------------------------*/
typedef struct Player
{
    Vector2 position;
    float speed;
    bool canJump;
} Player;


typedef struct EnvItem
{
    Rectangle rect;
    int blocking;
    Color color;
    bool espinho;
    bool fim;
    bool moeda;
    bool pegou;

} EnvItem;


typedef struct BOTAO
{
    Texture2D textura;
    Rectangle moldura;
    Rectangle bordas;
    int estado;    //O estado � usado apenas para a anima��o de movimento do bot�o
    bool acao;

}BOTAO;

typedef struct TEXTURA_MOEDA
{
    Texture2D textura_moeda;
    Rectangle moldura_moeda;
    Rectangle bordas_moeda;
}TEXTURA_MOEDA;


typedef struct JOGADOR
{
    char nome[30];
    int pontos;

}JOGADOR;

/* ------------------------------------Defini��o de estruturas-----------------------------------------*/

/*---------------------------------------Defini��o de fun��es------------------------------------------*/


/*Esta fun��o abre o arquivo texto que contem o mapa do jogo da fase referente e copia para uma matriz*/
void carrega_terreno(char terreno[][MAXCOMP], char fase[10])
{

    int i = 0, a = 0, b = 0, tam;
    char c[MAXCOMP] = {};
    FILE *fp;
    fp = fopen(fase, "r");
    for(int a = 0; a < MAXALT; a++)
    {
        for(int b = 0; b < MAXCOMP; b++)
        {
            terreno[a][b] = '\0';
        }
    }
    if(fp == NULL)
    {
        printf("Erro na abertura do arquivo\n");
    }
    else
    {
        while(!feof(fp))
        {
            if(fgets(c, MAXCOMP, fp) != NULL){

            c[strlen(c) - 1] = '\0';

            tam = strlen(c);

            for(b = 0; b < tam; b++)
            {
                terreno[a][b] = c[b];
            }

            a++;
            for(b = 0; b < tam; b++)
            {
                c[b] = '\0';
            }

            }
            i++;
        }
    }
    fclose(fp);
}


/*Esta fun��o recebe uma matriz com o mapa da fase e cria os blocos e itens do mapa de acordo com o conteudo da matriz*/
void cria_terreno(char terreno[MAXALT][MAXCOMP], EnvItem *obstaculos, int *xinicial, int *yinicial)
{
    int a, b, c = 0;

    for(a = 0; a < MAXALT; a++)
    {
        for(b = 0; b < MAXCOMP; b++)
        {
            switch(terreno[a][b])
            {
                case 'G' :
                        obstaculos[c].rect.x = b * B;
                         obstaculos[c].rect.y = a * H;
                         obstaculos[c].rect.width = B;
                         obstaculos[c].rect.height = H;
                         obstaculos[c].blocking = 1;
                         obstaculos[c].color = BLUE;
                         obstaculos[c].espinho = false;
                         obstaculos[c].fim = false;
                         obstaculos[c].moeda = false;
                         obstaculos[c].pegou = false;
                         c++;
                break;

                case 'S' :
                            obstaculos[c].rect.x = b * B;
                            obstaculos[c].rect.y = a * H;
                            obstaculos[c].rect.width = B;
                            obstaculos[c].rect.height = H;
                            obstaculos[c].blocking = 1;
                            obstaculos[c].color = RED;
                            obstaculos[c].espinho = true;
                            obstaculos[c].fim = false;
                            obstaculos[c].moeda = false;
                            obstaculos[c].pegou = false;
                            c++;
                break;

                case 'F' :
                            obstaculos[c].rect.x = b * B;
                            obstaculos[c].rect.y = a * H;
                            obstaculos[c].rect.width = B;
                            obstaculos[c].rect.height = H;
                            obstaculos[c].blocking = 1;
                            obstaculos[c].color = GREEN;
                            obstaculos[c].espinho = false;
                            obstaculos[c].fim = true;
                            obstaculos[c].moeda = false;
                            obstaculos[c].pegou = false;
                            c++;
                break;

                case 'C' :
                            obstaculos[c].rect.x = b * B;
                            obstaculos[c].rect.y = a * H;
                            obstaculos[c].rect.width = B;
                            obstaculos[c].rect.height = H;
                            obstaculos[c].blocking = 1;
                            obstaculos[c].color = LIGHTGRAY;
                            obstaculos[c].espinho = false;
                            obstaculos[c].fim = false;
                            obstaculos[c].moeda = true;
                            obstaculos[c].pegou = false;
                            c++;
                break;

                case 'I': *xinicial = b * H;
                            *yinicial = (a + 1) * B;
            }
        }
    }
}

/*Esta fun��o verifica a posi��o do player, se ele ir� colidir com algum obst�culo, se ele ir� pegar uma moeda e atualiza os respectivos contadores*/
void UpdatePlayer(Player *player, EnvItem *obstaculos, int obstaculosLength, float delta, int *tentativas, bool *chegou_no_fim, int *moedas, char terreno[MAXALT][MAXCOMP], float *rotation, int *xinicial, int *yinicial)
{
    if (IsKeyDown(KEY_SPACE) && player->canJump)
    {
        player->speed = -PLAYER_JUMP_SPD;
        player->canJump = false;
    }

    bool hitObstacle = false;
    bool parede = false;
    bool bateucabeca = false;
    for (int i = 0; i < obstaculosLength; i++) //Este for faz todas as verifica��es de colis�o seja com blocos, itens ou espinhos
    {
        EnvItem *ei = obstaculos + i;
        Vector2 *p = &(player->position);
        if (ei->blocking &&     //Verifica colis�o de cima pra baixo
            ei->rect.x <= p->x + B &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y >= p->y &&
            ei->rect.y <= p->y + player->speed*delta)
        {
            if(ei->moeda)
            {
                if(!(ei->pegou))
                {
                    *moedas += 1;
                    ei->pegou = true;
                }

                hitObstacle = false;
            }else
            {
                hitObstacle = true;
                player->speed = 0.0f;
                p->y = ei->rect.y;
            }

            if(ei->espinho)
            {
                p->x = *xinicial;
                p->y = *yinicial;
                *tentativas += 1;
                *moedas = 0;
                cria_terreno(terreno, obstaculos, xinicial, yinicial);
            }

        }

        if (ei->blocking &&     //Verifica colis�o de baixo pra cima
            ei->rect.x <= p->x + B &&
            ei->rect.x + ei->rect.width >= p->x &&
            ei->rect.y <= p->y - ALTURA &&
            ei->rect.y > (p->y - ALTURA) + player->speed*delta)
        {
            if(ei->moeda)
            {
                if(!(ei->pegou))
                {
                    *moedas += 1;
                    ei->pegou = true;
                }
                bateucabeca = false;
            }else
                {
                    bateucabeca = true;
                    p->y = ei->rect.y + ei->rect.height + 40;
                }

            if(ei->espinho)
            {
                p->x = *xinicial;
                p->y = *yinicial;
                *tentativas += 1;
                *moedas = 0;
                cria_terreno(terreno, obstaculos, xinicial, yinicial);
            }

        }

        if (ei->blocking &&    //Verifica colis�o frontal
            ei->rect.x > p->x &&
            ei->rect.y < p->y &&
            ei->rect.x <= p->x + PLAYER_HOR_SPD*delta + 40 &&
            ei->rect.y + ei->rect.height >= p->y)
        {
            if(ei->moeda)
            {
                if(!(ei->pegou))
                {
                    *moedas += 1;
                    ei->pegou = true;
                }
                parede = false;

            }else if(ei->fim)
                {
                    *chegou_no_fim = true;
                    cria_terreno(terreno, obstaculos, xinicial, yinicial);

                }else parede = true;


        }
    }

    if (!hitObstacle)                            //Se n�o estiver sobre algum bloco, n�o pode pular e a gravidade age
    {
        player->position.y += player->speed*delta;
        player->speed += GRAVIDADE * GetFrameTime();
        player->canJump = false;
        *rotation += 10;
    }else                                  //Se estiver sobre algum bloco n�o pode pular e a gravidade n�o age
    {
        player->canJump = true;
        *rotation = 0;
    }





    if(!parede)                            //Se n�o colidir frontalmente segue movendo o player
    {
        player->position.x += PLAYER_HOR_SPD*delta;
    }else                                  //Se colidir frontalmente morre
    {
        player->position = (Vector2){ (float)*xinicial, (float)*yinicial};
        *tentativas += 1;
        *moedas = 0;
        cria_terreno(terreno, obstaculos, xinicial, yinicial);

    }
    if(bateucabeca)
    {
        player->speed = 0.0f;
    }



}

/*Esta fun��o cria um arquivo top5 e coloca valores padr�o. Esta fun��o s� � chamada no caso de quando o arquivo  top5 n�o existe*/
void cria_arq_top5(void)
{
        JOGADOR top5[5] =
    {
        {"Jogador1", 0},
        {"Jogador2", 0},
        {"Jogador3", 0},
        {"Jogador4", 0},
        {"Jogador5", 0}
    };

    FILE *fp;
    fp = fopen("top5.bin", "w+");

    if(fp != NULL)
    {
        for(int i = 0; i < 5; i++)
        {
            if((fwrite(top5 + i, sizeof(JOGADOR), 1, fp)) == 1)
            {
                printf("Impressao correta\n");
            }else printf("Falha na impressao\n");
        }
    }else printf("Falha na criacao do arquivo\n");

    fclose(fp);

}

/*Esta armazena o nome do jogador*/
void pega_nome_jogador(JOGADOR *novo_jogador)
{
    int letterCount = 0;

    Rectangle textBox = {LARGURA_TELA/2.0f - 300, 180, 620, 50 };

    int framesCounter = 0;

    while (!WindowShouldClose() && !IsKeyPressed(KEY_ENTER))
    {
            int key = GetCharPressed();

            while (key > 0)
            {
                if ((key >= 32) && (key <= 125) && (letterCount < MAX_INPUT_CHARS))
                {
                    novo_jogador->nome[letterCount] = (char)key;
                    novo_jogador->nome[letterCount+1] = '\0';
                    letterCount++;
                }

                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                letterCount--;
                if (letterCount < 0) letterCount = 0;
                novo_jogador->nome[letterCount] = '\0';
            }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("CONGRATULATIONS! You are in the top5!", 240, 140, 20, GRAY);

            DrawRectangleRec(textBox, LIGHTGRAY);
            DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);

            DrawText(novo_jogador->nome, (int)textBox.x + 5, (int)textBox.y + 8, 40, MAROON);

            DrawText(TextFormat("INPUT CHARS: %i/%i", letterCount, MAX_INPUT_CHARS), 315, 250, 20, DARKGRAY);

                if (letterCount < MAX_INPUT_CHARS)
                {
                    if (((framesCounter/20)%2) == 0) DrawText("_", (int)textBox.x + 8 + MeasureText(novo_jogador->nome, 40), (int)textBox.y + 12, 40, MAROON);
                }
                DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
                DrawText("Press Enter when finished", 230, 350, 20, GRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
}

/*Inclui um novo jogador no top5 eliminando o �ltimo*/
void altera_top5(JOGADOR top5[5], int pontos, int posicao)
{
    JOGADOR novo_jogador = {};

    pega_nome_jogador(&novo_jogador);
    novo_jogador.pontos = pontos;

    for(int i = 0; i < 5 ; i++)
    {
        if((5 - i - 1) > posicao)
        {
            *(top5 + (5 - i - 1)) = *(top5 + (5 - i - 2)); //Move os jogadores uma posi��o para baixo

        }else if((5 - i - 1) == posicao)
                {
                    *(top5 + (5 - i - 1)) = novo_jogador;
                }
    }

}

/*Verifica se a quantidade de pontos que o jogador atual fez supera algum dos top5*/
void verifica_pontos(int pontos)
{
    JOGADOR top5[5] = {};
    int i;
    FILE *fp;
    bool pontuacao_maior = false;
    bool achou = false;
    while((fp = fopen("top5.bin", "rb+")) == NULL)
    {
        cria_arq_top5();
    }

    if(fp != NULL)
    {
        rewind(fp);
        for(i = 0; i < 5; i++)
        {
            if((fread(top5 + i, sizeof(JOGADOR), 1, fp) == 1))
            {
                printf("Leu corretamente\n");
            }else printf("Falha na Leitura\n");

            if(top5[i].pontos < pontos)
            pontuacao_maior = true;

        }
        rewind(fp);

        if(pontuacao_maior)
        {
            i = 0;
            while(!achou && i < 5)
            {
                if(top5[i].pontos < pontos)
                {
                    altera_top5(top5, pontos, i);
                    achou = true;
                }
                i++;
            }
            fclose(fp);

            fp = fopen("top5.bin", "wb");

            if(fp != NULL)
            {
                for(i = 0; i < 5; i++)
                {
                    if(fwrite(top5 + i, sizeof(JOGADOR), 1, fp) > 0)
                    {
                        printf("Escrita do top5 correta\n");
                    }
                    else printf("Falha na escrita do top5\n");
                }
            }else printf("Falha ao abrir o arquivo para paddar os jogadores\n");

        }
    }
    else printf("Fp esta vazio\n");


    fclose(fp);

}

/*Esta fun��o cria todos os bot�es usados nos menus*/
void cria_botoes(BOTAO botoes[6])
{
    // Bot�o de play
    botoes->textura = LoadTexture("play.png");
    botoes->moldura.x = 0;
    botoes->moldura.y = 0;
    botoes->moldura.width = (float)botoes->textura.width;
    botoes->moldura.height = (float)botoes->textura.height;
    botoes->bordas.x = (float)LARGURA_TELA/2.0f - (float)botoes->textura.width/2.00;
    botoes->bordas.y =  ALTURA_TELA/(64.00/22.00);
    botoes->bordas.width = (float)botoes->textura.width;
    botoes->bordas.height = (float)botoes->textura.height/(3.00/2.00);

    //Bot�o de leaderboard
    (botoes + 1)->textura = LoadTexture("top5.png");
    (botoes + 1)->moldura.x = 0;
    (botoes + 1)->moldura.y = 0;
    (botoes + 1)->moldura.width = (float)(botoes + 1)->textura.width;
    (botoes + 1)->moldura.height = (float)(botoes + 1)->textura.height;
    (botoes + 1)->bordas.x = (float)LARGURA_TELA/2.0f - (float)(botoes + 1)->textura.width/2.0f;
    (botoes + 1)->bordas.y =  ALTURA_TELA/(81.00/45.00);
    (botoes + 1)->bordas.width = (float)(botoes + 1)->textura.width;
    (botoes + 1)->bordas.height = (float)(botoes + 1)->textura.height/(3.00/2.00);

    //Bot�o de sair
    (botoes + 2)->textura = LoadTexture("quit.png");
    (botoes + 2)->moldura.x = 0;
    (botoes + 2)->moldura.y = 0;
    (botoes + 2)->moldura.width = (float)(botoes + 2)->textura.width;
    (botoes + 2)->moldura.height = (float)(botoes + 2)->textura.height;
    (botoes + 2)->bordas.x = (float)LARGURA_TELA/2.0f - (float)(botoes + 2)->textura.width/2.0f;
    (botoes + 2)->bordas.y =  ALTURA_TELA/(70.00/55.00);
    (botoes + 2)->bordas.width = (float)(botoes + 2)->textura.width;
    (botoes + 2)->bordas.height = (float)(botoes + 2)->textura.height/(3.00/2.00);

    //Bot�o fase 1
    (botoes + 3)->textura = LoadTexture("fase1.png");
    (botoes + 3)->moldura.x = 0;
    (botoes + 3)->moldura.y = 0;
    (botoes + 3)->moldura.width = (float)(botoes + 3)->textura.width;
    (botoes + 3)->moldura.height = (float)(botoes + 3)->textura.height;
    (botoes + 3)->bordas.x = (float)LARGURA_TELA/4.0f - (float)(botoes + 3)->textura.width/2.0f;
    (botoes + 3)->bordas.y =  ALTURA_TELA/2;
    (botoes + 3)->bordas.width = (float)(botoes + 3)->textura.width;
    (botoes + 3)->bordas.height = (float)(botoes + 3)->textura.height/(3.00/2.00);

    //Bot�o fase 2
    (botoes + 4)->textura = LoadTexture("fase2.png");
    (botoes + 4)->moldura.x = 0;
    (botoes + 4)->moldura.y = 0;
    (botoes + 4)->moldura.width = (float)(botoes + 4)->textura.width;
    (botoes + 4)->moldura.height = (float)(botoes + 4)->textura.height;
    (botoes + 4)->bordas.x = (float)LARGURA_TELA/2.0f - (float)(botoes + 4)->textura.width/2.0f;
    (botoes + 4)->bordas.y =  ALTURA_TELA/2;
    (botoes + 4)->bordas.width = (float)(botoes + 4)->textura.width;
    (botoes + 4)->bordas.height = (float)(botoes + 4)->textura.height/(3.00/2.00);

    //Bot�o fase 3
    (botoes + 5)->textura = LoadTexture("fase3.png");
    (botoes + 5)->moldura.x = 0;
    (botoes + 5)->moldura.y = 0;
    (botoes + 5)->moldura.width = (float)(botoes + 5)->textura.width;
    (botoes + 5)->moldura.height = (float)(botoes + 5)->textura.height;
    (botoes + 5)->bordas.x = (float)LARGURA_TELA/(4.0f / 3.0f) - (float)(botoes + 5)->textura.width/2.0f;
    (botoes + 5)->bordas.y =  ALTURA_TELA/2;
    (botoes + 5)->bordas.width = (float)(botoes + 5)->textura.width;
    (botoes + 5)->bordas.height = (float)(botoes + 5)->textura.height/(3.00/2.00);

    //Bot�o voltar
    (botoes + 6)->textura = LoadTexture("back.png");
    (botoes + 6)->moldura.x = 0;
    (botoes + 6)->moldura.y = 0;
    (botoes + 6)->moldura.width = (float)(botoes + 6)->textura.width;
    (botoes + 6)->moldura.height = (float)(botoes + 6)->textura.height;
    (botoes + 6)->bordas.x = (float)LARGURA_TELA/(10.0f / 9.0f) - (float)(botoes + 6)->textura.width/2.0f;
    (botoes + 6)->bordas.y =  ALTURA_TELA/(10.0f/8.0f);
    (botoes + 6)->bordas.width = (float)(botoes + 6)->textura.width;
    (botoes + 6)->bordas.height = (float)(botoes + 6)->textura.height/(3.00/2.00);
}

/*Le o arquivo top5 e copia para um vetor, seo arquivo n�o existir chama a fun��o que cria, esta fun��o tamb�m desenha o top5 na tela*/
void le_top5(BOTAO *botao)
{
    FILE *fp;
    JOGADOR top5[5] = {};
    char pontos[16] = {};
    int i;
    while((fp = fopen("top5.bin", "rb")) == NULL) //Enquanto o arquivo top5 n�o existir cria ele
    {
        cria_arq_top5();
    }

    if(fp != NULL)
    {
        rewind(fp);
        printf("Leitura--------------\n");
        for(int i = 0; i < 5; i++)
        {
            if((fread(&top5[i], sizeof(JOGADOR), 1, fp) == 1)) //Copia para o vetor
            {
                printf("Leu corretamente\n");
            }else printf("Falha na Leitura\n");

        }
    }

    Vector2 mousePoint = { 0.0f, 0.0f };
    while(!WindowShouldClose() && !botao->acao)
    {
        mousePoint = GetMousePosition();

                    if (CheckCollisionPointRec(mousePoint, botao->bordas)) //Checa se o bot�o de voltar
                        {
                            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                            {
                                botao->estado = 2;
                            }
                            else
                            {
                                botao->estado = 1;
                            }

                            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                            {
                                botao->acao = true;

                            }
                        }
                        else
                            {
                                botao->estado = 0;
                            }


    BeginDrawing();

    ClearBackground(YELLOW);

    DrawText(top5[0].nome, LARGURA_TELA/5.0f, (ALTURA_TELA/8.0f)*(1), 20, BLACK); //Desenha os nomes na tela
    for(i = 0; i < 5; i++)
    {
        DrawText( (top5 + i)->nome, LARGURA_TELA/5.0f, (ALTURA_TELA/8.0f)*(i + 1), 20, BLACK); //Desenha os nomes na tela
        sprintf(pontos, "%d", top5[i].pontos);  //Converte os pontos de inteiros para string
        DrawText(pontos, LARGURA_TELA/2.0f, ALTURA_TELA/8.0f*(i + 1), 20, BLACK);         //Desenha os pontos na tela
    }
    DrawTextureRec(botao->textura, botao->moldura, (Vector2){ botao->bordas.x, botao->bordas.y }, WHITE); //Desenha bot�o de voltar

    EndDrawing();

    botao->moldura.y = ((float)botao->textura.height/9) + (10 * ((float)botao->estado)); //Anima��o de movimento do bot�o
    }
    fclose(fp);
}

/*Fun��o de camera*/
void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, int width, int height)
{
    static Vector2 bbox = { 0.2f, 0.5f };

    Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2){ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height }, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2){ (1 + bbox.x)*0.5f*width, (1 + bbox.y)*0.5f*height }, *camera);
    camera->offset = (Vector2){ (1 - bbox.x)*0.5f * width, (1 - bbox.y)*0.5f*height };

    if (player->position.x < bboxWorldMin.x) camera->target.x = player->position.x;
    if (player->position.y < bboxWorldMin.y) camera->target.y = player->position.y;
    if (player->position.x > bboxWorldMax.x - 1000) camera->target.x = bboxWorldMin.x + (player->position.x - bboxWorldMax.x + 500);
    if (player->position.y > bboxWorldMax.y) camera->target.y = bboxWorldMin.y + (player->position.y - bboxWorldMax.y);
}
/*---------------------------------------Defini��o de fun��es------------------------------------------*/


int main(void)
{

    const int largura_tela = LARGURA_TELA;
    const int altura_tela = ALTURA_TELA;

    /*------------Declara��o e inicializa��o de vari�veis-------------*/
    int tentativas = 1;
    char tentativas_char[10] = {};


    Rectangle moeda;
    moeda.x = 0;
    moeda.y = 0;
    moeda.width = B;
    moeda.height = H;

    int moedas = 0;
    char moedas_char[3] = {};

    bool chegou_no_fim = false;
    bool sair = false;

    char fase1[10] = {"fase1.txt"};
    char fase2[10] = {"fase2.txt"};
    char fase3[10] = {"fase3.txt"};

    InitWindow(largura_tela, altura_tela, "raylib [core] example - 2d camera");

    Texture2D coin_text;
    coin_text = LoadTexture("coin.png");

    Texture2D fundo;
    fundo = LoadTexture("fundo.png");
    Rectangle fundo_moldura = {0, 0, LARGURA_TELA, ALTURA_TELA};

    Texture2D boneco;
    boneco = LoadTexture("boneco.png");

    Texture2D espinho;
    espinho = LoadTexture("espinho.png");

    Texture2D wall;
    wall = LoadTexture("fundo6.png");

    Rectangle fundo_wall = {0, 0, LARGURA_TELA, ALTURA_TELA};


    int xinicial = 0;
    int yinicial = 0;

    Player player = { 0 };
    player.speed = 0;
    player.canJump = true;
    float rotation;


    EnvItem obstaculos[MAXALT * MAXCOMP] = {};
    char terreno[MAXALT][MAXCOMP] = {};

    int pontos;


    int obstaculosLength = sizeof(obstaculos)/sizeof(obstaculos[0]);

    Camera2D camera = { 0 };
    camera.target = player.position;
    camera.offset = (Vector2){ largura_tela/2.0f, altura_tela/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 0.8f;


    BOTAO botoes[7];
    cria_botoes(botoes);

    Vector2 mousePoint = { 0.0f, 0.0f };
/*------------Declara��o e inicializa��o de vari�veis-------------*/


    SetTargetFPS(60);

/*--------------------------------LOOP PRINCIPAL---------------------------*/
    while (!WindowShouldClose() && !sair)
    {
        mousePoint = GetMousePosition();
        botoes[0].estado = false;


        if(!botoes[0].acao) //Se o bot�o de play n�o estiver ativado desenha o menu inicial
        {
            for(int i = 0; i < 3; i++)
            {
                if (CheckCollisionPointRec(mousePoint, botoes[i].bordas)) //Chega colis�o com os botoes do menu inicial
                {
                    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                    {
                        botoes[i].estado = 2;
                    }
                    else
                    {
                        botoes[i].estado = 1;
                    }

                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                    {
                        botoes[i].acao = true;

                    }
                }
                else botoes[i].estado = 0;
            }

        }else {                                    //Se o bot�o de play estiver ativado desenha do menu de fases
                        for(int i = 3; i <= 6; i++)
                        {
                            if (CheckCollisionPointRec(mousePoint, botoes[i].bordas)) //Chega colis�o com os bot�es de fase
                            {
                                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                                {
                                    botoes[i].estado = 2;
                                }
                                else
                                {
                                    botoes[i].estado = 1;
                                }

                                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                                {
                                    botoes[i].acao = true;
                                }
                            }
                            else botoes[i].estado = 0;
                        }
                }

        if(botoes[1].acao) //Se o bot�o leaderbord for pressionado chama a fun��o que l� e imprime o leader board
                {
                    le_top5(&botoes[6]);
                    botoes[1].acao = false;
                }



        if (botoes[3].acao || botoes[4].acao || botoes[5].acao) // Se bot�o fase1 fase2 ou fase3 for pressionado, come�a o jogo
        {
            if(botoes[3].acao)  //Se fase 1 tiver sido pressionado carrega fase1
            carrega_terreno(terreno, fase1);

            if(botoes[4].acao)  //Se fase 2 tiver sido pressionado carrega fase2
            carrega_terreno(terreno, fase2);

            if(botoes[5].acao)  //Se fase 3 tiver sido pressionado carrega fase3
            carrega_terreno(terreno, fase3);

            botoes[0].acao = false;

            for(int i = 3; i <= 5; i++)
            {
                botoes[i].acao = false;
            }

            cria_terreno(terreno, obstaculos, &xinicial, &yinicial);
            player.position = (Vector2){ (float)xinicial, (float)yinicial};
            tentativas = 1;
            chegou_no_fim = false;

            //Loop do jogo-------------------------------------------------------------
            while(!IsKeyPressed(KEY_ESCAPE) && !WindowShouldClose() && !chegou_no_fim)
            {
                pontos = 1001;

                float deltaTime = GetFrameTime();

                UpdatePlayer(&player, obstaculos, obstaculosLength, deltaTime, &tentativas, &chegou_no_fim, &moedas, terreno, &rotation, &xinicial, &yinicial);

                pontos -= tentativas;
                pontos += moedas;

                camera.zoom += ((float)GetMouseWheelMove()*0.05f);

                if (camera.zoom > 2.0f)
                {
                    camera.zoom = 0.8f;
                }
                else if (camera.zoom < 0.25f)
                    {
                         camera.zoom = 0.25f;
                    }

                if (IsKeyPressed(KEY_R))
                {
                    camera.zoom = 1.0f;
                    player.position = (Vector2){(float)xinicial, (float)yinicial};
                }

                UpdateCameraPlayerBoundsPush(&camera, &player, LARGURA_TELA, ALTURA_TELA);

                //Desenha o jogo
                BeginDrawing();

                    ClearBackground(LIGHTGRAY);

                    DrawTextureRec(wall, fundo_wall, (Vector2){0 , 0}, WHITE);
                    BeginMode2D(camera);


                        for (int i = 0; i < obstaculosLength; i++)
                        {
                            if(obstaculos[i].moeda && !obstaculos[i].pegou) //Se for uma moeda e ela n�o tiver sido pega desenha
                            {
                                DrawTextureRec(coin_text, moeda, (Vector2){obstaculos[i].rect.x, obstaculos[i].rect.y}, WHITE);

                            }else if(obstaculos[i].espinho) //Se for espinho deseneha
                                    {
                                        DrawTextureRec(espinho, moeda, (Vector2){obstaculos[i].rect.x, obstaculos[i].rect.y}, WHITE);
                                    }else if(!obstaculos[i].moeda) //Se n�o desenha blocos normal
                                    DrawRectangleRec(obstaculos[i].rect, obstaculos[i].color);
                        }


                        Rectangle playerRect = { 0, 0, 40.0f, 40.0f };
                        Rectangle playersource = {player.position.x + 20, player.position.y - 20, 40.00, 40.00};
                        DrawTexturePro(boneco, playerRect, playersource, (Vector2){20, 20}, rotation, WHITE); //Desenha o player


                    EndMode2D();

                    DrawText("Tentativas: ", 100, 100, 20, BLACK);
                    sprintf(tentativas_char, "%d", tentativas);
                    DrawText(tentativas_char, 230, 100, 20, BLACK);
                    DrawText("Moedas: ", 300, 100, 20, BLACK);
                    sprintf(moedas_char, "%d", moedas);
                    DrawText(moedas_char, 400, 100, 20, BLACK);

                EndDrawing();
            }
            //Loop do jogo-------------------------------------------------------------

            if(chegou_no_fim)     //Se chegar no fim do mapa chama a fun��o verfica pontos
            {
                verifica_pontos(pontos);
            }


        }

        if(botoes[6].acao) //Se o bot�o de voltar for presionado volta pro menu inicial
        {
            for(int i = 0; i <= 6; i++)
            {
                botoes[i].acao = false;
            }
        }

        for(int i = 0; i <= 6; i++) //Anima��o de movimento dos bot�es
        {
            botoes[i].moldura.y = ((float)botoes[i].textura.height/9) + (10 * ((float)botoes[i].estado));
        }

        if(botoes[2].acao)//Se bot�o de sair for pressionado fecha tudo
        {
            sair = true;
        }



            BeginDrawing();  //Desenha os bot�es

                ClearBackground(RAYWHITE);

                DrawTextureRec(fundo, fundo_moldura, (Vector2){0, 0}, WHITE);

                if(!(botoes[0].acao)) //Se o play n�o estiver ativo desenha o menu inicial
                {
                    for(int i = 0; i < 3; i++)
                    {
                        DrawTextureRec(botoes[i].textura, botoes[i].moldura, (Vector2){ botoes[i].bordas.x, botoes[i].bordas.y }, WHITE);
                    }

                }else {   //Se o play estiver ativo desenha o menu de fases
                                for(int i = 3; i <= 6; i++)
                                {
                                    DrawTextureRec(botoes[i].textura, botoes[i].moldura, (Vector2){ botoes[i].bordas.x, botoes[i].bordas.y }, WHITE);
                                }
                }



            EndDrawing();

    }
/*--------------------------------LOOP PRINCIPAL---------------------------*/

    sair = false;


    CloseWindow();

    return 0;
}


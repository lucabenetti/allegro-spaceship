#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#define TELA_LARGURA 900
#define TELA_ALTURA 480
#define NOME_GAME "Yellow Ship"
#define FPS 60
#define MAX_METEOROS 20
#define MAX_TIROS 20
#define TAM_PIXEL 6
#define MAX_SALVOS 4
#define SOM 1

typedef struct{
    int r, g, b, a;
} cor;

typedef struct{
    int x, y;
} posicao;

typedef struct{
    int x, y;
    int largura, altura;
    char *texto;
    cor cor_fundo, cor_texto;
} botao;

typedef struct {
    posicao pos;
    int largura, altura;
    ALLEGRO_BITMAP *img;
    int flag;
    posicao sprite;
} elemento;

typedef struct{
    elemento el;
    int balas;
} naveEspacial;

//Variaveis globais e unicas
ALLEGRO_DISPLAY *janela = NULL;
ALLEGRO_EVENT_QUEUE *filaEventosPrincipal = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_FONT *fonte = NULL;

cor cor_botao, cor_texto, cor_mouse, cor_click;

void iniciaAllegro(void); //Inicia todas as libs necessarias do allegro
void encerraAllegro(void); //Encerra e destroi todas as variaveis allegro

int telaMenu(char *nick);
int telaJogo(char *nick, int *pontuacao);
int telaFinal(char *nick, int *pontuacao);
int telaRank(void);

void iniciaAleatorio(void); // inicia SRand
int aleatorio(int a, int b); // Retorna Número Aleatorio de 'a' ate 'b'
void confereNULL(void **ptr);

int tamanhoStr(char *str); // Strlen
void copiaStr(char *str1, char *str2, int len); // Strcpy
cor criaCor(int r, int g, int b, int a); // Cria estrutura de cor
void setCor(botao *ptr, cor cor_fundo); // Coloca cor no fundo do botao

void iniciaBotao(botao *ptr, char *texto, cor cor_texto, int x, int y, cor cor_fundo, int largura, int altura);
void destroiBotao(botao *ptr); // free(Botao)
void renderizaBotao(botao *ptr, ALLEGRO_DISPLAY *janela, ALLEGRO_FONT *fonte);
void encerraBotoes(botao *ptr, int len);
int mouseNoBotao(botao *ptr, int mouse_x, int mouse_y); // Confere se o Mouse esta em cima do Botão

void defPosicao(posicao *ptr, int x, int y); // Define a posicao do elemento
void iniciaElemento(elemento *ptr, char *img, int x, int y, int largura, int altura);
void destroiElemento(ALLEGRO_BITMAP *ptr); // free(Elemento)
void renderizaElemento(elemento *ptr);
void renderizaElementoRotacao(elemento *ptr, int fator);
void renderizaElementoSprite(elemento *ptr, int x, int y);

void geraMeteoro(elemento **meteoros);
void geraTiro(elemento **tiros, int x, int y);
int colisao(elemento *a, elemento *b); // Confere se elemento a colidiu com elemento b
void addBalas(naveEspacial *ptr);
void dificultaJogo(int pontuacao, int *velocidadeMeteoro, int *intervaloTempo);

int main(){
    cor_botao = criaCor(40, 20, 200, 0);
    cor_texto = criaCor(0, 0, 0, 255);
    cor_mouse = criaCor(159, 159, 159, 0);
    cor_click = criaCor(236, 12, 45, 255);
    iniciaAleatorio();
    iniciaAllegro();

    int escolha, pontuacao;
    char nick[4] = {'A', 'A', 'A', '\0'};
    while(escolha != -1){
        escolha = telaMenu(nick);
        if (escolha == 1){
            printf("JOGAR\n");
            pontuacao = 0;
            escolha = telaJogo(nick, &pontuacao);
            if (escolha == 0){
                escolha = telaFinal(nick, &pontuacao);
            }
        }
        else if (escolha == 2){
            printf("RANK\n");
            escolha = telaRank();
        }
    }
    encerraAllegro();
    return 0;
}

void iniciaAllegro(void){
    al_init();
    al_init_image_addon();
    al_init_primitives_addon();
    timer = al_create_timer(1.0 / FPS);
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();
    al_install_audio();
    al_init_acodec_addon();
    al_reserve_samples(10);

    fonte = al_load_font("_fontes/8BIT.ttf", 30, 0);

    janela = al_create_display(TELA_LARGURA, TELA_ALTURA);
    al_set_window_title(janela, NOME_GAME);
    filaEventosPrincipal = al_create_event_queue();
    al_register_event_source(filaEventosPrincipal, al_get_display_event_source(janela));
    al_register_event_source(filaEventosPrincipal, al_get_timer_event_source(timer));
    al_set_system_mouse_cursor(janela, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
    al_start_timer(timer);
}

void encerraAllegro(void){
    al_destroy_display(janela);
    al_destroy_event_queue(filaEventosPrincipal);
    al_destroy_timer(timer);
    al_destroy_font(fonte);
}

int telaMenu(char *nick){
    ALLEGRO_BITMAP *planoFundo;
    ALLEGRO_EVENT_QUEUE *filaEventos;
    filaEventos = al_create_event_queue();
    al_register_event_source(filaEventos, al_get_mouse_event_source());
    planoFundo = al_load_bitmap("_imagens/planoFundo.png");
    ALLEGRO_EVENT eventoPrincipal, evento;
    int mouse_x = 0, mouse_y = 0;
    int i;
    botao botoes[5];
    iniciaBotao(&botoes[0], "JOGAR!\0", cor_texto, TELA_LARGURA / 2, 270, cor_botao, 220, 60);
    iniciaBotao(&botoes[1], "RANK\0", cor_texto, TELA_LARGURA / 2, 340, cor_botao, 180, 60);
    for (i = 0; i < 3; i++){
        iniciaBotao(&botoes[i+2], "\0", cor_texto, TELA_LARGURA / 2 - 75 + (80 * i), 170, cor_botao, 60, 60);
    }

    while(1){

        if (!al_is_event_queue_empty(filaEventosPrincipal)){
            al_wait_for_event(filaEventosPrincipal, &eventoPrincipal);
            if (eventoPrincipal.type == ALLEGRO_EVENT_TIMER){
                al_clear_to_color(al_map_rgb(255, 255, 255));
                al_draw_bitmap(planoFundo, 0, 0, 0);
                for (i = 0; i < 5; i++){
                    renderizaBotao(&botoes[i], janela, fonte);
                }

                for (i = 0; i < 3; i++){
                    al_draw_textf(fonte, al_map_rgb(0, 0, 0), TELA_LARGURA / 2 - 75 + (80 * i) , 170, ALLEGRO_ALIGN_CENTRE, "%c", nick[i]);
                }

                al_flip_display();
            }
            else if (eventoPrincipal.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                encerraBotoes(botoes, 5);
                return -1;
            }
        }

        if (!al_is_event_queue_empty(filaEventos)){

            al_wait_for_event(filaEventos, &evento);

            if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){
                mouse_x = evento.mouse.x;
                mouse_y = evento.mouse.y;
                for (i = 0; i < 5; i++){
                    if (mouseNoBotao(&botoes[i], mouse_x, mouse_y)){
                        botoes[i].cor_fundo = cor_mouse;
                    }
                    else{
                        botoes[i].cor_fundo = cor_botao;
                    }
                }
            }
            else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                for (i = 0; i < 5; i++){
                    if (mouseNoBotao(&botoes[i], mouse_x, mouse_y)){
                        if (i == 0){
                            encerraBotoes(botoes, 5);
                            return 1;
                        }
                        else if (i == 1){
                            encerraBotoes(botoes, 5);
                            return 2;
                        }
                        else{
                            nick[i-2] += 1;
                            if (nick[i-2] == 91)
                                nick[i-2] = 65;
                            botoes[i].cor_fundo = cor_mouse;
                        }
                    }
                }
            }
            else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                for (i = 0; i < 5; i++)
                    if (mouseNoBotao(&botoes[i], mouse_x, mouse_y))
                        botoes[i].cor_fundo = cor_click;
            }
        }
    }
    al_destroy_event_queue(filaEventos);
}

int telaJogo(char *nick, int *pontuacao){
    ALLEGRO_EVENT evento;
    ALLEGRO_BITMAP *planoFundo2 = NULL, *planoFundo1 = NULL;
    ALLEGRO_EVENT_QUEUE *filaEventos = NULL;
    ALLEGRO_SAMPLE *somTiro = NULL, *somExplosaoNave = NULL, *somExplosaoMeteoro = NULL, *somMunicaoAdd = NULL;
    filaEventos = al_create_event_queue();
    al_register_event_source(filaEventos, al_get_keyboard_event_source());

    somTiro = al_load_sample("_sons/tiro1.wav");
    somExplosaoNave = al_load_sample("_sons/explosao1.wav");
    somExplosaoMeteoro = al_load_sample("_sons/explosao2.wav");
    somMunicaoAdd = al_load_sample("_sons/municaoAdd.wav");
    planoFundo1 = al_load_bitmap("_imagens/fundoJogo.png");
    planoFundo2 = al_load_bitmap("_imagens/fundoJogo.png");
    naveEspacial nave;
    iniciaElemento(&nave.el, "_imagens/nave/nave1.png", 25, 280, 83, 40);
    nave.balas=10;
    int direcao = 0;

    elemento **meteoros;
    meteoros = (elemento **) calloc(MAX_METEOROS, sizeof(elemento *));
    confereNULL(&meteoros);

    elemento **tiros;
    tiros = (elemento **) calloc(MAX_TIROS, sizeof(elemento *));
    confereNULL(&meteoros);
    //inicia tudo com NULL
    ALLEGRO_FONT *fonteTexto = NULL;
    ALLEGRO_TIMER *timerPontos = NULL;
    timerPontos = al_create_timer(1.0 / 6);
    al_register_event_source(filaEventos, al_get_timer_event_source(timerPontos));
    fonteTexto = al_load_font("_fontes/8BIT.ttf", 12, 0);
    int velocidadePlanoFundo = 2;
    posicao posPlanoFundo1, posPlanoFundo2;
    defPosicao(&posPlanoFundo1, 0, 0);
    defPosicao(&posPlanoFundo2, TELA_LARGURA, 0);
    int i, j, sair = 42;
    int clock = 30;
    int intervalo = 50;
    int velocidadeMeteoro = 6;
    int velocidadeTiro = 8;
    al_start_timer(timerPontos);

    while(sair == 42){
        while(sair == 42 && !al_is_event_queue_empty(filaEventosPrincipal)){
            al_wait_for_event(filaEventosPrincipal, &evento);
            if(evento.type == ALLEGRO_EVENT_TIMER){
                al_clear_to_color(al_map_rgb(255, 255, 255));
                al_draw_bitmap(planoFundo1, posPlanoFundo1.x, posPlanoFundo1.y, 0);
                al_draw_bitmap(planoFundo2, posPlanoFundo2.x, posPlanoFundo2.y, 0);
                defPosicao(&posPlanoFundo1, posPlanoFundo1.x - velocidadePlanoFundo, posPlanoFundo1.y);
                if (posPlanoFundo1.x <= -TELA_LARGURA)
                    posPlanoFundo1.x = TELA_LARGURA;
                defPosicao(&posPlanoFundo2, posPlanoFundo2.x - velocidadePlanoFundo, posPlanoFundo2.y);
                if (posPlanoFundo2.x <= -TELA_LARGURA)
                    posPlanoFundo2.x = TELA_LARGURA;
                //Printa os meteoros
                for (i = 0; i < MAX_METEOROS; i++){
                    if(meteoros[i] != NULL){
                        if(meteoros[i]->flag >= 10){
                            renderizaElementoSprite(meteoros[i], meteoros[i]->sprite.x, meteoros[i]->sprite.y);
                            meteoros[i]->flag++;
                            if ((meteoros[i]->flag - 10) % 5 == 0){
                                meteoros[i]->sprite.x += 256;
                                if (meteoros[i]->sprite.x >= 1792){
                                    meteoros[i]->flag = -1;
                                    destroiElemento(meteoros[i]->img);
                                    free(meteoros[i]);
                                    meteoros[i] = NULL;
                                    continue;
                                }
                            }
                        }
                        else
                            renderizaElementoRotacao(meteoros[i], clock);
                        //ALTERACAO DA POSICAO DO PROXIMO FRAME

                        if ((meteoros[i]->flag > -1 && meteoros[i]->flag < 10) && colisao(&nave.el, meteoros[i]) && (nave.el.flag > -1 && nave.el.flag < 10)){
                            printf("Colidiu Nave\n");
                            if(meteoros[i]->flag == 3){
                                addBalas(&nave);
                                if (SOM)
                                    al_play_sample(somMunicaoAdd, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                            else{
                                nave.el.flag = 10;
                                nave.el.img = al_load_bitmap("_imagens/explosao/explosao.png");
                                if (SOM)
                                    al_play_sample(somExplosaoNave, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                            }
                            destroiElemento(meteoros[i]->img);
                            free(meteoros[i]);
                            meteoros[i] = NULL;
                            break;
                        }

                        defPosicao(&meteoros[i]->pos, meteoros[i]->pos.x - velocidadeMeteoro, meteoros[i]->pos.y);
                        if(meteoros[i]->pos.x < 0 - meteoros[i]->largura){
                            destroiElemento(meteoros[i]->img);
                            free(meteoros[i]);
                            meteoros[i] = NULL;
                            printf("Apagado Meteoro\n");
                        }
                    }
                }
                //Printa os tiros
                for (i = 0; i < MAX_TIROS; i++){
                    if (tiros[i] != NULL){
                        renderizaElemento(tiros[i]);
                        //ALTERACAO DA POSICAO DO PROXIMO FRAME
                        defPosicao(&tiros[i]->pos, tiros[i]->pos.x + velocidadeTiro, tiros[i]->pos.y);

                        if (tiros[i]->pos.x > TELA_LARGURA){
                            destroiElemento(tiros[i]->img);
                            free(tiros[i]);
                            tiros[i] = NULL;
                            printf("TIRO APAGADO\n");
                        }
                        else{
                            for (j = 0; j < MAX_METEOROS; j++){
                                if(meteoros[j] != NULL)
                                    if (colisao(tiros[i], meteoros[j]) && meteoros[j]->flag > -1 && meteoros[j]->flag < 10){
                                        if(meteoros[j]->flag == 3){
                                            addBalas(&nave);
                                            if (SOM)
                                                al_play_sample(somMunicaoAdd, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                                        }
                                        meteoros[j]->flag = 10;
                                        meteoros[j]->img = al_load_bitmap("_imagens/explosao/explosao.png");
                                        destroiElemento(tiros[i]->img);
                                        free(tiros[i]);
                                        tiros[i] = NULL;
                                        if (SOM)
                                            al_play_sample(somExplosaoMeteoro, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                                        break;

                                    }
                            }
                        }
                    }
                }
                if (nave.el.flag > -1 && nave.el.flag < 10)
                    renderizaElemento(&nave.el);
                else{
                    nave.el.flag++;
                    if ((nave.el.flag - 10) % 10 == 0){
                        nave.el.sprite.x += 256;
                        if (nave.el.sprite.x >= 1792){
                            nave.el.flag = -1;
                            sair = 0;
                            break;
                        }
                    }
                    renderizaElementoSprite(&nave.el, nave.el.sprite.x, nave.el.sprite.y);
                }

                al_draw_textf(fonteTexto, al_map_rgba(255, 255, 255, 1), TELA_LARGURA - 220, 25, ALLEGRO_ALIGN_LEFT, "PONTOS: %.8d", (*pontuacao));

                al_draw_textf(fonteTexto, al_map_rgba(255, 255, 255, 1), TELA_LARGURA - 350, 25, ALLEGRO_ALIGN_LEFT, "BALAS: %.2d", (nave.balas));

                clock++;
                al_flip_display();
            }
            else if(evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
                sair = -1; //fechou a janela
                break;
            }
        }
        while(sair == 42 && !al_is_event_queue_empty(filaEventos)){
            al_wait_for_event(filaEventos, &evento);
            if (evento.type == ALLEGRO_EVENT_KEY_DOWN){
                if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN)
                    direcao = 1;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP)
                    direcao = -1;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_SPACE){
                        if (nave.balas >= 1){
                            geraTiro(tiros, nave.el.pos.x + 5, nave.el.pos.y + (nave.el.altura / 2 - 5));
                            nave.balas--;
                            printf("TIRO GERADO\n");
                            if (SOM)
                                al_play_sample(somTiro, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
                        }
                }
            }
            else if (evento.type == ALLEGRO_EVENT_KEY_UP){
                if (evento.keyboard.keycode == ALLEGRO_KEY_DOWN)
                    direcao = 0;
                else if (evento.keyboard.keycode == ALLEGRO_KEY_UP)
                    direcao = 0;
            }
            else if (evento.type == ALLEGRO_EVENT_TIMER){
                (*pontuacao)++;
                if (*pontuacao > 99999999)
                    (*pontuacao) = 0;
                dificultaJogo(*pontuacao, &velocidadeMeteoro, &intervalo);
            }

            if (direcao != 0 && nave.el.sprite.x > -1 && nave.el.sprite.x < 10)
                if((direcao == -1 && nave.el.pos.y > 5) || (direcao == 1 && nave.el.pos.y < 425))
                    defPosicao(&nave.el.pos, nave.el.pos.x, nave.el.pos.y + direcao * 10);
        }

        if (clock == 366)
        clock = 0;
        if (clock % intervalo == 0){
            geraMeteoro(meteoros);
            printf("Gerou Meteoro\n");
            clock++;
        }

    }
    for (i = 0; i < MAX_METEOROS; i++)
        if (meteoros[i] != NULL){
            destroiElemento(meteoros[i]->img);
            free(meteoros[i]);
        }
    free(meteoros);
    for (i = 0; i < MAX_TIROS; i++)
        if (tiros[i] != NULL){
            destroiElemento(tiros[i]->img);
            free(tiros[i]);
        }
    free(tiros);
    al_destroy_event_queue(filaEventos);
    al_destroy_font(fonteTexto);
    al_destroy_timer(timerPontos);
    destroiElemento(nave.el.img);
    al_destroy_sample(somTiro);
    al_destroy_sample(somExplosaoMeteoro);
    al_destroy_sample(somExplosaoNave);
    al_destroy_sample(somMunicaoAdd);
    return sair;
}

int telaFinal(char *nick, int *pontuacao){
    char **jogadores = NULL, nickMaxPontos[4];
    int *placar = NULL, i, maxPontos;

    FILE *arq;
    arq = fopen("rankYellowShip.txt", "r");
    jogadores = (char **) malloc(MAX_SALVOS * sizeof(char *));
    confereNULL(&jogadores);
    placar = (int *) malloc(MAX_SALVOS * sizeof(int));
    confereNULL(&placar);
    for (i = 0; i < MAX_SALVOS; i++){
        jogadores[i] = (char *) malloc(4 * sizeof(char));
        confereNULL(&jogadores[i]);
    }
    fscanf(arq, "%s %d%*c", nickMaxPontos, &maxPontos);
    copiaStr(jogadores[0], nick, tamanhoStr(nick));
    placar[0] = (*pontuacao);
    for (i = 1; i < MAX_SALVOS; i++){
        fscanf(arq, "%s %d%*c", jogadores[i], &placar[i]);
    }
    if ((*pontuacao) > maxPontos){
        maxPontos = (*pontuacao);
        copiaStr(nickMaxPontos, nick, tamanhoStr(nick));
    }
    fclose(arq);

    arq = fopen("rankYellowShip.txt","w");
    fprintf(arq, "%s %d\n", nickMaxPontos, maxPontos);
    for (i = 0; i < MAX_SALVOS; i++){
        fprintf(arq, "%s %d\n", jogadores[i], placar[i]);
    }
    fclose(arq);

    free(placar);
    for (i = 0; i < MAX_SALVOS; i++)
        free(jogadores[i]);
    free(jogadores);

    ALLEGRO_BITMAP *planoFundo;
    ALLEGRO_EVENT_QUEUE *filaEventos;
    filaEventos = al_create_event_queue();
    al_register_event_source(filaEventos, al_get_mouse_event_source());
    planoFundo = al_load_bitmap("_imagens/telaFinal.png");
    ALLEGRO_EVENT evento;
    int mouse_x = 0, mouse_y = 0;
    int sair = 42;
    botao botao_retornar;
    iniciaBotao(&botao_retornar, "RETORNAR AO MENU!\0", cor_texto, TELA_LARGURA / 2, 380, cor_botao, 600, 60);

    while(sair == 42){
        while (!al_is_event_queue_empty(filaEventosPrincipal)){
            al_wait_for_event(filaEventosPrincipal, &evento);
            if (evento.type == ALLEGRO_EVENT_TIMER){
                al_clear_to_color(al_map_rgb(255, 255, 255));
                al_draw_bitmap(planoFundo, 0, 0, 0);

                renderizaBotao(&botao_retornar, janela, fonte);
                for (i = 0; i < 3; i++){
                    al_draw_textf(fonte, al_map_rgb(0, 0, 0), TELA_LARGURA / 2 - 35 + (40 * i) , 200, ALLEGRO_ALIGN_CENTRE, "%c", nick[i]);
                }
                al_draw_textf(fonte, al_map_rgb(0, 0, 0), TELA_LARGURA / 2, 270, ALLEGRO_ALIGN_CENTRE, "PONTOS: %d", (*pontuacao));

                al_flip_display();
            }
            else if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                sair = -1;
                break;
            }
        }

        while (!al_is_event_queue_empty(filaEventos)){
            al_wait_for_event(filaEventos, &evento);

            if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){
                mouse_x = evento.mouse.x;
                mouse_y = evento.mouse.y;
                if (mouseNoBotao(&botao_retornar, mouse_x, mouse_y))
                    botao_retornar.cor_fundo = cor_mouse;
                else
                    botao_retornar.cor_fundo = cor_botao;
            }
            else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                if (mouseNoBotao(&botao_retornar, mouse_x, mouse_y)){
                    printf("VOLTANDO AO MENU\n");
                    sair = 0;
                }
            }
            else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if (mouseNoBotao(&botao_retornar, mouse_x, mouse_y))
                    botao_retornar.cor_fundo = cor_click;
            }
        }
    }
    destroiBotao(&botao_retornar);
    al_destroy_event_queue(filaEventos);
    return sair;
}

int telaRank(void){
    al_flip_display();
    ALLEGRO_EVENT_QUEUE *filaEventos = NULL;
    filaEventos = al_create_event_queue();
    al_register_event_source(filaEventos, al_get_mouse_event_source());
    ALLEGRO_EVENT evento;
    ALLEGRO_BITMAP *planoFundo = NULL;
    ALLEGRO_FONT *fonteTexto1 = NULL, *fonteTexto2 = NULL;
    fonteTexto1 = al_load_font("_fontes/8BIT.ttf", 18, 0);
    fonteTexto2 = al_load_font("_fontes/8BIT.ttf", 14, 0);
    botao botao_retornar;
    iniciaBotao(&botao_retornar, "VOLTAR!\0", cor_texto, TELA_LARGURA / 2, 400, cor_botao, 250, 60);
    planoFundo = al_load_bitmap("_imagens/fundoRank.png");
    //LENDO ARQ
    int *placar = NULL, i;
    char **jogadores = NULL;
    placar = (int *) malloc((MAX_SALVOS + 1) * sizeof(int));
    confereNULL(&placar);
    jogadores = (char **) malloc((MAX_SALVOS + 1) * sizeof(char *));
    confereNULL(&jogadores);
    for (i = 0; i < (MAX_SALVOS + 1); i++){
        jogadores[i] = (char *) malloc(4 * sizeof(char));
        confereNULL(&jogadores[i]);
    }
    FILE *arq = fopen("rankYellowShip.txt", "r");
    for (i = 0; i < MAX_SALVOS + 1; i++){
        fscanf(arq, "%s %d%*c", jogadores[i], &placar[i]);
        jogadores[i][3] = '\0';
    }
    fclose(arq);

    int mouse_x = 0, mouse_y = 0;
    int sair = 42;
    posicao impressaoNick;
    while(sair == 42){
        while(sair == 42 && !al_is_event_queue_empty(filaEventosPrincipal)){
			al_wait_for_event(filaEventosPrincipal, &evento);
            if(evento.type == ALLEGRO_EVENT_TIMER){
                al_clear_to_color(al_map_rgb(255, 255, 255));
                al_draw_bitmap(planoFundo, 0, 0, 0);
                //renderizaPlacar
                for (i = 0; i < (MAX_SALVOS + 1); i++){
                    if (i == 0){
                        impressaoNick.x = 641; impressaoNick.y = 238;
                    }
                    else{
                        impressaoNick.x = (i == 1 || i == 3) ? 120 : 332;
                        impressaoNick.y = (i == 1 || i == 2) ? 160 : 275;
                    }
                    al_draw_textf(fonteTexto1, al_map_rgb(255, 255, 255), impressaoNick.x , impressaoNick.y, ALLEGRO_ALIGN_CENTRE, "%s", jogadores[i]);
                    al_draw_textf(fonteTexto2, al_map_rgb(255, 255, 255), impressaoNick.x , impressaoNick.y + (i == 0 ? 57 : 43), ALLEGRO_ALIGN_CENTRE, "%d", placar[i]);
                }

                renderizaBotao(&botao_retornar, janela, fonte);
                al_flip_display();
            }
			else if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE){
                sair = -1;
                break;
            }
		}
        while(sair == 42 && !al_is_event_queue_empty(filaEventos)){
            al_wait_for_event(filaEventos, &evento);
            if (evento.type == ALLEGRO_EVENT_MOUSE_AXES){
                mouse_x = evento.mouse.x;
                mouse_y = evento.mouse.y;
                if (mouseNoBotao(&botao_retornar, mouse_x, mouse_y))
                    botao_retornar.cor_fundo = cor_mouse;
                else
                    botao_retornar.cor_fundo = cor_botao;
            }
            else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
                if (mouseNoBotao(&botao_retornar, mouse_x, mouse_y)){
                    printf("VOLTANDO AO MENU\n");
                    sair = 0;
                    break;
                }
            }
            else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
                if (mouseNoBotao(&botao_retornar, mouse_x, mouse_y))
                    botao_retornar.cor_fundo = cor_click;
            }
        }
    }
    free(placar);
    for (i = 0; i < MAX_SALVOS + 1; i++)
        free(jogadores[i]);
    free(jogadores);
    destroiBotao(&botao_retornar);
    al_destroy_font(fonteTexto1);
    al_destroy_font(fonteTexto2);
    al_destroy_event_queue(filaEventos);
    al_destroy_bitmap(planoFundo);
    return sair;
}

void iniciaAleatorio(void){
    srand(time(NULL));
}

int aleatorio(int a, int b){
    return (int)((((double) rand()) / RAND_MAX) * (b - a) + a);
}

void confereNULL(void **ptr){
    if((*ptr) == NULL){
        printf("PROBLEMA DURANTE A ALOCACAO DINAMICA\n");
        exit(1);
    }
}

int tamanhoStr(char *str){
    int i;
    for (i = 0; str[i] != '\0'; i++);
    return i;
}

void copiaStr(char *str1, char *str2, int len){
    int i;
    for (i = 0; i <= len; i++){
        str1[i] = str2[i];
    }
}

cor criaCor(int r, int g, int b, int a){
    cor aux;
    aux.r = r;
    aux.g = g;
    aux.b = b;
    aux.a = a;
    return aux;
}

void setCor(botao *ptr, cor cor_fundo){
    ptr->cor_fundo = cor_fundo;
}

void iniciaBotao(botao *ptr, char *texto, cor cor_texto, int x, int y, cor cor_fundo, int largura, int altura){
    ptr->x = x;
    ptr->y = y;
    ptr->cor_texto = cor_texto;
    ptr->texto = (char *) malloc(tamanhoStr(texto) * sizeof(char));
    confereNULL(&ptr->texto);
    copiaStr(ptr->texto, texto, tamanhoStr(texto));
    ptr->largura = largura;
    ptr->altura = altura;
    setCor(ptr, cor_fundo);
}

void destroiBotao(botao *ptr){
    free(ptr->texto);
}

void renderizaBotao(botao *ptr, ALLEGRO_DISPLAY *janela, ALLEGRO_FONT *fonte){
    al_draw_filled_rectangle(ptr->x - (ptr->largura / 2), ((ptr->altura <= 50) ? ptr->y : ptr->y - (ptr->altura - 50) / 2), ptr->x - (ptr->largura / 2) + ptr->largura, ((ptr->altura <= 50) ? ptr->y : ptr->y - (ptr->altura - 50)) + ptr->altura, al_map_rgba(ptr->cor_fundo.r, ptr->cor_fundo.g, ptr->cor_fundo.b, ptr->cor_fundo.a));
    al_draw_text(fonte, al_map_rgba(ptr->cor_texto.r, ptr->cor_texto.g, ptr->cor_texto.b, ptr->cor_texto.a), ptr->x, ptr->y, ALLEGRO_ALIGN_CENTRE, ptr->texto);
}

void encerraBotoes(botao *ptr, int len){
    int i;
    for (i = 0; i < len; i++){
        destroiBotao(&ptr[i]);
    }
}

int mouseNoBotao(botao *ptr, int mouse_x, int mouse_y){
    if (mouse_y > ((ptr->altura <= 50) ? ptr->y : ptr->y - (ptr->altura - 50) / 2) &&
        mouse_y < ((ptr->altura <= 50) ? ptr->y : ptr->y + ptr->altura - (ptr->altura - 50) / 2) &&
        mouse_x > (ptr->x - ptr->largura / 2) &&
        mouse_x < (ptr->x + ptr->largura / 2))
            return 1;
    return 0;
}

void defPosicao(posicao *ptr, int x, int y){
    ptr->x = x;
    ptr->y = y;
}

void iniciaElemento(elemento *ptr, char *img, int x, int y, int largura, int altura){
    ptr->largura = largura;
    ptr->altura = altura;
    ptr->img = NULL;
    ptr->img = al_load_bitmap(img);
    defPosicao(&ptr->pos, x, y);
    ptr->flag = 0;
    defPosicao(&ptr->sprite, 0, 0);
}

void destroiElemento(ALLEGRO_BITMAP *ptr){
    al_destroy_bitmap(ptr);
}

void renderizaElemento(elemento *ptr){
    al_draw_scaled_bitmap(ptr->img, ptr->sprite.x, ptr->sprite.y, al_get_bitmap_width(ptr->img), al_get_bitmap_height(ptr->img), ptr->pos.x, ptr->pos.y, ptr->largura, ptr->altura, 0);
}

void renderizaElementoRotacao(elemento *ptr, int fator){
    al_draw_scaled_rotated_bitmap(ptr->img, ptr->largura / 2, ptr->altura / 2, ptr->pos.x + ptr->largura /2, ptr->pos.y + ptr->altura / 2, (ptr->largura  /  (double) al_get_bitmap_width(ptr->img)), (ptr->altura / (double) al_get_bitmap_height(ptr->img)), (-1/60.0) * fator, 0);
}

void renderizaElementoSprite(elemento *ptr, int x, int y){
    al_draw_scaled_bitmap(ptr->img, x + 47, y + 51, 256, 256, ptr->pos.x, ptr->pos.y, ptr->largura * 2 - 19, ptr->altura * 2 , 0);
}

void geraMeteoro(elemento **meteoros){
    int i, pos = -1;
    for (i = 0; i < MAX_METEOROS; i++)
        if (meteoros[i] == NULL){
            pos = i;
            break;
        }
    if (pos != -1){
        elemento *meteoro;
        double proporcao = (aleatorio(47, 110) / 100.0);
        int esc = aleatorio(1, 100);
        meteoro = (elemento *) malloc(sizeof(elemento));
        confereNULL(&meteoro);
        if (esc <= 10){
            iniciaElemento(meteoro, "_imagens/meteoro/municao.png", TELA_LARGURA + 100, aleatorio(0, 440) - 10, 98 * proporcao, 94 * proporcao);
            meteoro->flag = 3;
        }
        else if (esc <= 55)
            iniciaElemento(meteoro, "_imagens/meteoro/marrom.png", TELA_LARGURA + 100, aleatorio(0, 440) - 10, 98 * proporcao, 94 * proporcao);
        else
            iniciaElemento(meteoro, "_imagens/meteoro/cinza.png", TELA_LARGURA + 100, aleatorio(0, 440) - 10, 98 * proporcao, 94 * proporcao);

        meteoros[pos] = meteoro;
    }
}

void geraTiro(elemento **tiros, int x, int y){
    int i, pos = -1;
    for (i = 0; i < MAX_TIROS; i++)
        if (tiros[i] == NULL){
            pos = i;
            break;
        }
    if (pos != -1){
        elemento *tiro;
        tiro = (elemento *) malloc(sizeof(elemento));
        confereNULL(&tiro);
        iniciaElemento(tiro, "_imagens/tiro/tiro1.png", x, y, 62, 13);
        tiros[pos] = tiro;
    }
}

int colisao(elemento *a, elemento *b){ //retorna 1 caso tenha ocorrido colisao e 0 caso contrario
    posicao p;
    p.x = a->pos.x + a->largura / 2;
    p.y = a->pos.y + a->altura / 2;
    if ((((a->pos.x >= b->pos.x && a->pos.x <= b->pos.x + b->largura) ||
        (p.x >= b->pos.x && p.x <= b->pos.x + b->largura)) &&
        ((a->pos.y >= b->pos.y && a->pos.y <= b->pos.y + b->altura) ||
        (a->pos.y + a->altura >= b->pos.y + b->altura && a->pos.y <= b->pos.y))) ||
        (p.x > b->pos.x && p.x < b->pos.x + b->largura &&
        p.y > b->pos.y && p.y < b->pos.y + b->altura))
            return 1;
    return 0;
}

void addBalas(naveEspacial *ptr){
    ptr->balas += aleatorio(1,4);
    if(ptr->balas >= 100 )
        ptr->balas = 99;
}

void dificultaJogo(int pontuacao, int *velocidadeMeteoro, int *intervaloTempo){
    if (pontuacao == 1500){
        *(velocidadeMeteoro) += 2;
    }
    else if (pontuacao == 1000){
        *(intervaloTempo) -= 10;
    }
    else if (pontuacao == 900){
        (*intervaloTempo) -= 5;
    }
    else if (pontuacao == 800){
        (*velocidadeMeteoro) += 2;
    }
    else if (pontuacao == 650){
        (*velocidadeMeteoro)++;
        (*intervaloTempo) -= 5;
    }
    else if (pontuacao == 500){
        (*velocidadeMeteoro)++;
        (*intervaloTempo) -= 10;
    }
    else if (pontuacao == 400){
        (*velocidadeMeteoro)++;
    }
    else if(pontuacao == 300){
        (*intervaloTempo) -= 5;
    }
    else if (pontuacao == 180){
        (*velocidadeMeteoro)++;
    }
    else if (pontuacao == 100){
        (*intervaloTempo) -= 10;
    }
}

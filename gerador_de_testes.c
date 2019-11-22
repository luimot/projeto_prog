/*============================================================================*/
/* GERADOR DE CASOS DE TESTE PARA O PROBLEMA DO ESTIMADOR DE DISTANCIA        */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu - nassu@dainf.ct.utfpr.edu.br                       */
/*============================================================================*/
/** Um gerador de casos de teste para o problema do estimador de distancia.
 * O gerador funciona abrindo uma imagem de referência e adicionando sobre ela
 * a imagem de um veículo, além de algum ruído. A imagem é rotacionada. Cada
 * teste contém 2 imagens, com o veículo em diferentes locais, além de uma
 * estimativa da distancia. */
/*============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gerador_de_testes.h"

/*============================================================================*/

#define PI 3.1415926535897932384626433832795

#define MOVIMENTO_MINIMO 5 /* Mínimo movimento aceitável. */
#define W_JANELA_CENTRAL 768 /* O veículo deve aparecer em uma janela WxW no centro da imagem de fundo (antes da rotação). */
#define PARAMETRO_SUAVIZACAO 20 /* Quanto menor, mais borradas as imagens tendem a ficar. */
#define SIGMA_RUIDO 10 /* Quanto maior, mais ruído. */

/* Estes parâmetros se referem às imagens usadas pelo gerador. Idealmente, este
 tipo de coisa não deveria ficar no fonte, mas aqui o ganho em generalidade que
 teríamos usando um arquivo de configurações simplesmente não compensaria a
 complexidade extra! */

#define BACKGROUND_FILE "gen/back.bmp"

#define N_VEICULOS 7

char* VEICULOS [] =
{
	"gen/VEI1.BMP",
	"gen/VEI2.BMP",
	"gen/VEI3.BMP",
	"gen/VEI4.BMP",
	"gen/VEI5.BMP",
	"gen/VEI6.BMP",
	"gen/VEI7.BMP",
};

#define N_TAMANHOS 5
int LARGURAS [] = {1024, 768, 768, 1024, 800};
int ALTURAS [] = {768, 1024, 768, 1024, 800};

/*============================================================================*/

int insereVeiculo (GeradorDeTestes* g, Imagem* img1, Imagem* img2, Imagem* img_veiculo);
void desenhaVeiculo (Imagem* img_veiculo, int pos, int pega_r, int pega_g, int pega_b, Imagem* out);
void copiaImagem (Imagem* img, Imagem* copia);
void suavizaImagem (Imagem* img);
double rand_normal (double mean, double stddev);
void adicionaRuido (Imagem* img);
void capturaJanelaRotacionada (Imagem* img, double angulo, Imagem* out);

#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)

/*============================================================================*/
/** Cria um gerador de testes.
 *
 * Parâmetros: nenhum.
 *
 * Valor de retorno: ponteiro para o gerador de testes criado. */

GeradorDeTestes* criaGeradorDeTestes ()
{
	int i;
	GeradorDeTestes* g;
	g = (GeradorDeTestes*) malloc (sizeof (GeradorDeTestes));

	/* Lê já TODAS as imagens de veículos. É mais eficiente que ler tudo de novo para cada imagem gerada... */
	g->img_veiculos = (Imagem**) malloc (sizeof (Imagem*) * N_VEICULOS);
	for (i = 0; i < N_VEICULOS; i++)
	{
		g->img_veiculos [i] = abreImagem (VEICULOS [i]);
		if (!g->img_veiculos [i])
			return (NULL);
	}

	/* Lê o fundo. */
	g->background = abreImagem (BACKGROUND_FILE);
	if (!g->background)
        return (NULL);

	return (g);
}

/*----------------------------------------------------------------------------*/
/** Desaloca um gerador de testes.
 *
 * Parâmetros: GeradorDeTestes* g: gerador de testes a destruir.
 *
 * Valor de retorno: nenhum. */

void destroiGeradorDeTestes (GeradorDeTestes* g)
{
	int i;

    destroiImagem (g->background);

	/* Joga fora as imagens dos veículos. */
	for (i = 0; i < N_VEICULOS; i++)
		destroiImagem (g->img_veiculos [i]);
	free (g->img_veiculos);

	free (g);
}

/*----------------------------------------------------------------------------*/
/** Função central do gerador, cria um caso de testes.
 *
 * Parâmetros: GeradorDeTestes* g: o gerador a usar.
 *             int seed: a semente para o gerador de números aleatórios.
 *             Imagem** img1: ponteiro para a primeira imagem. Será alocada
 *               aqui dentro!
 *             Imagem** img2: ponteiro para a segunda imagem. Será alocada
 *               aqui dentro!
 *             Imagem** bg: ponteiro para o background (rotacionado). Será
 *               alocado aqui dentro.
 *
 * Valor de retorno: a distância percorrida pelo veículo. */

double geraCasoDeTeste (GeradorDeTestes* g, int seed, Imagem** img1, Imagem** img2, Imagem** bg)
{
    static Imagem* img1_base = NULL;
    static Imagem* img2_base = NULL;
    Imagem* img_veiculo;
    double angulo;
    int dist, tamanho_saida;

	srand (seed);

	/* Copia a imagem do fundo. */
	if (!img1_base)
	{
        img1_base = criaImagem (g->background->largura, g->background->altura, 3);
        if (!img1_base)
            return (-1);
        img2_base = criaImagem (g->background->largura, g->background->altura, 3);
        if (!img2_base)
            return (-1);
	}

	copiaImagem (g->background, img1_base);
	copiaImagem (g->background, img2_base);

	/* Insere o veículo nas imagens. */
    img_veiculo = g->img_veiculos [rand () % N_VEICULOS];
	dist = insereVeiculo (g, img1_base, img2_base, img_veiculo);

	/* Seleciona uma janela rotacionada. */
	tamanho_saida = rand () % N_TAMANHOS;
	*img1 = criaImagem (LARGURAS [tamanho_saida], ALTURAS [tamanho_saida], 3);
	*img2 = criaImagem (LARGURAS [tamanho_saida], ALTURAS [tamanho_saida], 3);
    *bg = criaImagem (LARGURAS [tamanho_saida], ALTURAS [tamanho_saida], 3);

    angulo = (((double) rand ()) / ((double) RAND_MAX)) * 2 * PI;
    capturaJanelaRotacionada (img1_base, angulo, *img1);
    capturaJanelaRotacionada (img2_base, angulo, *img2);
    capturaJanelaRotacionada (g->background, angulo, *bg);

    /* Suaviza um pouco as imagens. */
	suavizaImagem (*img1);
	suavizaImagem (*img2);

	/* Adiciona um pouco de ruído. */
	adicionaRuido (*img1);
	adicionaRuido (*img2);

    return ((double) dist);
}

/*----------------------------------------------------------------------------*/
/** Insere um veículo em um par de imagens.
 *
 * Parâmetros: GeradorDeTestes* g: o gerador usado.
 *             Imagem* img1: primeira imagem.
 *             Imagem* img2: segunda imagem.
 *             Imagem* img_veiculo: imagem do veículo a inserir.
 *
 * Valor de retorno: a distância (em pixels) entre os dois veículos. */

int insereVeiculo (GeradorDeTestes* g, Imagem* img1, Imagem* img2, Imagem* img_veiculo)
{
	int pega_r, pega_g, pega_b; /* Variáveis usadas para definir as cores do veículo. */
	int margem_esquerda, margem_direita, pos1, pos2; /* Variáveis usadas para controlar a posição na qual o veículo é desenhado. */

    /* Define uma variação de cor. Nos arquivos originais, o canal dominante é
	 o R, o G é o intermediário, e o B é o canal mais fraco. Para gerar as
	 variações, selecionamos um canal aleatório da imagem original para cada
	 canal da imagem colada. */
    do
    {
        pega_r = rand () % 3;
        pega_g = rand () % 3;
        pega_b = rand () % 3;
    } while ((pega_r == 2 || pega_r == 1) && pega_r == pega_g && pega_r == pega_b); /* Evita cinza (que é confundido com o asfalto). */

    /* Define a posição nas imagens. Começamos definindo uma margem esquerda e
     uma margem direita. A esquerda do veículo deve estar entre estas margens
     em ambas as imagens. */
    margem_esquerda = (g->background->largura - W_JANELA_CENTRAL)/2;
    margem_direita = g->background->largura - margem_esquerda - img_veiculo->largura;

    /* Primeira imagem. */
    pos1 = margem_esquerda + (rand () % (margem_direita - margem_esquerda - MOVIMENTO_MINIMO));
    desenhaVeiculo (img_veiculo, pos1, pega_r, pega_g, pega_b, img1);

    /* Segunda imagem. */
    pos2 = pos1 + MOVIMENTO_MINIMO + (rand () % (margem_direita - pos1 - MOVIMENTO_MINIMO));
    desenhaVeiculo (img_veiculo, pos2, pega_r, pega_g, pega_b, img2);

    return (pos2-pos1);
}

/*----------------------------------------------------------------------------*/
/** Desenha um veículo em uma imagem. Na horizontal, a posição do veículo é
 * dada, na vertical ele é desenhado sempre no centro da imagem.
 *
 * Parâmetros: Imagem* img_veiculo: imagem do veículo.
 *             int pos: coluna mais à esquerda do veículo.
 *             int pega_r, int pega_g, int pega_b: indicam os canais da imagem
 *               original selecionados para a imagem de saída. Permitem que
 *               os canais originais do veículo sejam recombinados.
 *             Imagem* out: imagem de saída.
 *
 * Valor de retorno: nenhum. */

void desenhaVeiculo (Imagem* img_veiculo, int pos, int pega_r, int pega_g, int pega_b, Imagem* out)
{
    int row, col, row_out, col_out;

    row_out = (out->altura - img_veiculo->altura)/2;
	for (row = 0; row < img_veiculo->altura; row++, row_out++)
	{
	    col_out = pos;

		for (col = 0; col < img_veiculo->largura; col++, col_out++)
		{
            /* Magenta == translúcido. */
            if (img_veiculo->dados [0][row][col] == img_veiculo->dados [2][row][col] && img_veiculo->dados [1][row][col] == 0)
            {
                float tone = img_veiculo->dados [0][row][col] / 255.0f;
                out->dados [0][row_out][col_out] = (int) (out->dados [0][row_out][col_out] * tone);
                out->dados [1][row_out][col_out] = (int) (out->dados [1][row_out][col_out] * tone);
                out->dados [2][row_out][col_out] = (int) (out->dados [2][row_out][col_out] * tone);
            }
            else
            {
                out->dados [0][row_out][col_out] = img_veiculo->dados [pega_r][row][col];
                out->dados [1][row_out][col_out] = img_veiculo->dados [pega_g][row][col];
                out->dados [2][row_out][col_out] = img_veiculo->dados [pega_b][row][col];
            }
		}
	}
}

/*----------------------------------------------------------------------------*/
/** Copia o conteúdo de uma imagem para outra.
 *
 * Parâmetros: Imagem* img: imagem a copiar.
 *             Imagem* copia: imagem de destino. Pressupomos que tem o mesmo
 *               tamanho da imagem de entrada.
 *
 * Valor de retorno: nenhum. */

void copiaImagem (Imagem* img, Imagem* copia)
{
	unsigned long i, j, k;

	for (i = 0; i < 3; i++)
		for (j = 0; j < copia->altura; j++)
			for (k = 0; k < copia->largura; k++)
				copia->dados [i][j][k] = img->dados [i][j][k];
}

/*----------------------------------------------------------------------------*/
/** Suaviza a imagem com um filtro 3x3. O peso do pixel central é escolhido
 * aleatoriamente, ou seja, a imagem pode ficar mais ou menos borrada.
 *
 * Parâmetros: Imagem* img: imagem a suavizar.
 *
 * Valor de retorno: nenhum */

void suavizaImagem (Imagem* img)
{
	unsigned long row, col;
	int canal, peso_centro;
	Imagem* copia; /* Precisamos guardar os dados originais. Seria bem mais eficiente guardar apenas uma janela, mas copiar tudo é bem mais simples... */
	copia = criaImagem (img->largura, img->altura, 3);
	copiaImagem (img, copia);

	peso_centro = rand () % PARAMETRO_SUAVIZACAO + 6;
	for (canal = 0; canal < 3; canal++)
		for (row = 1; row < img->altura-1; row++)
			for (col = 1; col < img->largura-1; col++)
				img->dados [canal][row][col] = (copia->dados [canal][row-1][col-1] +
				                                copia->dados [canal][row-1][col]*2 +
												copia->dados [canal][row-1][col+1] +
			                                    copia->dados [canal][row][col-1]*2 +
												copia->dados [canal][row][col]*peso_centro +
												copia->dados [canal][row][col+1]*2 +
												copia->dados [canal][row+1][col-1] +
												copia->dados [canal][row+1][col]*2 +
												copia->dados [canal][row+1][col+1])/(peso_centro + 12);
	destroiImagem (copia);
}

/*----------------------------------------------------------------------------*/
/** Adiciona ruído a uma imagem.
 *
 * Parâmetros: Imagem* img: imagem à qual adicionaremos ruído.
 *
 * Valor de retorno: nenhum. */

void adicionaRuido (Imagem* img)
{
	unsigned long row, col, canal;

	for (canal = 0; canal < 3; canal++)
		for (row = 0; row < img->altura; row++)
			for (col = 0; col < img->largura; col++)
			{
				double ruido = rand_normal (0, SIGMA_RUIDO);
				img->dados [canal][row][col] = (unsigned char) (MIN (255, MAX (0, img->dados [canal][row][col] + ruido)));
			}
}

/*----------------------------------------------------------------------------*/
/** Código copiado de outro lugar para gerar números aleatórios seguindo uma
 * distribuição normal. Original em:
 * en.literateprograms.org/Box-Muller_transform_%28C%29#chunk%20use:complete%20Box-Muller%20function */

double rand_normal (double mean, double stddev)
{
	static double n2 = 0.0;
	static int n2_cached = 0;
	if (!n2_cached) {
		double x, y, r;
		do {
			x = 2.0*rand()/RAND_MAX - 1;
			y = 2.0*rand()/RAND_MAX - 1;
			r = x*x + y*y;
		} while (r == 0.0 || r > 1.0);
		{
			double result;
			double d = sqrt(-2.0*log(r)/r);
			double n1 = x*d;
			n2 = y*d;
			result = n1*stddev + mean;
			n2_cached = 1;
			return result;
		}
	}
	else {
		n2_cached = 0;
		return n2*stddev + mean;
	}
}

/*----------------------------------------------------------------------------*/
/** Captura uma janela rotacionada de uma imagem.
 *
 * Parâmetros: Imagem* img: imagem original.
 *             double angulo: ângulo de rotação.
 *             Imagem* out: imagem de saída.
 *
 * Valor de retorno: nenhum. */

void capturaJanelaRotacionada (Imagem* img, double angulo, Imagem* out)
{
    int row, col;
    double se, co, cx_co, cx_se, cy_co, cy_se;
    double cx = out->largura/2, cy = out->altura/2;
    double margin_x = (img->largura-out->largura)/2, margin_y = (img->altura-out->altura)/2;

    /* Valores pré-computados, para evitar multiplicações desnecessárias. */
    se = sin (angulo);
    co = cos (angulo);
    cx_co = cx*co;
    cx_se = cx*se;
    cy_co = cy*co;
    cy_se = cy*se;

    /* Para cada pixel da imagem de saída... */
    for (row = 0; row < out->altura; row++)
    {
        double row_se = row*se, row_co = row*co;

        for (col = 0; col < out->largura; col++)
        {
            /* De onde veio este pixel? */
            double x, y;
            x = col*co - row_se - cx_co + cy_se + cx + margin_x;
            y = col*se + row_co - cx_se - cy_co + cy + margin_y;

            if (floor (x) >= 0 && floor (y) >= 0 && ceil (x) < img->largura && ceil (y) < img->altura)
            {
                double peso_floor_x, peso_ceil_x, peso_floor_y, peso_ceil_y;

                if (x == floor (x))
                {
                    peso_floor_x = 1;
                    peso_ceil_x = 0;
                }
                else
                {
                    peso_floor_x = ceil (x) - x;
                    peso_ceil_x = x - floor (x);
                }

                if (y == floor (y))
                {
                    peso_floor_y = 1;
                    peso_ceil_y = 0;
                }
                else
                {
                    peso_floor_y = ceil (y) - y;
                    peso_ceil_y = y - floor (y);
                }

                /* Preenche usando interpolação bilinear nos 3 canais. */
                out->dados [0][row][col] = img->dados [0][(int) floor (y)][(int) floor (x)] * peso_floor_x * peso_floor_y +
                                           img->dados [0][(int) ceil (y)][(int) floor (x)] * peso_floor_x * peso_ceil_y +
                                           img->dados [0][(int) floor (y)][(int) ceil (x)] * peso_ceil_x * peso_floor_y +
                                           img->dados [0][(int) ceil (y)][(int) ceil (x)] * peso_ceil_x * peso_ceil_y;

                out->dados [1][row][col] = img->dados [1][(int) floor (y)][(int) floor (x)] * peso_floor_x * peso_floor_y +
                                           img->dados [1][(int) ceil (y)][(int) floor (x)] * peso_floor_x * peso_ceil_y +
                                           img->dados [1][(int) floor (y)][(int) ceil (x)] * peso_ceil_x * peso_floor_y +
                                           img->dados [1][(int) ceil (y)][(int) ceil (x)] * peso_ceil_x * peso_ceil_y;

                out->dados [2][row][col] = img->dados [2][(int) floor (y)][(int) floor (x)] * peso_floor_x * peso_floor_y +
                                           img->dados [2][(int) ceil (y)][(int) floor (x)] * peso_floor_x * peso_ceil_y +
                                           img->dados [2][(int) floor (y)][(int) ceil (x)] * peso_ceil_x * peso_floor_y +
                                           img->dados [2][(int) ceil (y)][(int) ceil (x)] * peso_ceil_x * peso_ceil_y;
            }
            else
            {
                /* Não deve cair aqui, mas por segurança... */
                out->dados [0][row][col] = 0;
                out->dados [1][row][col] = 0;
                out->dados [2][row][col] = 0;
            }
        }
    }
}

/*============================================================================*/

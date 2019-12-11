/*============================================================================*/
/* IF61C - 2014-1 - TRABALHO 3                                                */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu - nassu@dainf.ct.utfpr.edu.br                       */
/*============================================================================*/
/** Teste para o trabalho 3 (cálculo da distânica percorrida por veículos). */
/*============================================================================*/

#include <stdio.h>
#include <time.h>
#include <math.h>

#include "gerador_de_testes.h"
#include "trabalho3.h"

/*============================================================================*/
/* Um modificador para a semente usada pelo gerador de números aleatórios na
 * hora de criar imagens de teste. Todos os trabalhos usarão as mesmas
 * imagens, então o offset precisa ser igual para todos. Entretanto, usaremos
 * um valor diferente na hora de testar os trabalhos de fato. */

#define RANDOM_SEED_OFFSET 1000

#define N_TESTES 500

#define SALVA_INTERMEDIARIOS 0 /* Flag que diz se devemos salvar as imagens de teste. Desative se for rodar muitos testes! */

/*============================================================================*/

int main ()
{
	int i;
	GeradorDeTestes* gerador; /* Gerador para os casos de teste. */
	Imagem *img1, *img2, *bg; /* Imagens em 2 instantes de tempo, além de um background. */
	clock_t tempo_inicio, tempo_total = 0;
	double erro, erro_total = 0, maior_erro = 0;
	int pior_teste = -1;
	double distancia_real, distancia_medida;

    /* Cria o gerador de testes. */
	gerador = criaGeradorDeTestes ();
	if (!gerador)
	{
		printf ("Nao conseguiu carregar o gerador!\n");
		return (1);
	}

    /* Realiza N_TESTES testes. */
	for (i = 0; i < N_TESTES; i++)
	{
		printf ("----- Teste %d\n", i);

		/* Gera um caso de teste. */
		img1 = img2 = bg = NULL;
		distancia_real = geraCasoDeTeste (gerador, i+RANDOM_SEED_OFFSET, &img1, &img2, &bg);
		if (!img1 || !img2 || distancia_real < 0)
		{
			printf ("Erro gerando caso de teste (avisar o professor!!!).\n");
			return (1);
		}

		/* Invoca o testador. */
		tempo_inicio = clock ();
		distancia_medida = calculaDistancia (bg, img1, img2);
		tempo_total += clock () - tempo_inicio;

		/* Compara os resultados com o que foi gerado. */
		if (isfinite (distancia_medida))
            erro = fabs (distancia_real - distancia_medida);
        else
            erro = distancia_real;
        erro_total += erro;

        if (erro > maior_erro)
        {
            maior_erro = erro;
            pior_teste = i;
        }

        printf ("Distancia real: %.2f, Medida: %.2f, Erro: %.2f\n", distancia_real, distancia_medida, erro);

		/* Salva (se necessário). */
		if (SALVA_INTERMEDIARIOS)
		{
			char foostring [64];
			sprintf (foostring, "teste%d-1.bmp", i);
			salvaImagem (img1, foostring);
			sprintf (foostring, "teste%d-2.bmp", i);
			salvaImagem (img2, foostring);
			sprintf (foostring, "teste%d-bg.bmp", i);
			salvaImagem (bg, foostring);
		}

		/* Joga fora as imagens geradas. */
		destroiImagem (img1);
		destroiImagem (img2);
		destroiImagem (bg);
	}

	/* Mostra os resultados. */
	printf ("----------------------------------------\n");
	printf ("Erro medio: %.2f\n", erro_total / N_TESTES);
	printf ("Pior teste: %d (erro: %.2f)\n", pior_teste, maior_erro);
	printf ("Tempo total: %d\n", (int) tempo_total);

	/* Final. */
	destroiGeradorDeTestes (gerador);
	return (0);
}

/*============================================================================*/

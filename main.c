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
	int cont2=0,cont5=0,cont20=0,cont100=0,contC=0;
	int pos2[N_TESTES]={0},pos5[N_TESTES]={0},pos20[N_TESTES]={0},pos100[N_TESTES]={0};
    /* Realiza N_TESTES testes. */
	for (i = 1; i <= N_TESTES; i++)
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
		distancia_medida = calculaDistancia (bg, img1, img2,i);
		tempo_total += clock () - tempo_inicio;

		/* Compara os resultados com o que foi gerado. */
		if (isfinite (distancia_medida))
            erro = fabs (distancia_real - distancia_medida);
        else
            erro = distancia_real;
        erro_total += erro;
        if(erro<1)
        	contC++;
        else if(erro>=2 && erro<5){
        	cont2++;pos2[cont2-1]=i;
        }
        else if(erro>=5 && erro<20){
        	cont5++;pos5[cont5-1]=i;
        }
        else if(erro>=20 && erro<100){
        	cont20++;pos20[cont20-1]=i;
        }
        else if(erro>=100){
        	cont100++;pos100[cont100-1]=i;
        }

        if (erro > maior_erro)
        {
            maior_erro = erro;
            pior_teste = i;
        }
        img1->largura=img2->largura;
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
	printf ("Tempo total: %.2f\n", (float)tempo_total/CLOCKS_PER_SEC);
	printf ("%d erros maiores que 2.\n\tTestes: ",cont2);
	for(int x=0;pos2[x]!=0;x++)
		printf("%d ",pos2[x]);
	printf("\n");
	printf ("%d erros maiores que 5.\n\tTestes: ",cont5);
	for(int x=0;pos5[x]!=0;x++)
		printf("%d ",pos5[x]);
	printf("\n");
	printf ("%d erros maiores que 20.\n\tTestes: ",cont20);
	for(int x=0;pos20[x]!=0;x++)
		printf("%d ",pos20[x]);
	printf("\n");
	printf ("%d erros maiores que 100.\n\tTestes: ",cont100);
	for(int x=0;pos100[x]!=0;x++)
		printf("%d ",pos100[x]);
	printf("\n");
	printf("%d testes corretos, acuracia: %.2f%\n",contC,((float)contC/N_TESTES)*100);
	/* Final. */
	destroiGeradorDeTestes (gerador);
	return (0);
}

/*============================================================================*/

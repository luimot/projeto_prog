/*============================================================================*/
/* GERADOR DE CASOS DE TESTE PARA O PROBLEMA DO ESTIMADOR DE DISTANCIA        */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu - nassu@dainf.ct.utfpr.edu.br                       */
/*============================================================================*/
/** Um gerador de casos de teste para o problema do estimador de distancia.
 * O gerador funciona abrindo uma imagem de refer�ncia e adicionando sobre ela
 * a imagem de um ve�culo, al�m de algum ru�do. A imagem � rotacionada. Cada
 * teste cont�m 2 imagens, com o ve�culo em diferentes locais, al�m de uma
 * estimativa da distancia. */
/*============================================================================*/

#ifndef __GERADOR_DE_TESTES_H
#define __GERADOR_DE_TESTES_H

/*============================================================================*/

#include "imagem.h"

/*============================================================================*/

typedef struct
{
	Imagem* background;
	Imagem** img_veiculos;

} GeradorDeTestes;

/*============================================================================*/

GeradorDeTestes* criaGeradorDeTestes ();
void destroiGeradorDeTestes (GeradorDeTestes* g);
double geraCasoDeTeste (GeradorDeTestes* g, int seed, Imagem** img1, Imagem** img2, Imagem** bg);

/*============================================================================*/
#endif /* __GERADOR_DE_TESTES_H */

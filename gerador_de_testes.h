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

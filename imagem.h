/*============================================================================*/
/* UM TIPO PARA MANIPULAÇÃO DE ARQUIVOS BMP                                   */
/*----------------------------------------------------------------------------*/
/* Autor: Bogdan T. Nassu - nassu@dainf.ct.utfpr.edu.br                       */
/*============================================================================*/
/** Este arquivo traz declarações de um tipo e rotinas para manipulação de
 * arquivos bmp. Como temos um propósito puramente didático, apenas um sub-
 * conjunto mínimo do formato foi implementado. Matrizes são usadas para
 * representar os dados. Vetores seriam computacionalmente mais eficientes, mas
 * aqui procuramos priorizar a clareza e a facilidade de uso. */
/*============================================================================*/

#ifndef __IMAGEM_H
#define __IMAGEM_H

/*============================================================================*/

#define CANAL_R 0 /* Constante usada para se referir ao canal vermelho. */
#define CANAL_G 1 /* Constante usada para se referir ao canal verde. */
#define CANAL_B 2 /* Constante usada para se referir ao canal azul. */

typedef struct
{
	unsigned long largura;
	unsigned long altura;
	unsigned short n_canais; /* Número de canais (1 - monocromática, 3 - RGB). */
	unsigned char*** dados; /* Matrizes de dados. Imagens monocromáticas têm somente uma matriz, imagens coloridas têm 3. */
} Imagem;

/*============================================================================*/

Imagem* criaImagem (int largura, int altura, int n_canais);
void destroiImagem (Imagem* img);
Imagem* abreImagem (char* arquivo);
int salvaImagem (Imagem* img, char* arquivo);

/*============================================================================*/
#endif /* __IMAGEM_H */

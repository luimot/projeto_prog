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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "imagem.h"

/*============================================================================*/

unsigned long getLittleEndianULong (unsigned char* buffer);
int leHeaderBitmap (FILE* stream, unsigned long* offset);
int leHeaderDIB (FILE* stream, unsigned long* largura, unsigned long* altura);
int leDados (FILE* stream, Imagem* img);

int salvaHeaderBitmap (FILE* stream, Imagem* img);
int salvaHeaderDIB (FILE* stream, Imagem* img);
int salvaDados (FILE* stream, Imagem* img);
void putLittleEndianULong (unsigned long val, unsigned char* buffer);
void putLittleEndianUShort (unsigned short val, unsigned char* buffer);

/*============================================================================*/
/** Cria uma imagem vazia.
 *
 * Parâmetros: int largura: largura da imagem.
 *             int altura: altura da imagem.
 *             int n_canais: 1 (escala de cinza) ou 3 (RGB).
 *
 * Valor de retorno: a imagem alocada. A responsabilidade por desalocá-la é do
 *                   chamador. Se algum erro ocorrer, retorna NULL. */

Imagem* criaImagem (int largura, int altura, int n_canais)
{
	int i, j;
	Imagem* img;

	if (n_canais != 3 && n_canais != 1)
	{
		printf ("Erro criando imagem: o numero de canais precisa ser 1 ou 3.\n");
		return (NULL);
	}

	img = (Imagem*) malloc (sizeof (Imagem));
	img->largura = largura;
	img->altura = altura;
	img->n_canais = n_canais;

	img->dados = (unsigned char***) malloc (sizeof (unsigned char**) * n_canais); /* Uma matriz por canal. */
	for (i = 0; i < n_canais; i++)
	{
		img->dados [i] = (unsigned char**) malloc (sizeof (unsigned char*) * altura);
		for (j = 0; j < altura; j++)
			img->dados [i][j] = (unsigned char*) malloc (sizeof (unsigned char) * largura);
	}

	return (img);
}

/*----------------------------------------------------------------------------*/
/** Destroi uma imagem dada.
 *
 * Parâmetros: Imagem* img: a imagem a destruir.
 *
 * Valor de retorno: nenhum. */

void destroiImagem (Imagem* img)
{
	unsigned long i, j;

	for (i = 0; i < img->n_canais; i++)
	{
		for (j = 0; j < img->altura; j++)
			free (img->dados [i][j]);
		free (img->dados [i]);
	}
	free (img->dados);
	free (img);
}

/*----------------------------------------------------------------------------*/
/** Abre um arquivo de imagem dado.
 *
 * Parâmetros: char* arquivo: caminho do arquivo a abrir.
 *
 * Valor de retorno: uma imagem alocada contendo os dados do arquivo, ou NULL
 *                   se não for possível abrir a imagem. */

Imagem* abreImagem (char* arquivo)
{
	FILE* stream;
	unsigned long data_offset = 0, largura = 0, altura = 0;
	Imagem* img;

	/* Abre o arquivo. */
	stream = fopen (arquivo, "rb");
	if (!stream)
		return (NULL);

	if (!leHeaderBitmap (stream, &data_offset))
	{
		fclose (stream);
		return (NULL);
	}

	if (!leHeaderDIB (stream, &largura, &altura))
	{
		fclose (stream);
		return (NULL);
	}

	/* Pronto, cabeçalhos lidos! Vamos agora colocar o fluxo nos dados... */
	if (fseek (stream, data_offset, SEEK_SET) != 0)
	{
		printf ("Error reading file data.\n");
		fclose (stream);
		return (NULL);
	}

	/* ... e tudo pronto para criar nossa imagem! */
	img = criaImagem (largura, altura, 3);
	if (!img)
	{
		printf ("Error creating image.\n");
		fclose (stream);
		return (NULL);
	}

	/* Lê os dados. */
	if (!leDados (stream, img))
	{
		printf ("Error reading data from file.\n");
		fclose (stream);
		free (img);
		return (NULL);
	}

	fclose (stream);
    return (img);
}

/*----------------------------------------------------------------------------*/
/** Pega os 4 primeiros bytes do buffer e coloca em um unsigned long,
 * considerando os bytes em ordem little endian.
 *
 * Parâmetros: unsigned char* buffer: lê 4 bytes daqui.
 *
 * Valor de Retorno: um unsigned long com os dados do buffer reorganizados. */

unsigned long getLittleEndianULong (unsigned char* buffer)
{
	return (buffer [3] << 24) | (buffer [2] << 16) | (buffer [1] << 8) | buffer [0];
}

/*----------------------------------------------------------------------------*/
/** Lê o header de 14 bytes do formato BMP.
 *
 * Parâmetros: FILE* stream: arquivo a ser lido. Supomos que já está aberto.
 *             unsigned long* offset: parâmetro de saída, é o deslocamento dos
 *               dados a partir do início do arquivo.
 *
 * Valor de Retorno: 1 se não ocorreram erros, 0 do contrário. */

int leHeaderBitmap (FILE* stream, unsigned long* offset)
{
	unsigned char data [14]; /* O bloco tem exatamente 14 bytes. */

	if (fread ((void*) data, 1, 14, stream) != 14)
	{
		printf ("Error reading the Bitmap header.\n");
		return (0);
	}

	/* Os 2 primeiros bytes precisam ser 'B' e 'M'. */
	if (data [0] != 'B' || data [1] != 'M')
	{
		printf ("Error: can read only BM format.\n");
		return (0);
	}

	/* Vou pular todo o resto e ir direto para o offset. */
	*offset = getLittleEndianULong (&(data [10]));
	return (1);
}

/*----------------------------------------------------------------------------*/
/** Lê o header DIB.
 *
 * Parâmetros: FILE* stream: arquivo a ser lido. Supomos que já está aberto.
 *             unsigned long* largura: parâmetro de saída. Largura da imagem.
 *             unsigned long* altura: parâmetro de saída. Altura da imagem.
 *
 * Valor de Retorno: 1 se não ocorreram erros, 0 do contrário. */

int leHeaderDIB (FILE* stream, unsigned long* largura, unsigned long* altura)
{
	unsigned long size = 0; /* O tamanho do cabeçalho DIB. */

	if (fread ((void*) &size, 4, 1, stream) != 1)
	{
		printf ("Error reading DIB header.\n");
		return (0);
	}

	if (size == 12) /* Formato BITMAPCOREHEADER. */
	{
		printf ("Error: BITMAPCOREHEADER not supported (is this file really THAT old!?)\n");
		return (0);
	}
	else if (size >= 40) /* Outros formatos. */
	{
		unsigned short tmp_short = 0;
		unsigned long tmp_long = 0;

		/* Largura. */
		if (fread ((void*) largura, 4, 1, stream) != 1 || *largura <= 0)
		{
			printf ("Error: invalid width.\n");
			return (0);
		}

		/* Altura. */
		if (fread ((void*) altura, 4, 1, stream) != 1 || *altura <= 0)
		{
			printf ("Error: invalid height.\n");
			return (0);
		}

		/* Color planes. Precisa ser 1. */
		if (fread ((void*) &tmp_short, 2, 1, stream) != 1 || tmp_short != 1)
		{
			printf ("Error reading DIB header.\n");
			return (0);
		}

		/* Bpp. Aqui, estou forçando 24 bpp. */
		if (fread ((void*) &tmp_short, 2, 1, stream) != 1 || tmp_short != 24)
		{
			printf ("Error: this function supports only 24 bpp files.\n");
			return (0);
		}

		/* Compressão. Vou aceitar só imagens sem compressão. */
		if (fread ((void*) &tmp_long, 4, 1, stream) != 1 || tmp_long != 0)
		{
			printf ("Error: this function supports only uncompressed files.\n");
			return (0);
		}

		/* Pula os próximos 12 bytes. */
		if (fseek (stream, 12, SEEK_CUR) != 0)
		{
			printf ("Error reading DIB header.\n");
			return (0);
		}

		/* Paleta. Não é para usar! */
		if (fread ((void*) &tmp_long, 4, 1, stream) != 1 || tmp_long != 0)
		{
			printf ("Error: this function does not support color palettes.\n");
			return (0);
		}

		return (1);
	}

	return (0);
}

/*----------------------------------------------------------------------------*/
/** Lê os dados de um arquivo.
 *
 * Parâmetros: FILE* stream: arquivo a ser lido. Supomos que já está aberto.
 *             Imagem* img: imagem a preencher.
 *
 * Valor de Retorno: 1 se não ocorreram erros, 0 do contrário. */

int leDados (FILE* stream, Imagem* img)
{
	long long i, j;
	int line_padding;

	/* Calcula quantos bytes preciso pular no fim de cada linha.
	  Aqui, cada linha precisa ter um múltiplo de 4. */
	line_padding = (int) ceil (img->largura*3.0/4.0)*4 - (img->largura*3);

	/* Lê! */
	for (i = img->altura-1; i >= 0; i--)
	{
		for (j = 0; j < img->largura; j++)
		{
			if (fread (&(img->dados [CANAL_B][i][j]), 1, 1, stream) != 1)
				return (0);

			if (fread (&(img->dados [CANAL_G][i][j]), 1, 1, stream) != 1)
				return (0);

			if (fread (&(img->dados [CANAL_R][i][j]), 1, 1, stream) != 1)
				return (0);
		}

		if (fseek (stream, line_padding, SEEK_CUR) != 0)
			return (0);
	}

	return (1);
}

/*----------------------------------------------------------------------------*/
/** Salva uma imagem em um arquivo dado.
 *
 * Parâmetros: Imagem* img: imagem a salvar.
 *             char* arquivo: caminho do arquivo a abrir.
 *
 * Valor de retorno: 0 se ocorreu algum erro, 1 do contrário. */

int salvaImagem (Imagem* img, char* arquivo)
{
	FILE* stream;

	/* Abre o arquivo. */
	stream = fopen (arquivo, "wb");
	if (!stream)
		return (0);

	/* Escreve os blocos. */
	if (!salvaHeaderBitmap (stream, img))
	{
		fclose (stream);
		return (0);
	}

	if (!salvaHeaderDIB (stream, img))
	{
		fclose (stream);
		return (0);
	}

	if (!salvaDados (stream, img))
	{
		fclose (stream);
		return (0);
	}

	fclose (stream);
	return (1);
}

/*----------------------------------------------------------------------------*/
/** Coloca um unsigned long nos 4 primeiros bytes do buffer, em ordem little
 * endian.
 *
 * Parâmetros: unsigned long val: valor a escrever.
 *             unsigned char* buffer: coloca o valor aqui.
 *
 * Valor de Retorno: NENHUM */

void putLittleEndianULong (unsigned long val, unsigned char* buffer)
{
	buffer [0] = (unsigned char) val;
	buffer [1] = (unsigned char) (val >> 8);
	buffer [2] = (unsigned char) (val >> 16);
	buffer [3] = (unsigned char) (val >> 24);
}

/*----------------------------------------------------------------------------*/
/** Coloca um unsigned short nos 2 primeiros bytes do buffer, em ordem little
 * endian.
 *
 * Parâmetros: unsigned short val: valor a escrever.
 *             unsigned char* buffer: coloca o valor aqui.
 *
 * Valor de Retorno: NENHUM */

void putLittleEndianUShort (unsigned short val, unsigned char* buffer)
{
	buffer [0] = (unsigned char) val;
	buffer [1] = (unsigned char) (val >> 8);
}

/*----------------------------------------------------------------------------*/
/** Escreve o header Bitmap.
 *
 * Parâmetros: FILE* file: arquivo a ser escrito. Supomos que já está aberto.
 *             Imagem* img: imagem a ser salva.
 *
 * Valor de Retorno: 1 se não ocorreram erros, 0 do contrário. */

int salvaHeaderBitmap (FILE* stream, Imagem* img)
{
	unsigned char data [14]; /* O bloco tem exatamente 14 bytes. */
	int pos = 0;
	unsigned long bytes_por_linha;

	data [pos++] = 'B';
	data [pos++] = 'M';

	/* Tamanho do arquivo. Definimos como sendo 14+40 (dos cabeçalhos) + o espaço dos dados. */
	bytes_por_linha = (unsigned long) ceil (img->largura*3.0/4.0)*4;
	putLittleEndianULong (14+40+img->altura*bytes_por_linha, &(data [pos]));
	pos+=4;

	/* Reservado. */
	putLittleEndianULong (0, &(data [pos]));
	pos+=4;

	/* Offset. Definimos como 14+40 (o tamanho dos cabeçalhos). */
	putLittleEndianULong (14+40, &(data [pos]));

	if (fwrite ((void*) data, 1, 14, stream) != 14)
	{
		printf ("Error writing Bitmap header.\n");
		return (0);
	}

	return (1);
}

/*----------------------------------------------------------------------------*/
/** Escreve o header DIB.
 *
 * Parâmetros: FILE* file: arquivo a ser escrito. Supomos que já está aberto.
 *             Imagem* img: imagem a ser salva.
 *
 * Valor de Retorno: 1 se não ocorreram erros, 0 do contrário. */

int salvaHeaderDIB (FILE* stream, Imagem* img)
{
	unsigned char data [40]; /* O bloco tem exatamente 40 bytes. */
	int pos = 0;
	unsigned long bytes_por_linha;

	/* Tamanho do header. Vamos usar um BITMAPINFOHEADER. */
	putLittleEndianULong (40, &(data [pos]));
	pos += 4;

	/* Largura. */
	putLittleEndianULong (img->largura, &(data [pos]));
	pos += 4;

	/* Altura. */
	putLittleEndianULong (img->altura, &(data [pos]));
	pos += 4;

	/* Color planes. */
	putLittleEndianUShort (1, &(data [pos]));
	pos += 2;

	/* bpp. */
	putLittleEndianUShort (24, &(data [pos]));
	pos += 2;

	/* Compressão. */
	putLittleEndianULong (0, &(data [pos]));
	pos += 4;

	/* Tamanho dos dados. */
	bytes_por_linha = (unsigned long) ceil (img->largura*3.0/4.0)*4;
	putLittleEndianULong (img->altura*bytes_por_linha, &(data [pos]));
	pos += 4;

	/* Resolução horizontal e vertical (simplesmente copiei este valor de algum arquivo!). */
	putLittleEndianULong (0xF61, &(data [pos]));
	pos += 4;
	putLittleEndianULong (0xF61, &(data [pos]));
	pos += 4;

	/* Cores. */
	putLittleEndianULong (0, &(data [pos]));
	pos += 4;
	putLittleEndianULong (0, &(data [pos]));
	pos += 4;

	if (fwrite ((void*) data, 1, 40, stream) != 40)
	{
		printf ("Error writing DIB header.\n");
		return (0);
	}

	return (1);
}

/*----------------------------------------------------------------------------*/
/** Escreve o bloco de dados.
 *
 * Parâmetros: FILE* file: arquivo a ser escrito. Supomos que já está aberto.
 *             Imagem* img: imagem a ser salva.
 *
 * Valor de Retorno: 1 se não ocorreram erros, 0 do contrário. */

int salvaDados (FILE* stream, Imagem* img)
{
	long long i, j;
	unsigned long largura_linha, line_padding;
	unsigned char* linha;
	unsigned long pos_linha;

	/* Calcula quantos bytes preciso pular no fim de cada linha.
	  Aqui, cada linha precisa ter um múltiplo de 4. */
	largura_linha = (unsigned long) ceil (img->largura*3.0/4.0)*4;
	line_padding = largura_linha - (img->largura*3);
	linha = (unsigned char*) malloc (sizeof (unsigned char) * largura_linha);

    for (i = img->altura-1; i >= 0; i--)
	{
		pos_linha = 0;
		for (j = 0; j < img->largura; j++)
		{
			if (img->n_canais == 3)
			{
				linha [pos_linha++] = img->dados [CANAL_B][i][j];
				linha [pos_linha++] = img->dados [CANAL_G][i][j];
				linha [pos_linha++] = img->dados [CANAL_R][i][j];
			}
			else
			{
				linha [pos_linha++] = img->dados [0][i][j];
				linha [pos_linha++] = img->dados [0][i][j];
				linha [pos_linha++] = img->dados [0][i][j];
			}
		}

		for (j = 0; j < line_padding; j++)
			linha [pos_linha++] = 0;

		if (fwrite ((void*) linha, 1, largura_linha, stream) != largura_linha)
		{
			printf ("Error writing image data.\n");
			free (linha);
			return (0);
		}
	}

	free (linha);
	return (1);
}

/*============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "trabalho3.h"
#include "imagem.h"

#define MAX 64
#define V_MIN_TRESHOLD 60	//Valor mínimo para limpar campos com muito ruído no arquivo limpo //Entre 40-60 ta bom
#define TAM_JANELA 3
#define THRESH_INTER 203
#define PRETO 0
#define BRANCO 255
#define R 0
#define G 1
#define B 2
#define LIM_PM 1

#define SALVA_MODELOS 0		//Flag para salvar os modelos

void filtroMedia(Imagem* img, Imagem* out, int winSize);
int mediaVizinhanca(Imagem* v,int x, int y,int t,int canal);
void interseccaoCanais(Imagem* in,Imagem* out);
void alocaDados(Imagem* x);
void desalocaDados(Imagem* x);
void procuraBlob(Imagem in,unsigned long *xi,unsigned long *yi, unsigned long *xf,unsigned long *yf);
void filtroPonto(Imagem* img);
void pontoMedio(Imagem img,unsigned long *vet,int canal);

double calculaDistancia (Imagem* bg, Imagem* img1, Imagem* img2,int i){
	Imagem *copia[2]={img1,img2};
	unsigned long pos_i[2],pos_f[2];
	int somaM=0,maior[2];
	Imagem outLimpa[2];
	filtroMedia(img1,img1,TAM_JANELA);
	filtroMedia(img2,img2,TAM_JANELA);
	for(int im=0;im<2;im++){
		outLimpa[im].altura=bg->altura;
		outLimpa[im].largura=bg->largura;
		outLimpa[im].n_canais=bg->n_canais;
		alocaDados(&outLimpa[im]);
		for(int c=0;c<3;c++){
			for(int i=0;i<outLimpa[im].altura;i++){
				somaM=0;
				for(int j=0;j<outLimpa[im].largura;j++){
					outLimpa[im].dados[c][i][j]= copia[im]->dados[c][i][j] - bg->dados[c][i][j];
					somaM+= outLimpa[im].dados[c][i][j];
					if(outLimpa[im].dados[c][i][j] < V_MIN_TRESHOLD)
						outLimpa[im].dados[c][i][j]=255;
				}
			}
		}
		interseccaoCanais(&outLimpa[im],&outLimpa[im]);
		filtroPonto(&outLimpa[im]);
	}
	if(SALVA_MODELOS){
		char str[2][MAX];
		sprintf(str[0],"Limpa-%d-1.bmp",i);
		sprintf(str[1],"Limpa-%d-2.bmp",i);
		salvaImagem(&outLimpa[0],str[0]);
		salvaImagem(&outLimpa[1],str[1]);
	}
	pontoMedio(outLimpa[0],pos_i,0);
	pontoMedio(outLimpa[1],pos_f,0);
	desalocaDados(&outLimpa[0]);
	desalocaDados(&outLimpa[1]);
	return sqrt( pow(abs(pos_f[0]-pos_i[0]),2)+ pow(abs(pos_f[1]-pos_i[1]),2) );
}

void filtroMedia(Imagem* img, Imagem* out, int winSize){
	int tam=winSize/2;	//Setta o tamanho para pular bordas
	unsigned long i,j;
	for(int canal=0;canal<3;canal++){
	for(i=tam;i<img->altura-tam;i++){
		for(j=tam;j<img->largura-tam;j++){
			out->dados[canal][i][j]=mediaVizinhanca(img,i,j,winSize,canal);
		}
	}
	//preenche as bordas
	if(SALVA_MODELOS){
		for(i=0;i<tam;i++)
			for(j=0;j<img->largura;j++)
				out->dados[canal][i][j]=img->dados[canal][i][j];
		for(i=tam;i<img->altura-tam;i++){
			for(j=0;j<tam;j++)
				out->dados[canal][i][j]=img->dados[canal][i][j];
			for(j=img->largura-tam;j<img->largura;j++)
				out->dados[canal][i][j]=img->dados[canal][i][j];
		}
		for(i=img->altura-tam;i<img->altura;i++)
			for(j=0;j<img->largura;j++)
				out->dados[canal][i][j]=img->dados[canal][i][j];
		}
	}
}

int mediaVizinhanca(Imagem* v,int x, int y,int t,int canal){
	int media=0,tam=t/2;
	for(unsigned long i=x-tam;i<=x+tam;i++)
		for(unsigned long j=y-tam;j<=y+tam;j++)
			media+=v->dados[canal][i][j];
	return media/(t*t);
}

void interseccaoCanais(Imagem* in,Imagem* out){
	for(int i=0;i<in->altura;i++){
		for(int j=0;j<in->largura;j++){
			if((in->dados[R][i][j] + in->dados[G][i][j] + in->dados[B][i][j])/3.0 < THRESH_INTER)
				in->dados[R][i][j] = in->dados[G][i][j] = in->dados[B][i][j]=BRANCO;
			else
				in->dados[R][i][j] = in->dados[G][i][j] = in->dados[B][i][j]=PRETO;
		}
	}
}

void alocaDados(Imagem* x){
	x->dados=(unsigned char***)malloc(sizeof(unsigned char**)*x->n_canais);
		for(int i=0;i<x->n_canais;i++){
			x->dados[i]=(unsigned char**)malloc(sizeof(unsigned char*)*x->altura);
			for(unsigned long j=0;j<x->altura;j++)
				x->dados[i][j]=(unsigned char*)malloc(sizeof(unsigned char)*x->largura);
		}
}

void desalocaDados(Imagem* x){
	for (int i=0;i<x->n_canais;i++){
			for (unsigned long j=0;j<x->altura;j++)
				free (x->dados[i][j]);
			free (x->dados[i]);
		}
		free(x->dados);
}
void filtroPonto(Imagem* img){
    int i,j;
    for(i = 0 ; i < img->altura-5 ; i ++){
        for(j = 0 ; j < img->largura-5 ; j ++){
            if(img->dados[R][i][j] == BRANCO && img->dados[R][i+1][j+1] == PRETO && img->dados[R][i+2][j+2] && img->dados[R][i+2][j+2] == PRETO && img->dados[R][i+3][j+3] == PRETO){
                img->dados[R][i][j] = img->dados[G][i][j] = img->dados[B][i][j] = PRETO;
            }
        }
    }

}
void pontoMedio(Imagem img,unsigned long *vet,int canal){
    unsigned long i,j,k,l,cont=0;
    unsigned long x[2]={0,img.largura},y[2]={0,img.altura};
    //Checa se todos os vizinhos numa janela 3x3 sao brancos
    //e sua coordenada i j sejam as maiores ou menores possiveis
    //e em seguida salva esse ponto como referencia.
    for(i = LIM_PM ; i < img.altura-LIM_PM ; i++){
        for(j = LIM_PM ; j < img.largura-LIM_PM ; j ++){
            for(k = i-LIM_PM ; k <= i +LIM_PM ; k++)
                for(l = j - LIM_PM ; l <= j+LIM_PM ; l++)
                    if(img.dados[canal][k][l]==255)
						cont ++;
            if(cont >= 7){
                if(i > y[0] && j > x[0]){
                    x[0] = j;
                    y[0] = i;
                }
                if(i < y[1] && j < x[1]){
                    x[1] = j;
                    y[1]= i;
                }
            }
            cont=0;
        }
    }
    //faz a media dos pontos de referencia
    vet[0] = (y[0]+y[1])/2;
    vet[1] = (x[0]+x[1])/2;
}

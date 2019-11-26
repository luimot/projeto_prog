#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "trabalho3.h"
#include "imagem.h"

#define MAX 64
#define V_MIN_TRESHOLD 30	//Valor mínimo para limpar campos com muito ruído no arquivo limpo //Entre 40-60 ta bom 
#define TAM_JANELA 3
#define THRESH_INTER 210
#define PRETO 0
#define BRANCO 255
#define R 0
#define G 1
#define B 2

#define SALVA_MODELOS 1

void filtroMedia(Imagem* img, Imagem* out, int winSize);
int mediaVizinhanca(Imagem* v,int x, int y,int t,int canal);
void interseccaoCanais(Imagem* in,Imagem* out);
void alocaDados(Imagem* x);
void desalocaDados(Imagem* x);
void procuraBlob(Imagem in,unsigned long *xi,unsigned long *yi, unsigned long *xf,unsigned long *yf);
void filtroPonto(Imagem* img);

double calculaDistancia (Imagem* bg, Imagem* img1, Imagem* img2,int i){
	Imagem *copia[2]={img1,img2};
	unsigned long pos_i[2],pos_f[2];	
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
				for(int j=0;j<outLimpa[im].largura;j++){
					outLimpa[im].dados[c][i][j]= copia[im]->dados[c][i][j] - bg->dados[c][i][j];
					if(outLimpa[im].dados[c][i][j] < V_MIN_TRESHOLD)
						outLimpa[im].dados[c][i][j]=255;
				}
			}
		}
		filtroPonto(&outLimpa[im]);
		interseccaoCanais(&outLimpa[im],&outLimpa[im]);
	}
	if(SALVA_MODELOS){
		char str[2][MAX];
		sprintf(str[0],"limpa-%d-1.bmp",i);
		sprintf(str[1],"limpa-%d-2.bmp",i);
		salvaImagem(&outLimpa[0],str[0]);
		salvaImagem(&outLimpa[1],str[1]);
	}
	pontoMedio(outLimpa[0],pos_i);
	pontoMedio(outLimpa[1],pos_f);
	printf("pos1 (%ld,%ld)\n pos2 (%ld,%ld)\ndist %f\n",pos_i[0],pos_i[1],pos_f[0],pos_f[1],sqrt( pow(pos_f[0]-pos_i[0],2)+ pow(pos_f[1]-pos_i[1],2) ));
	desalocaDados(&outLimpa[0]);
	desalocaDados(&outLimpa[1]);
	return sqrt( pow(pos_f[0]-pos_i[0],2)+ pow(pos_f[1]-pos_i[1],2) );
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
			if((in->dados[R][i][j] + in->dados[G][i][j] + in->dados[B][i][j])/3.0 < THRESH_INTER){
				in->dados[R][i][j]=BRANCO;in->dados[G][i][j]=BRANCO;in->dados[B][i][j]=BRANCO;
			}
			else{
				in->dados[R][i][j]=PRETO;in->dados[G][i][j]=PRETO;in->dados[B][i][j]=PRETO;
			}
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
    for(i = 0 ; i < img->altura-2 ; i ++){
        for(j = 0 ; j < img->largura-2 ; j ++){
            if(img->dados[R][i][j] == BRANCO &&  img->dados[R][i+1][j+1] == PRETO){
                img->dados[R][i][j] = PRETO;
                img->dados[G][i][j] = PRETO;
                img->dados[B][i][j] = PRETO;
            }
        }
    }

}
void pontoMedio(Imagem img,unsigned long *vet){
    unsigned long i,j,ii,jj,cont=0;
    unsigned long x[2]={0,img.largura},y[2]={0,img.altura};
    //Checa se todos os vizinhos numa janela 5x5 sao pretos
    //e sua coordenada i j sejam as maiores ou menores possiveis
    //e em seguida salva esse ponto como referencia.
    for(i = 2 ; i < img.altura-2 ; i++){
        for(j = 2 ; j < img.largura-2 ; j ++){
            for(ii = i-2 ; ii <= i +2 ; ii++){
                for(jj = j - 2 ; jj <= j+2 ; jj++)
                    cont += img.dados[0][ii][jj];
            }
            if(cont == 0){
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

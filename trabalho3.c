#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "trabalho3.h"
#include "imagem.h"

typedef unsigned long ul;

#define MAX 64
#define V_MIN_TRESHOLD 30	//Valor mínimo para limpar campos com muito ruído no arquivo limpo //Entre 40-60 ta bom 
#define TAM_JANELA 3
#define THRESH_INTER 235

void filtroMedia(Imagem* img, Imagem* out, int winSize);
int mediaVizinhanca(Imagem* v,int x, int y,int t,int canal);
void interseccaoCanais(Imagem* in,Imagem* out);
void alocaDados(Imagem* x);
void desalocaDados(Imagem* x);
void procuraBlob(Imagem in,ul *xi,ul *yi, ul *xf,ul *yf);

double calculaDistancia (Imagem* bg, Imagem* img1, Imagem* img2,int i){
	Imagem *copia[2]={img1,img2};
	ul *xi,*xf,*yi,*yf;
	char str[2][MAX];
	sprintf(str[0],"limpa-%d-1.bmp",i);
	sprintf(str[1],"limpa-%d-2.bmp",i);	
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
		interseccaoCanais(&outLimpa[im],&outLimpa[im]);
		//procuraBlob(outLimpa[1],xi,yi,xf,yf);
		//outLimpa[1].dados[0][*xi][*yi]=255;outLimpa[1].dados[1][*xi][*yi]=0;outLimpa[1].dados[2][*xi][*yi]=0;
		//	outLimpa[1].dados[0][*xf][*yf]=255;outLimpa[1].dados[1][*xf][*yf]=0;outLimpa[1].dados[2][*xf][*yf]=0;
		salvaImagem(&outLimpa[im],str[im]);
		desalocaDados(&outLimpa[im]);
	}
	return 100.00;
}

void filtroMedia(Imagem* img, Imagem* out, int winSize){
	int tam=winSize/2;	//Setta o tamanho para pular bordas
	ul i,j;
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
	for(ul i=x-tam;i<=x+tam;i++)
		for(ul j=y-tam;j<=y+tam;j++)
			media+=v->dados[canal][i][j];
	return media/(t*t);
}

void interseccaoCanais(Imagem* in,Imagem* out){
	for(int i=0;i<in->altura;i++){
		for(int j=0;j<in->largura;j++){
			if((in->dados[0][i][j] + in->dados[1][i][j] + in->dados[2][i][j])/3.0 < THRESH_INTER){
				in->dados[0][i][j]=0;in->dados[1][i][j]=0;in->dados[2][i][j]=0;
			}
			else{
				in->dados[0][i][j]=255;in->dados[1][i][j]=255;in->dados[2][i][j]=255;
			}
		}
	}
}

void alocaDados(Imagem* x){
	x->dados=(unsigned char***)malloc(sizeof(unsigned char**)*x->n_canais);
		for(int i=0;i<x->n_canais;i++){
			x->dados[i]=(unsigned char**)malloc(sizeof(unsigned char*)*x->altura);
			for(ul j=0;j<x->altura;j++)
				x->dados[i][j]=(unsigned char*)malloc(sizeof(unsigned char)*x->largura);
		}
}

void desalocaDados(Imagem* x){
	for (int i=0;i<x->n_canais;i++){
			for (ul j=0;j<x->altura;j++)
				free (x->dados[i][j]);
			free (x->dados[i]);
		}
		free(x->dados);
}

void procuraBlob(Imagem in,ul *xi,ul *yi, ul *xf,ul *yf){
	int tam = TAM_JANELA/2;
	ul size,aux,maior[3]={0};	//Maior armazena={tamanho,coord. Xf,coord. Yf}
	for(ul i=tam;i<=in.altura-tam;i++){
		size=0;
		for(ul j=tam;j<=i && j<in.largura-tam;j++){
			if(size==0 && in.dados[0][i-j][j]==255)
				aux=j;
			size += in.dados[0][i-j][j]==255;
		}
		if(size>maior[0]){
			maior[0]=size;
			maior[1]=i;
			maior[2]=aux;
		}
	}
	*xf=maior[1];
	*yf=maior[2];
	*xi=(*xf)-(unsigned long)maior[0]*sqrt(2)/2;
	*yi=(*yf)-(unsigned long)maior[0]*sqrt(2)/2;
}
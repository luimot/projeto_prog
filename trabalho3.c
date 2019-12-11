/*============================================================================*/
/* CSF13 - TRABALHO 2                                                         */
/*----------------------------------------------------------------------------*/
/* Autores:	LUIZ FERNANDO DE ALMEIDA MOTA / RA:1661833						  */
/*			RUDINEI							 								  */
/*			ROBSON															  */
/*============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "trabalho3.h"
#include "imagem.h"

#define MAX 64
#define V_MIN_TRESHOLD 27	//Valor mínimo para limpar campos com muito ruído no arquivo limpo //38-39 ta bom demais
#define TAM_JANELA 3
#define JANELA_PM 5
#define THRESH_INTER 202
#define PRETO 0
#define BRANCO 255
#define R 0
#define G 1
#define B 2
#define LIM_PM 20
#define LIMITE_PONTO 15 //13

#define SALVA_MODELOS 0		//Flag para salvar os modelos

typedef struct posicao{
	unsigned long x;
	unsigned long y;
} Posicao;

void filtroMedia(Imagem* img, int winSize);
unsigned char mediaVizinhanca(Imagem v,int x, int y,int t,int canal);
void interseccaoCanais(Imagem* in);
void alocaDados(Imagem* x);
void desalocaDados(Imagem* x);
void procuraBlob(Imagem in,unsigned long *xi,unsigned long *yi, unsigned long *xf,unsigned long *yf);
void filtroPonto(Imagem* img);
void pontoMedio(Imagem img,Posicao* pos);
bool janelaPosterior(Imagem img,unsigned long x,unsigned long y);


/*A função calculaDistancia recebe as imagens a serem analisadas como também seu fundo
 * e seu índice, calcula a distancia entre os veículos das duas imagens e retorna a distância estimada.
 *Parâmetros:Imagem* bg:É a struct com o fundo das imagens geradas.	
 *	Imagem* img1:É a primeira struct com as informações da imagem formada.
 *	Imagem* img2:É a segunda struct com as informações da imagem formada.
 *Valor de retorno: Uma double com a distância estimada entre os veículos nas imagens fornecidas.
 */
double calculaDistancia (Imagem* bg, Imagem* img1, Imagem* img2){
	Imagem *copia[2]={img1,img2};
	Posicao pos_i,pos_f;
	int somaM=0,maior[2];
	Imagem outLimpa[2];
	filtroMedia(img1,TAM_JANELA);
	filtroMedia(img2,TAM_JANELA);
	int im,c,k,l;
	unsigned long i,j;
	for(im=0;im<2;im++){
		outLimpa[im].altura=bg->altura;
		outLimpa[im].largura=bg->largura;
		outLimpa[im].n_canais=bg->n_canais;
		alocaDados(&outLimpa[im]);
		for(c=0;c<3;c++){
			for(i=0;i<outLimpa[im].altura;i++){
				somaM=0;
				for(j=0;j<outLimpa[im].largura;j++){
					outLimpa[im].dados[c][i][j]= copia[im]->dados[c][i][j] - bg->dados[c][i][j];
					somaM+= outLimpa[im].dados[c][i][j];
					if(outLimpa[im].dados[c][i][j] < V_MIN_TRESHOLD)
						outLimpa[im].dados[c][i][j]=BRANCO;
				}
			}
		}
		interseccaoCanais(&outLimpa[im]);
		filtroPonto(&outLimpa[im]);
	}
	pontoMedio(outLimpa[0],&pos_i);
	pontoMedio(outLimpa[1],&pos_f);
	#if (SALVA_MODELOS)		//diretiva para compilar o código de salvamento de modelos ou não
	char str[2][MAX];
	sprintf(str[0],"modelo-1.bmp");
	sprintf(str[1],"modelo-2.bmp");
	for(k=-2;k<3;k++){
		for(l=-2;l<3;l++){
			outLimpa[0].dados[R][pos_i.y+k][pos_i.x+l]=BRANCO;	//Insere pontos vermelhos nas coordenadas de interesse.
			outLimpa[0].dados[G][pos_i.y+k][pos_i.x+l]=PRETO;
			outLimpa[0].dados[B][pos_i.y+k][pos_i.x+l]=PRETO;
			outLimpa[1].dados[R][pos_f.y+k][pos_f.x+l]=BRANCO;
			outLimpa[1].dados[G][pos_f.y+k][pos_f.x+l]=PRETO;
			outLimpa[1].dados[B][pos_f.y+k][pos_f.x+l]=PRETO;
		}
	}
	salvaImagem(&outLimpa[0],str[0]);
	salvaImagem(&outLimpa[1],str[1]);
	#endif
	desalocaDados(&outLimpa[0]);
	desalocaDados(&outLimpa[1]);
	return sqrt( pow(abs(pos_f.x-pos_i.x),2)+ pow(abs(pos_f.y-pos_i.y),2) );
}
/*A função filtroMedia recebe uma imagem e a altera a mesma com uma filtragem de Média,
 * eliminando uma parte dos ruídos da imagem analisada.
 *Parâmetros: Imagem *img:É a struct com os dados da imagem a serem lidos e alterados.
 *	int winSize:Tamanho da janela utilizada no filtro
 *	Valores de retorno:
 */
void filtroMedia(Imagem* img, int winSize){
	int tam=winSize/2;	//Setta o tamanho para pular bordas
	int canal;
	unsigned long i,j;
	for(canal=0;canal<3;canal++){
		for(i=tam;i<img->altura-tam;i++){
			for(j=tam;j<img->largura-tam;j++){
				img->dados[canal][i][j]=mediaVizinhanca(*img,i,j,winSize,canal);
			}	
		}
	}
}
/**A função mediaVizinhanca recebe os dados da imagem, a posição de um pixel requisitado,
 * o tamanho da janela e o canal analizado e retorna a media entre os vizinhos desse pixel.
 *Parâmetros: Imagem v:É a struct com os dados da imagem a serem lidos.
 *	int x e int y:É a coordenada x e y onde o pixel analizado se encontra e é requisitado.
 *	int t:É o tamanho da janela que a função irá aplicar a média.
 *	int canal:É o canal analizado.
 *Valores de retorno: Um unsigned char com o valor da média dos vizinhos daquele pixel. 
 */
unsigned char mediaVizinhanca(Imagem v,int x, int y,int t,int canal){
	int media=0,tam=t/2;
	unsigned long i,j;
	for(i=x-tam;i<=x+tam;i++)
		for(j=y-tam;j<=y+tam;j++)
			media+=v.dados[canal][i][j];
		return media/(t*t);
}

/**A função interseccaoCanais recebe os dados da imagem e faz a intersecção entre os canais dessa 
 * imagem, binarizando-a a partir de um limiar entre a media dos 3 canais.
 *Parâmetros: Imagem* in:É a struct com os dados da imagem a serem maniplados.
 *Valores de retorno:NENHUM
 */
void interseccaoCanais(Imagem* in){
	unsigned long i,j;
	for(i=0;i<in->altura;i++){
		for(j=0;j<in->largura;j++){
			if((in->dados[R][i][j] + in->dados[G][i][j] + in->dados[B][i][j])/3.0 < THRESH_INTER){
				in->dados[R][i][j] = BRANCO;
				#if SALVA_MODELOS
				in->dados[G][i][j] = in->dados[B][i][j]=BRANCO;
				#endif
			}
			else{
				in->dados[R][i][j] = PRETO;
				#if SALVA_MODELOS
					in->dados[G][i][j] = in->dados[B][i][j]= PRETO;
				#endif
			}
		}
	}
}
/**A função alocaDados recebe o ponteiro de Imagem para alocá-lo na memória e retornar por 
 *	referência de acordo com a necessidade.
 *Parâmetros: Imagem* x:É o ponteiro de imagem para alocamento.
 *Valores de retorno: NENHUM
 */
void alocaDados(Imagem* x){
	int i;
	unsigned long j;
	x->dados=(unsigned char***)malloc(sizeof(unsigned char**)*x->n_canais);
	for(i=0;i<x->n_canais;i++){
		x->dados[i]=(unsigned char**)malloc(sizeof(unsigned char*)*x->altura);
		for(j=0;j<x->altura;j++)
			x->dados[i][j]=(unsigned char*)malloc(sizeof(unsigned char)*x->largura);
	}
}
/**A função desalocaDados recebe por parâmetro, o ponteiro de Imagem para desalocar todos
 *	seus ponteiros e ponteiros de ponteiros, etc.
 *Parâmetros: Imagem *x:É o ponteiro da imagem para desalocar
 *Valor de retorno:NENHUM.
 */
void desalocaDados(Imagem* x){
	int i;
	unsigned long j;
	for (i=0;i<x->n_canais;i++){
		for (j=0;j<x->altura;j++)
			free (x->dados[i][j]);
		free (x->dados[i]);
	}
	free(x->dados);
}
/**A função filtroPonto recebe a imagem e filtra certos pixels e pequenos blobs
 *	"errantes" que dificultam a medição. 
 *Parâmetros: Imagem *img: É a struct contendo os dados da imagem a serem filtrados e 
 *	que serão alterados.
 *Valores de retorno:NENHUM.
 */
void filtroPonto(Imagem* img){
	unsigned long i,j;
	int k,cont;
	for(i = LIM_PM ; i < img->altura-LIM_PM ; i ++){
		for(j = LIM_PM ; j < img->largura-LIM_PM ; j ++){
			if(img->dados[R][i][j] == BRANCO){
				cont=0;
				for(k=1;k<=LIMITE_PONTO;k++)
					if(img->dados[R][i+k][j+k] == PRETO)		
						cont++;
					if(cont==LIMITE_PONTO)
						img->dados[R][i][j] = img->dados[G][i][j] = img->dados[B][i][j] = PRETO;
			}
		}
	}
	
}
/**A função pontoMedio tenta encontrar os pontos iniciais e finais de um blob e
 * com isso calcula a média entre os dois pontos com respeito a suas coordenadas.
 *Parâmetros:Imagem img:É a struct com os dados da imagem a serem lidos.
 *Posicao* pos:É a struct que recebe os dados de posicao do ponto médio do objeto
 * procurado por referência.
 *Valores de retorno:NENHUM
 */
void pontoMedio(Imagem img, Posicao* pos){
	int winSize = JANELA_PM/2;
	int cont;
	int maximo=(int)pow(2*winSize+1,2);
	bool flag_continuo=false;
	Posicao posQuina[2];
	unsigned long i,j;
	int k,l;
	for(i=LIM_PM;i<img.altura-LIM_PM && !flag_continuo;i++){
		for(j=LIM_PM;j<img.largura-LIM_PM && !flag_continuo;j++){
			//Janela 1
			cont=0;
			for(k=-winSize;k<=winSize && !flag_continuo;k++){
				for(l=-winSize;l<=winSize && !flag_continuo;l++){	//Se mais da metade+1 da janela for de brancos, identifica o primeiro ponto
					if(img.dados[R][i+k][j+l] == BRANCO)
						cont++;
					if(cont>maximo/2+1){
						posQuina[0].x=j;
						posQuina[0].y=i;
						flag_continuo=true;
					}
				}
			}
		}
	}
	flag_continuo=false;
	for(i=posQuina[0].y;i<img.altura-LIM_PM && !flag_continuo;i++){
		for(j=posQuina[0].x+1;j<img.largura-LIM_PM && !flag_continuo;j++){
			//Janela2
			if(janelaPosterior(img,j,i) && janelaPosterior(img,j+1,i) && janelaPosterior(img,j+2,i)){
				i++;
				break;
			}
			for(k=-winSize;k<=winSize && !flag_continuo;k++){
				for(l=-winSize;l<=winSize;l++){		//Igual o anterior, mas checando para pretos com valor consideravel de brancos
					if(img.dados[R][i+k][j+l] == PRETO)//Se todo preto, todo branco ou poucos pretos e brancos, ignora. 
						cont++;
				}
				if(cont>=maximo/2+1 && cont<maximo){
					posQuina[1].x=j;
					posQuina[1].y=i;
					flag_continuo=true;
				}
			}
		}
	}
	pos->x=(posQuina[0].x+posQuina[1].x)/2;
	pos->y=(posQuina[0].y+posQuina[1].y)/2;
}
/**A função janelaPosterior é útil para facilitar a implementação da função pontoMedio
 *	verificando se certos pixels têm a vizinhança com considerável número de brancos.
 *Parâmetros:Imagem img:É a struct que contém os dados a serem analisados
 *	unsigned long x e unsigned long y:São as coordenadas x e y na imagem que foram 
 *	requisitadas para análise.
 *Valor de retorno: Boolean, se todas forem majoritariamente com pixels brancos, true, 
 *	senão, false.
 */
bool janelaPosterior(Imagem img,unsigned long x,unsigned long y){	//Verfica se todos os pixeis na janela sao brancos, retorna 1 se sim.
	int winSize=TAM_JANELA/2;
	int cont=0;
	unsigned long i,j;
	for(i=y-winSize;i<y+winSize;i++){
		for(j=x-winSize;j<x+winSize;j++){
			if(img.dados[R][i][j]==BRANCO)
				cont++;
			if(cont > (int)pow(2*winSize+2,2))
				return true;
		}
	}
	return false;
}
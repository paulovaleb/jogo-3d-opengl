#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <stdio.h>
#include <time.h>         //Gera índices aleatórios para a velocidade do barril
#include <windows.h>      //reproduz sons de fundo
#include <mmsystem.h>    //Reproduz sons de fundo
#define STB_IMAGE_IMPLEMENTATION//Carrega textura das imagens
#include "stb_image.h" //Carrega textura das imagens
#include "glm.h"//Carregar os arquivos obj
#define quantEscadas 4
#define BLOCOS 47
#include <string>

float R, G, B;

bool cima = false, baixo = false, pulo = false, menu = true, gameover = false; //botoes para mover o personagem

float personX = 0.85, personY = 0.0, personZ = 0.83, raioPerson = 0.07; //coordenadas iniciais do personagem

GLMmodel * pmodel = NULL; //carrega um objeto
GLMmodel * pmode2 = NULL; //carrega um objeto

int contPulo = 0; //Translacao dopersonagem ao pular

int personagem_colidiu_escada_subida = 0; //quando sobe a escada
int translacao_frente_escada_subida = 0; //auxilia p transaladar p frente da escada enquanto sobe

int personagem_colidiu_escada_descida = 0; //quando desce a escada
int translacao_frente_escada_descida = 0; //auxilia p transaladar p frente da escada enquanto desce

float raioBarril = 0.05; //coordenadas iniciais do barril
float velocidadesBarril[3] = {0.015 , 0.015, 0.015}; //Vetor que é usado para definir a velocidade do barril
typedef struct Retangulo;  //struct para auxiliar na textura das figuras dos menus

// Declaracoes forward das funcoes utilizadas
void init(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void keyboard_special(int key, int x, int y);
void GerenciaMouse(int button, int state, int x, int y);
void display(void);
void criaCenario(void);
void DefineIluminacao(void);
void criaPersonagens(void);
void menuPrincipal(void);
void menuGameOver(void);
void timer_callback(int value);
bool colisao(float x1, float y1, float z1, float raio1, float x2, float y2, float z2, float raio2); //colisao esfera esfera
bool colisaoBlock(float x1, float y1 , float z1, float altura, float largura, float profundidade, float x2, float y2, float z2, float raio2); //colisao esfera e paralelepipedo
bool colisaoMouse(Retangulo figura, float x_clique, float y_clique); //olha se um ponto(clique) está dentro de um quadrilatero (colisao quadriátero e ponto)
void drawModel(char * fname);
void drawMode2(char * fname);
void criaEscada(float R, float G, float B);
void criaQuadrado(float x);
void criaCubo(float x);

struct Escada
{
	float x;
	float y;
	float z;
	float altura;
	float largura;
	float profundidade;

};

typedef struct BlocoTeste
{
	float x;
	float y;
	float z;
	float raio;
	bool colide;
} tBloco;


typedef struct Barril
{
	float x;
	float y;
	float z;
	float raio;
	float velX;
	float velZ;
	float rotacao;
	bool esquerda;
	bool direita;
	bool baixo;
} tBarril;

typedef struct Retangulo
{
	float x;
	float y;
	float altura;
	float largura;
};

//Global
tBloco blocks[BLOCOS]; //vetor de blocos do cenário
Escada escadas[quantEscadas]; //vetor de escadas do cenário
tBloco clif[3];  //vetor que indica os clif do cenário
tBarril barris[20]; //vetor dos barris do cenário
int quantBarris = 0;  //variável que indica a quantidade atual de barris
int intervalo = 0; //variável usada para saber a quantidade de vezes que da função display foi chamada

//rotacionar visão
double rotate_y = 0;
double rotate_x = 0;
GLfloat angulo = 0.0f;

int main(int argc, char * * argv)
{

	glutInit( & argc, argv); // Passagens de parametros C para o glut
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // double-buffering | sistema de cor utilizado | Associação com profundidade
	glutInitWindowSize (1000, 700);  // Tamanho da janela do OpenGL
	glutInitWindowPosition (100, 100); //Posicao inicial da janela do OpenGL

	glutCreateWindow ("Donkey Kong"); // Da nome para uma janela OpenGL
	glutMouseFunc(GerenciaMouse);
	init(); // Chama a funcao init();
	glEnable(GL_DEPTH_TEST);// Habilitando o teste de profundidade do Z-buffer
	// Registra a funcao callback que ser chamada a cada intervalo de tempo
	glutReshapeFunc(reshape); //funcao callback para redesenhar a tela
	glutDisplayFunc(display); //funcao callback de desenho
	glutKeyboardFunc(keyboard); //funcao callback do teclado
	glutSpecialFunc(keyboard_special);	//funcao callback do teclado especial

	glutMainLoop(); // executa o loop do OpenGL
	glDisable(GL_TEXTURE_2D);
	return 0; // retorna 0 para o tipo inteiro da funcao main();
}




// Funcao com alguns comandos para a inicializacao do OpenGL;
void init(void)
{
	PlaySound(TEXT("../audios/fundo.wav"), NULL, SND_ASYNC | SND_LOOP);  //Inicia o áudio de fundo do jogo
	srand(time(NULL));//função para o random
	tBarril b; //criando o primeiro barril
	b.x = 0.2;
	b.y = 0.0;
	b.z = -0.65;
	b.raio = 0.05;
	b.velX = 0.015;
	b.velZ = 0.015;
	b.rotacao = velocidadesBarril[rand() % 3];  //seleciona a velocidade entre os valores do vetor após o sorteio
	b.esquerda = true;
	b.direita = false;
	b.baixo = false;
	barris[quantBarris++] = b; //coloca o barril no vetor e incrementa a quantiade de barris
	//mostra os objetos com um
	glEnable(GL_FOG);
	glEnable(GL_COLOR_MATERIAL);// Habilita a definicao da cor do material(objeto) a partir da cor corrente
	glEnable(GL_LIGHTING); 	//Habilita o uso de iluminacao
	glEnable(GL_LIGHT0);	// Habilita a luz de numero 0
	glEnable(GL_DEPTH_TEST);  // Habilita o depth-buffering
	glEnable(GL_BLEND); //Exibe os valores de cores do fragmento computado com os valores nos buffers de cores.
	/* Activa o modelo de sombreagem de "Gouraud". */
	glShadeModel( GL_SMOOTH );
	glEnable(GL_TEXTURE_2D);//ativa o modo textura 2D

	glutTimerFunc(30, timer_callback, 30); //define a funcao a ser chamada a cada 30 milisegundos
}

void GerenciaMouse(int button, int state, int x, int y)
{
	Retangulo r;
	if (button == GLUT_LEFT_BUTTON)
		if (state == GLUT_DOWN)
		{
			if(menu)
			{//define local do click
				r.x = 600; 
				r.y = 550; 
				r.largura = 250;
				r.altura = 200;

				if(colisaoMouse(r, x, y))
				{
					menu = false;
					PlaySound(TEXT("../audios/fundo.wav"), NULL, SND_ASYNC | SND_LOOP);
				}
			}
			else if(gameover)// tentar novamente
			{
				r.x = 550; 
				r.y = 310;
				r.largura = 240;
				r.altura = 90;
				if(colisaoMouse(r, x, y))
				{
					gameover = false;
					PlaySound(TEXT("../audios/fundo.wav"), NULL, SND_ASYNC | SND_LOOP);
				}
				else//Sair
				{
					r.x = 500; 
					r.y = 360;
					r.largura = 80;
					r.altura = 30;
					if(colisaoMouse(r, x, y))
					{
						exit(0);
					}
				}
			}
		}
	glutPostRedisplay();
}

void reshape(int w, int h)
{

	/* define a zona da janela onde se vai desenhar */
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	/* muda para o modo GL_PROJECTION e reinicia a projecção */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/* Define a forma do "viewing volume" para termos *
	 * uma projecção de perspectiva (3D).             */
	gluPerspective(30, (float)w / (float)h, 1.0, 100.0);

	/* muda para o modo GL_MODELVIEW (não pretendemos alterar a projecção
	 * quando estivermos a desenhar no display) */
	glMatrixMode (GL_MODELVIEW);

}


// Funcao usada na funcao callback para controlar as teclas comuns (1 Byte) do teclado
void keyboard(unsigned char key, int x, int y)
{
	switch (key)   // key - variavel que possui valor ASCII da tecla precionada
	{
	case 27: // codigo ASCII da tecla ESC
		exit(0); // comando pra finalizacao do programa
		break;
	case ' '://tecla espaco do teclado
		pulo = true;
		break;
	}
}

void timer_callback(int value)
{
	glutTimerFunc(value, timer_callback, value);
	//Verifica se o personagem colidiu com um dos barris
	for(int i = 0; i < quantBarris; i++)
	{
		if(personagem_colidiu_escada_subida == 0 && personagem_colidiu_escada_descida == 0)   //se nao estiver subindo/descendo na escada
		{//mario colidir com barril
			if(colisao(barris[i].x, barris[i].y, barris[i].z, barris[i].raio, personX, personY, personZ, raioPerson))
			{
				personX = 0.85;//restaura todas coordenadas padrões
				personY = 0.0;
				personZ = 0.83;
				personagem_colidiu_escada_subida = 0;
				personagem_colidiu_escada_descida = 0;
				translacao_frente_escada_subida = 0;
				translacao_frente_escada_descida = 0;
				contPulo = 0;
				cima = false;
				baixo = false;
				pulo = false;
				gameover = true;
				quantBarris = 0;
				PlaySound(TEXT("../audios/gameover.wav"), NULL, SND_SYNC);
				break;
			}
		}
		else     //se estiver subindo na escada passa como parametro o raio do person um pouco maior p colidir com o barril
		{// se o mario colidir com barril na escada
			if(colisao(barris[i].x, barris[i].y, barris[i].z, barris[i].raio, personX, personY, personZ, raioPerson + 0.05))
			{
				personX = 0.85;//restaura todas coordenadas padrões
				personY = 0.0;
				personZ = 0.83;
				personagem_colidiu_escada_subida = 0;
				personagem_colidiu_escada_descida = 0;
				translacao_frente_escada_subida = 0;
				translacao_frente_escada_descida = 0;
				contPulo = 0;
				cima = false;
				baixo = false;
				pulo = false;
				gameover = true;
				quantBarris = 0;
				PlaySound(TEXT("audios/perdeu.wav"), NULL, SND_SYNC);
				break;
			}
		}
//se barriu colide com parede muda direção
		if(barris[i].x <= -1)
		{
			barris[i].esquerda = false;
			barris[i].direita = true;
			barris[i].baixo = false;
		}
		if(barris[i].x >= 1)
		{
			barris[i].esquerda = true;
			barris[i].direita = false;
			barris[i].baixo = false;
		}
		int barril_colidiu = 0, barril_colidiu_primeiro_bloco = -1;
		for(int j = 0; j < BLOCOS; j ++)   //verifica quando o barril desce
		{
			if(blocks[j].colide && colisao(blocks[j].x, blocks[j].y, blocks[j].z, blocks[j].raio, barris[i].x, barris[i].y, barris[i].z, barris[i].raio) )
			{
				barril_colidiu = 1;
				break;
			}//verifica se barriu colidio com o ultimo bloco
			if(j == 0 && !blocks[j].colide && colisao(blocks[j].x, blocks[j].y, blocks[j].z, blocks[j].raio, barris[i].x, barris[i].y, barris[i].z, barris[i].raio) )
			{
				barril_colidiu_primeiro_bloco = 0;
			}
		}
		if(barril_colidiu == 0)   //verifica se barriu colidiu com o bloco da escada ou ultimo bloco
		{
			barris[i].baixo = true;
			if(barril_colidiu_primeiro_bloco == 0)   //restaura os valores de translacao inicial do barril
			{
				barris[i].x = 0.2;
				barris[i].y = 0.0;
				barris[i].z = -0.65;
				barris[i].esquerda = true;
				barris[i].direita = false;
				barris[i].baixo = false;
			}
		}

	}
	if(!pulo) //se nao pulou, verifica se colidiu com
		for(int i = 0; i < 3; i++)   //clif
		{
			if(colisaoBlock(clif[i].x, clif[i].y, clif[i].z, clif[i].raio * 2 + 0.25, 0.06, 0.08, personX, personY, personZ, raioPerson))
				personZ += 0.035;
		}
	if(colisao(0.23, 0.02, -1.21, 0.07, personX, personY, personZ, raioPerson))   //verifica se o mario colidiu com a princesa
	{
		PlaySound(TEXT("../audios/ganhou.wav"), NULL, SND_ASYNC);
		personX = 0.85;//restaura todas coordenadas padrões
		personY = 0.0;
		personZ = 0.83;
		contPulo = 0;
		cima = false;
		baixo = false;
		pulo = false;
		menu = true;  //se ganhou, reinicia o menu
		quantBarris = 0;
		menuPrincipal();

	}
	glutPostRedisplay(); // Manda redesenhar o display em cada frame
}



//Funcao para controlar as teclas especiais (2 Byte) do teclado
void keyboard_special(int key, int x, int y)
{
	switch(key)
	{
		//  Rotacao 5 graus esquerda
	case GLUT_KEY_F1:
		rotate_y += 5;
		glutPostRedisplay();
		break;

		//  Rotacao 5 graus direita
	case GLUT_KEY_F2:
		rotate_y -= 5;
		glutPostRedisplay();
		break;

	case GLUT_KEY_F3: //Rotacao 5 graus cima
		rotate_x += 5;
		glutPostRedisplay();
		break;

	case GLUT_KEY_F4://Rotacao 5 graus baixo
		rotate_x -= 5;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT://seta esquerda
		if(cima || baixo) //evita que o personagem se movimente na horizontal se ele estiver subindo uma escada
			break;
		if(personX > -1)
			personX -= 0.035;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT://seta direita
		if(cima || baixo) //evita que o personagem se movimente na horizontal se ele estiver subindo uma escada
			break;
		if(personX < 1)
			personX += 0.035;
		glutPostRedisplay();
		break;
	case GLUT_KEY_UP://seta cima
		personagem_colidiu_escada_subida = 0;
		if(!pulo)    // verifica se colidiu com a escada
		{
			for(int i = 0; i < quantEscadas; i++)   //verifica se ha colisao com a escada e personagem
			{
				if (colisaoBlock(escadas[i].x, escadas[i].y, escadas[i].z, escadas[i].altura, escadas[i].largura, escadas[i].profundidade, personX, personY, personZ, raioPerson))
				{
					personZ -= 0.035;
					cima = true;
					baixo = false;
					personagem_colidiu_escada_subida = 1;
					translacao_frente_escada_subida++;
					break;
				}
			}
			if(personagem_colidiu_escada_subida == 0)   //quando nao colidir mais com a escada
			{
				cima = false;
				if(translacao_frente_escada_subida > 0 && personY < 0.0)   //transalada o personagem para tras	verifica se ja nao foi transaladado enquanto descia
				{
					personY = 0;
				}
				translacao_frente_escada_subida = 0;
				translacao_frente_escada_descida = 0;
			}
			if(cima && translacao_frente_escada_subida == 1 && personY == 0)   //transalada o personagem para frente confere se ja nao foi colocado p frente enquando descia
			{
				personY = -0.14;
			}
		}
		glutPostRedisplay();
		break;


	case GLUT_KEY_DOWN://seta baixo
		personagem_colidiu_escada_descida = 0;
		//verifica se colidiu com alguma escada
		for(int i = 1; i < BLOCOS; i++)
		{
			//verifica se há colisao e mario colide com bloco
			if(blocks[i].colide && colisao(blocks[i].x, blocks[i].y, blocks[i].z, blocks[i].raio, personX, personY, personZ, raioPerson))
			{
				cima = false;
				baixo = false;
			}//se há !colide e bloco i == 21 e i == 22 e mario colide com o bloco
			if(!blocks[i].colide && (i == 21 || i == 22) && colisaoBlock(blocks[i].x, blocks[i].y, blocks[i].z, 0.18 * 2 + 0.09, 0.18, 0.18, personX, personY, personZ, raioPerson))
			{

				personZ += 0.035;
				baixo = true;
				cima = false;
				personagem_colidiu_escada_descida = 1;//desce escada
				translacao_frente_escada_descida++;
				break;
			}
			else if(!blocks[i].colide && (i == 24) && colisaoBlock(blocks[i].x, blocks[i].y, blocks[i].z, 0.18 * 2 + 0.12, 0.05, 0.18, personX, personY, personZ, raioPerson))
			{

				personZ += 0.035;
				baixo = true;
				cima = false;
				personagem_colidiu_escada_descida = 1;
				translacao_frente_escada_descida++;
				break;
			}
			else if(!blocks[i].colide && (i == 43) && colisaoBlock(blocks[i].x, blocks[i].y, blocks[i].z, 0.18 * 2 + 0.13, 0.05, 0.18, personX, personY, personZ, raioPerson))
			{

				personZ += 0.035;
				baixo = true;
				cima = false;
				personagem_colidiu_escada_descida = 1;
				translacao_frente_escada_descida++;
				break;
			}

		}
		if(personagem_colidiu_escada_descida == 0)   //quando nao colidir mais
		{
			baixo = false;
			if(translacao_frente_escada_descida > 0 && personY < 0)   //transalada o personagem para tras
			{
				personY = 0;
			}
			translacao_frente_escada_descida = 0;
			translacao_frente_escada_subida = 0;
		}
		if(baixo && translacao_frente_escada_descida == 1 && personY == 0)   //transalada o personagem para frente
		{
			personY = -0.14;
		}

		glutPostRedisplay();
		break;
	default:
		glutPostRedisplay();
		break;

	}

}

// Funcao usada na funcao callback para desenhar na tela
void display(void)
{
	/* Limpa depth buffer, e reinicia a matriz */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	// Especifica sistema de coordenadas de projecao
	glMatrixMode(GL_PROJECTION);
	glRotatef( rotate_x, 1.0, 0.0, 0.0 );
	glRotatef( rotate_y, 0.0, 1.0, 0.0 );
	DefineIluminacao();
	gluLookAt(0.0f, 0.15f, 0.1f, 0.0f, 0.f, 0.f, 0.f, 1.0f, 0.f);//visao da camera

	if(menu){menuPrincipal();}
	else
	{
		if(gameover){menuGameOver();}
		else{criaCenario();criaPersonagens();}
	}
	intervalo += 1;
	glutSwapBuffers();
}


void criaCubo(float x)
{
	// Desenhas as linhas das "bordas" do cubo
	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(1.6f);
	glBegin(GL_LINE_LOOP);	// frontal
	glVertex3f(x, x, x);
	glVertex3f(-x, x, x);
	glVertex3f(-x, -x, x);
	glVertex3f(x, -x, x);
	glEnd();
	glBegin(GL_LINE_LOOP);	//  posterior
	glVertex3f(x, x, -x);
	glVertex3f(x, -x, -x);
	glVertex3f(-x, -x, -x);
	glVertex3f(-x, x, -x);
	glEnd();
	glBegin(GL_LINES);	//  laterais
	glVertex3f(-x, x, -x);
	glVertex3f(-x, x, x);
	glVertex3f(-x, -x, -x);
	glVertex3f(-x, -x, x);
	glVertex3f(x, x, -x);
	glVertex3f(x, x, x);
	glVertex3f(x, -x, -x);
	glVertex3f(x, -x, x);
	glEnd();

	// Desenha as faces do cubo preenchidas
	// Face frontal
	glBegin(GL_QUADS);
	glNormal3f(0.0, 0.0, 1.0);	// Normal da face
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(x, x, x);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-x, x, x);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-x, -x, x);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(x, -x, x);
	// Face posterior
	glNormal3f(0.0, 0.0, -1.0);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(x, x, -x);

	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex3f(x, -x, -x);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-x, -x, -x);

	glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-x, x, -x);

	// Face lateral esquerda
	glNormal3f(-1.0, 0.0, 0.0);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-x, x, x);

	glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-x, x, -x);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-x, -x, -x);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-x, -x, x);

	// Face lateral direita
	glNormal3f(1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(x, x, x);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(x, -x, x);
	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex3f(x, -x, -x);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(x, x, -x);
	// Face superior
	glNormal3f(0.0, 1.0, 0.0);
	glColor3f(0.0f, 1.0f, 1.0f);
	glVertex3f(-x, x, -x);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-x, x, x);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(x, x, x);
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex3f(x, x, -x);

	glNormal3f(0.0, -1.0, 0.0);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-x, -x, -x);
	glTexCoord2f(1, 0); //atribui coordenada de textura ao objeto
	glColor3f(1.0f, 0.0f, 1.0f);
	glVertex3f(x, -x, -x);
	glTexCoord2f(1, 1);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(x, -x, x);
	glTexCoord2f(0, 1);
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-x, -x, x);
	glTexCoord2f(0, 0);
	glEnd();
}

void DefineIluminacao (void)
{

	GLfloat posLuz[4] = { -0.2, -0.2, -1.0, 1.0 };// Posicao da fonte de luz
	GLfloat luzAmbiente[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat luzDifusa[4] = {0.7, 0.7, 0.7, 1.0};
	GLfloat luzEspecular[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat two_side[4] = {0.0, 1.0, 0.0, 1.0};
	// Capacidade de brilho do material
	GLfloat especularidade[4] = {1.0, 1.0, 1.0, 1.0};
	GLint especMaterial = 60;

	// Define a reflet?ncia do material
	glMaterialfv(GL_FRONT, GL_SPECULAR, especularidade);
	// Define a concentra??o do brilho
	glMateriali(GL_FRONT, GL_SHININESS, especMaterial);

	// Ativa o uso da luz ambiente
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

	// Define os par?metros da luz de numero 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa );
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular );
	glLightfv(GL_LIGHT0, GL_POSITION, posLuz );
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, two_side);

}



void criaCenario()
{
	float c = 1;
	GLuint texture1, texture2;

	int w, h;
	unsigned char * uc = stbi_load("texturas/ferro.png", & w, & h, NULL, 0);
	glGenTextures(1, & texture1); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture1); //Ativa a textura atual

	//Cria a textura lateral de cada bloco

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);


	int auxiliar = 0;
	tBloco b;
	for (int k = 0; k < 5; k++)   //numero de andares 'k'
	{
		int BLOCKS = 12;
		for(int i = 0; i < BLOCKS; i++)   //numero de blocos em cada andar 'i'
		{
			float tx, ty, tz;
			glPushMatrix();
			//verifica se draw block
			if(k == 0 || (k < 4 && i < 11) || (k == 4 && i > 3 && i < 6))
			{
				//numeros pares 
				if(k % 2 == 0)
				{
					tx = c;
					ty = 0.0;
					tz = 1 - 0.5 * k;
				}
				else
				{
					tx = - 0.2 + c;
					ty = 0.0;
					tz = 1 - 0.5 * k;
				}

				b.x = tx;
				b.y = ty;
				b.z = tz;
				b.raio = 0.12;
				//andar 0
				if(k == 0)
				{
					if(i == 0)
						b.colide = false;
					else
						b.colide = true;
				}
				
				else if(k == 1)
				{
					if(i == BLOCKS - 2 || i == BLOCKS - 3)
						b.colide = false;
					else
						b.colide = true;
				}
				
				else if(k == 2)
				{
					if(i < 2)
						b.colide = false;
					else
						b.colide = true;
				}

				else if(k == 3)
				{
					if(i == BLOCKS - 2 || i == BLOCKS - 3)
						b.colide = false;
					else
						b.colide = true;
				}
				
				blocks[auxiliar] = b;
				auxiliar++;
			}
			else
			{
				if(k == 1)
				{
					b.x = 1.0;
					b.y = 0.0;
					b.z = 0.5;
					b.raio = 0.12;
					b.colide = false;
					clif[0] = b;
				}
				else if(k == 2)
				{
					b.x = -0.98;
					b.y = 0.0;
					b.z = 0.0;
					b.raio = 0.12;
					b.colide = false;
					clif[1] = b;
				}
				else if(k == 3)
				{
					b.x = 1;
					b.y = 0.0;
					b.z = -0.5;
					b.raio = 0.12;
					b.colide = false;
					clif[2] = b;
				}
			}
			glTranslatef(tx, ty, tz);
			criaCubo(0.09);
			c = c - 0.18;//distancia entre cada bloco
			glPopMatrix();
		}
		c = 1;
	}
	//glDisable(GL_TEXTURE_2D);  //desativa a textura dos blocos
	glDeleteTextures(1, & texture1);
	stbi_image_free(uc);

	criaEscada(0.59, 0.29, 0.0);

	unsigned char * uc3 = stbi_load("texturas/barril.jpg", & w, & h, NULL, 0);
	glGenTextures(1, & texture2); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture2); //Ativa a textura atual
	//Cria a textura de cada barril

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glEnable(GL_COLOR_MATERIAL);


	//barril
	GLUquadric * quad = gluNewQuadric(); //cria um quadrado
	gluQuadricNormals(quad, GLU_FLAT); //FLAT significa uma normal para cada face do quadrado
	gluQuadricOrientation(quad, GLU_INSIDE); //vetor da normal apontando p dentro
	for(int barril = 0, fator = 0; barril < 4; barril++, fator++)   //barril proximo ao macaco
	{
		glPushMatrix();
		glColor3f(0.0f, 0.0f, 0.0f);//cor do objeto verde
		glRotatef(90, 1.0f, 0.0f, 0.0f);
		gluQuadricTexture(quad, GLU_TRUE);//textura do quadrado ativada
		//posicao final do barril
		if(fator < 2){glTranslatef(0.75 + (fator - fator * 0.9), -0.64, 0.0);}
		else{glTranslatef(0.75 + ((fator - 2) - (fator - 2) * 0.9), -0.74, 0.0);}

		gluSphere(quad, raioBarril, 30, 30); //gera a esfera baseado no objeto do quadrado
		glPopMatrix();

	}
	if(intervalo % 200 == 0 && quantBarris < 6)
	{
		srand(time(NULL));
		tBarril b;
		b.x = 0.2;
		b.y = 0.0;
		b.z = -0.65;
		b.raio = 0.05;
		b.velX = velocidadesBarril[rand() % 3];
		b.velZ = 0.015;
		b.rotacao = -0.02;
		b.esquerda = true;
		b.direita = false;
		b.baixo = false;
		barris[quantBarris++] = b;
	}

	for(int i = 0; i < quantBarris; i ++)
	{

		glPushMatrix();//barril em movimento
		glColor3f(0.0f, 0.0f, 0.0f);//cor do objeto verde
		gluQuadricTexture(quad, GLU_TRUE);//Ativa textura do quadrado
		glTranslatef(barris[i].x, barris[i].y, barris[i].z); //posicao final do barril
		glRotatef(90, barris[i].rotacao, 0.0f, 0.0f);//rotacao
		gluSphere(quad, barris[i].raio, 30, 30);//gera a esfera baseado no objeto do quadrado
		glPopMatrix();
		//Muda velocidade do barril
		if(barris[i].esquerda)
			barris[i].x -= barris[i].velX;
		if(barris[i].direita)
			barris[i].x += barris[i].velX;
		if(barris[i].baixo)
		{
			barris[i].z += barris[i].velZ;
			barris[i].baixo = false;
		}

		barris[i].rotacao = - barris[i].rotacao;  //Rotacao do barril

	}
	glDeleteTextures(1, & texture2);
	stbi_image_free(uc3);

	//Mario
	uc = stbi_load("models/m.jpg", & w, & h, NULL, 0);
	glGenTextures(1, &texture1); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture1); //Ativa a textura atual
	//Cria a textura lateral de cada bloco

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	//mario
	glPushMatrix();
	glTranslatef(personX, personY, personZ);
	glRotatef(180, 0, 0, 0);
	criaQuadrado(0.05);
	glPopMatrix();

	//Pulo do mario
	if (pulo)
	{
		contPulo += 1;
		if (contPulo <= 15) //numero de frames
			personZ -= 0.015;
		else if(contPulo > 15 && contPulo <= 30)
			personZ += 0.015;
		else
		{
			contPulo = 0;
			pulo = false;
		}
	}

	glDeleteTextures(1, & texture1);
	stbi_image_free(uc);
}

void menuPrincipal()
{
	GLuint texture1, texture2;
	int w, h;
	unsigned char * uc2 = stbi_load("texturas/dklogo.jpg", & w, & h, NULL, 0);
	glGenTextures(1, & texture2); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture2); //vincula a textura atual

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glPushMatrix();
	glTranslatef(0, 0, 0);
	glRotatef(180, 0, 0, 0); //rotacao de 180 graus
	glScalef(0.8, 0, 0.8);
	criaQuadrado(0.9);
	glPopMatrix();
	glDeleteTextures(1, & texture2);
	stbi_image_free(uc2);


	unsigned char * uc = stbi_load("texturas/play.jpg", & w, & h, NULL, 0);
	glGenTextures(1, & texture1); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture1); //Ativa a textura atual

	//Cria a textura do menu

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glPushMatrix();
	glTranslatef( 0, 0, 1);//origem
	glRotatef(180, 0, 0, 0); //rotacao de 180 graus
	criaQuadrado(0.09);
	glPopMatrix();
	glDeleteTextures(1, & texture1);
	stbi_image_free(uc);
}

void menuGameOver()
{
	GLuint texture1, texture2;
	int w, h;
	unsigned char * uc = stbi_load("texturas/game over.jpg", & w, & h, NULL, 0);
	glGenTextures(1, & texture1); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture1); //Ativa a textura atual

	//Cria a textura com o nome game over

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glPushMatrix();
	glTranslatef( 0, 0, -0.7);
	glRotatef(180, 0, 0, 0); //rotacao de 180 graus
	glScalef(4.5, 0, 2.5);
	criaQuadrado(0.09);
	glPopMatrix();
	glDeleteTextures(1, & texture1);
	stbi_image_free(uc);

	unsigned char * uc2 = stbi_load("texturas/opcoes.png", & w, & h, NULL, 0);
	glGenTextures(1, & texture2); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture2); //Ativa a textura atual

	//Cria a textura do menu Game Over

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glPushMatrix();
	glTranslatef( 0, 0, 0);//origem
	glRotatef(180, 0, 0, 0); //rotacao de 180 graus
	glScalef(4.5, 0, 4.5);
	criaQuadrado(0.09);
	glPopMatrix();
	glDeleteTextures(1, & texture2);
	stbi_image_free(uc2);

	gameover = true;


}

void criaEscada(float R, float G , float B)
{
	Escada e;

	//cria 4 escadas
	glPushMatrix();
	glColor3f(R, G, B);
	glTranslatef( -0.9, -0.09, 0.66);
	e.x = -0.9;
	e.y = -0.09;
	e.z = 0.66;
	e.altura = 0.5;
	e.largura = 0.06;
	e.profundidade = 0.08;
	escadas[0] = e;
	glScalef(0.1, 0.1, 0.25);
	glRotatef(180, 0.0f, 1.0f, -1.2f);
	drawMode2("models/escada.obj");
	glPopMatrix();

	glPushMatrix();
	glColor3f(R, G, B);
	glTranslatef( 0.80 , -0.09, 0.16);
	e.x = 0.80;
	e.y = -0.09;
	e.z = 0.16;
	e.altura = 0.48;
	e.largura = 0.06;
	e.profundidade = 0.08;
	escadas[1] = e;
	glScalef(0.1, 0.1, 0.25);
	glRotatef(180, 0.0f, 1.0f, -1.2f);
	drawMode2("models/escada.obj");
	glPopMatrix();

	glPushMatrix();
	glColor3f(R, G, B);
	glTranslatef( -0.80, -0.1, -0.33);
	e.x = -0.80;
	e.y = -0.1;
	e.z = -0.33;
	e.altura = 0.5101;
	e.largura = 0.06;
	e.profundidade = 0.08;
	escadas[2] = e;
	glScalef(0.1, 0.1, 0.26);
	glRotatef(180, 0.0f, 1.0f, -1.2f);
	drawMode2("models/escada.obj");
	glPopMatrix();

	glPushMatrix();
	glColor3f(R, G, B);
	glTranslatef( 0.25, -0.09, -0.855);
	e.x = 0.25;
	e.y = -0.09;
	e.z = -0.855;
	e.altura = 0.5;
	e.largura = 0.06;
	e.profundidade = 0.08;
	escadas[3] = e;
	glScalef(0.1, 0.1, 0.25);
	glRotatef(180, 0.0f, 1.0f, -1.2f);
	drawMode2("models/escada.obj");
	glPopMatrix();
}
//cria donkey kong
void drawModel(char * fname)
{
	if (!pmodel)
	{
		pmodel = glmReadOBJ(fname);
		if (!pmodel)
			exit(0);
		glmUnitize(pmodel);
		glmFacetNormals(pmodel);//normal do objeto
		glmVertexNormals(pmodel, 90.0);
		glmDraw(pmodel, GLM_SMOOTH);
	}
	glmDraw(pmodel, GLM_SMOOTH);
}
//Cria escadas
void drawMode2(char * fname)
{
	if (!pmode2)
	{
		pmode2 = glmReadOBJ(fname);
		if (!pmode2)
			exit(0);
		glmUnitize(pmode2);
		glmFacetNormals(pmode2);
		glmVertexNormals(pmode2, 90.0);
		glmDraw(pmode2, GLM_SMOOTH);
	}
	glmDraw(pmode2, GLM_SMOOTH);
}

void criaPersonagens()   //personagens(macaco e princesa)
{

	//cria macaco
	glPushMatrix();
	glColor3f(0.2f, 0.0f, 0.0f);
	glTranslatef( 0.55, 0.0, -0.68);
	glScalef(0.1, 0.1, 0.1);
	glRotatef(180, 0.0f, 1.0f, -0.5f);//rotacao
	drawModel("models/DKm.obj");
	glPopMatrix();

	//cria princesa
	float c = 1;
	GLuint texture1;
	int w, h;
	unsigned char * uc = stbi_load("models/p.jpg", & w, & h, NULL, 0);
	glGenTextures(1, &texture1); //gera nomes identificadores de texturas
	glBindTexture(GL_TEXTURE_2D, texture1); //Ativa a textura atual
	//Cria a textura lateral de cada bloco

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h,
				 0, GL_RGB, GL_UNSIGNED_BYTE, uc);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	glPushMatrix();
	glTranslatef( 0.18, -0.1, -1.21);
	glRotatef(180, 0, 0, 0);
	glScalef(0.5, 0, 1);
	criaQuadrado(0.1);
	glPopMatrix();

	glDeleteTextures(1, & texture1);
	stbi_image_free(uc);

}

void criaQuadrado(float x)
{

	glBegin(GL_QUADS);
	glColor3f(0.0f, 1.0f, 0.0f);//cor verde do objeto
	glTexCoord2f(1, 1); //atribui coordenada de textura ao objeto
	glVertex3f(-x, -x, -x);
	glTexCoord2f(0, 1);
	glVertex3f(x, -x, -x);
	glTexCoord2f(0, 0);
	glVertex3f(x, -x, x);
	glTexCoord2f(1, 0);
	glVertex3f(-x, -x, x);
	glEnd();
}

//colisao mario e barril
bool colisao(float x1, float y1, float z1, float raio1, float x2, float y2, float z2, float raio2)
{
	float d = sqrt((- x1 + x2)  * (- x1 +  x2) + (-y1 + y2) * (-y1 + y2) +  ( -z1 + z2) * (-z1 + z2));
	if(d <= (raio1 + raio2)){return true;}else{return false;}
}
//Colisao do paralelepípedo com esfera
bool colisaoBlock(float x1, float y1 , float z1, float altura, float largura, float profundidade, float x2, float y2, float z2, float raio2)
{
	double distanciaX = fabs(x2 - x1);
	double distanciaY = fabs(y2 - y1);
	double distanciaZ = fabs(z2 - z1);

	if (distanciaX > (largura / 2 + raio2)){return false;}
	if (distanciaY > (profundidade / 2 + raio2)){return false;}
	if (distanciaZ > (altura / 2 + raio2)){return false;}

	if (distanciaX <= (largura / 2)){return true;}
	if (distanciaY <= (profundidade / 2)){return true;}
	if (distanciaZ <= (altura / 2)){return true;}

	double distanciaDoCanto = ((distanciaX - largura / 2) * (distanciaX - largura / 2)) +
							   ((distanciaY - profundidade / 2) * (distanciaY - profundidade / 2) +
								((distanciaZ - altura / 2) * (distanciaZ - altura / 2)));

	return (distanciaDoCanto <= (raio2 * raio2));
}
//Colisao de um quadrado com um ponto(clique)
bool colisaoMouse(Retangulo figura, float x_clique, float y_clique)
{
	return (abs(figura.x - x_clique) * 2 < (figura.largura )) &&
		   (abs(figura.y - y_clique) * 2 < (figura.altura));
}
